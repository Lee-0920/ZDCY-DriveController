/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 设备简单链路协议。
 * @details DSLP 为上层提供无连接的、不可靠的传输服务。
 *  <p> DSLP 是逻辑链路层的一种简单实现，它没有任何确认机制来保证通信质量。
 *  DSLP 适用于物理通信比较可靠的链路，本身是不可靠的，通信系统的可靠性由
 *  上层协议保证。如果物理链路很不可靠，请使用带确认重传的LL层协议，以提高
 *  通信系统的整体性能。
 *  <p> DSLP 提供的功能包括：
 *    - 帧收发；
 *    - 上层协议分用；
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-23
 */

#include <string.h>
//#include <stdio.h>
#include "Dslp.h"

#include "Common/Utils.h"
#include "Common/SafeHeap.h"
#include "Common/MessageDigest.h"
#include "Tracer/Trace.h"

#include "FreeRTOS.h"
#include "semphr.h"

static xSemaphoreHandle s_semDslp;

//*******************************************************************
// ILaiHandle 接口声明

static void Dslp_OnReceived(Dslp* self, Byte* data, Uint16 len, Uint8 sourceAddr);
static Uint16 Dslp_OnSendQuery(Dslp* self);
static void Dslp_OnSendRequest(Dslp* self);

//*******************************************************************
// DSLP 实现

static void Dslp_Reset(Dslp* self, Uint16 action);
static Bool Dslp_IsSendable(Dslp* self);
static void Dslp_Send(Dslp* self, Uint8 upn, Uint8 destAddr, Frame* frame);

static const ILlComm s_kCommInterface =
{
        (IfLlReset) Dslp_Reset,
        (IfLlIsSendable) Dslp_IsSendable,
        (IfLlSend) Dslp_Send
};

/**
 * @brief 复位Ll协议状态逻辑。
 * @details 当上层协议体因为通信异常（如等待超时）需要复位Ll实体时调用。
 *  在Ll协议体通信之前不能也没有必要调用本函数。
 * @param ll 要操作的Ll层协议实体。
 * @param action 复位动作，可以是以下一个或多个动作的合并（或）：
 *        - @ref LL_TX_CLEAR
 *        - @ref LL_RX_CLEAR 
 *        - @ref LL_TX_ABORT 
 *        - @ref LL_RX_ABORT 
 * @note 实现时，应根据具体的底层协议，分别实现 @p action 指定的所有动作。
 */
void Dslp_Reset(Dslp* ll, Uint16 action)
{
    Lai* lai = ll->base.lai;

    //if (action & LL_TX_ABORT)
    //{
    //}

    //if (action & LL_RX_ABORT)
    //{
    //}

    //if (action & LL_TX_CLEAR)
    //{
    //}

    //if (action & LL_RX_CLEAR)
    //{
    //}
    
    lai->comm->Reset(lai, action);
}

/**
 * @brief 查询接口是否可发送数据。
 * @details 通常在 Send 之前调用。
 *  <p>可发送表示：本实体可立即发送一帧新数据，此时上层可直接调用 Send()
 *      函数把数据发送出去；
 *  <p>不可发送表示：本实体正在发送数据，或者发送缓冲中无剩余空间可供等待发送。
 *      此时上层实体应该等待 OnSendRequest 的到来进行异步发送。
 *  <p>如果正在发送，意味着发送缓冲存在数据或者硬件正在发送数据，
 *      此时上层可等待异步的发送请求。
 * @param ll 具体的Ll层协议实体，如 Dslp 。
 */
Bool Dslp_IsSendable(Dslp* ll)
{
    return !Queue_IsFull(&ll->transQueue);
}

/**
 * @brief 发送一帧数据。
 * @details 请先调用 IsSendable() 查询发送缓冲是否足够，无剩余空间则应该等待
 *   OnSendRequest 的到来进行异步发送。
 * @param ll 具体的Ll层协议实体，如 Dslp 。
 * @param upn 调用者的上层协议号。
 * @param destAddr 本帧要发往的目标地址（链路地址）。
 * @param frame 要发送的数据帧。
 */
void Dslp_Send(Dslp* ll, Uint8 upn, Uint8 destAddr, Frame* frame)
{
    FrameCtrlWord fcw;
    Uint16 chksum;
    Byte* data = Frame_GetData(frame);
    Byte* p;

    xSemaphoreTake(s_semDslp, portMAX_DELAY);
    Queue* queue = &ll->transQueue;

    // 处理 LL 层头部
    fcw.data = 0;
    fcw.info.uap = upn;
    data[0] = destAddr;
    data[1] = fcw.data;

    chksum = MessageDigest_Crc16Ccitt(0, data, frame->len - 2);
    p = data + frame->len - 2;
    *p++ = (Uint8) (chksum & 0x00FF);
    *p = (Uint8) ((chksum & 0xFF00) >> 8);

    // 按照队列的发送方式，先进先出
    if (!Queue_IsEmpty(queue))
    {
        // 队列中有数据待发送，本次数据包压入队列，先尝试发送队列中的数据
        Queue_Push(queue, frame);
        frame = Queue_Peek(queue);
        data = Frame_GetData(frame);
//      if (ll->laiComm->IsSendable(ll->base.lai, data[0]))
        if (ll->isSending == FALSE && ll->laiComm->IsSendable(ll->base.lai))
        {
            ll->isSending = TRUE;
            frame = Queue_Pop(queue);
            ll->laiComm->Send(ll->base.lai, data[0], data, frame->len);
            Frame_Delete(frame);
            ll->isSending = FALSE;
        }
    }
    else
    {
        // 队列为空， Lai 空闲则发送，繁忙则插入队列等待异步发送
//       if (ll->laiComm->IsSendable(ll->base.lai, destAddr))

        if (ll->isSending == FALSE && ll->laiComm->IsSendable(ll->base.lai))
        {
            ll->isSending = TRUE;
            ll->laiComm->Send(ll->base.lai, destAddr, data, frame->len);
            Frame_Delete(frame);
            ll->isSending = FALSE;
        }
        else
        {
            Queue_Push(queue, frame);
        }
    }

    xSemaphoreGive(s_semDslp);
}


//*******************************************************************
// DSLP 实现相关


/**
 * @brief Dslp 协议实体初始化。
 * @param self 自身对象。
 * @param laiToUse 协议使用的Lai实体，该对象必须已经被初始化。
 */
void Dslp_Init(Dslp* self, Lai* laiToUse)
{
    memset(self, 0, sizeof(Dslp));

    // 初始化各接口
    self->base.laiHandle.OnReceived = (IfLaiOnReceived) Dslp_OnReceived;
    self->base.laiHandle.OnSendQuery = (IfLaiOnSendQuery) Dslp_OnSendQuery;
    self->base.laiHandle.OnSendRequest = (IfLaiOnSendRequest) Dslp_OnSendRequest;
    self->base.comm = (ILlComm*) &s_kCommInterface;

    self->laiComm = laiToUse->comm;
    self->isSending = FALSE;

    /// @todo 调查：使用宏接口是否可以缩减代码？
    Queue_Init(&self->transQueue, (void**)self->transFrames, sizeof(self->transFrames)/sizeof(Frame*));

    vSemaphoreCreateBinary(s_semDslp);
}

/**
 * @brief Dslp 协议实体结束化，释放相关资源。
 */
void Dslp_Uninit(Dslp* self)
{
}


//*******************************************************************
// ILaiHandle 接口实现

void Dslp_OnReceived(Dslp* self, Byte* data, Uint16 len, Uint8 sourceAddr)
{
    FrameCtrlWord fcw;
    Uint16 chksum = 0;
    ILlHandle* handle;
    Byte* p = data + len - 1;
    Frame* frame;

    // 检验帧的完整性，不完整的帧将被丢弃
    if (len >= 4)
    {
        chksum = *p--;
        chksum <<= 8;
        chksum |= *p;
        if (chksum == MessageDigest_Crc16Ccitt(0, data, len-2))
        {
            fcw.data = data[1];
            handle = self->base.handles[fcw.info.uap];
            if (handle)
            {
                // 生成新帧并向上层提交，逐层传递，最终由顶层（应用层）释放该帧
                // 配置好帧的Pack属性（帧头偏移2），让上层可以直接使用
                frame = Frame_NewPack(2, len - 2 - 2);
                Frame_CopyFrom(frame, data, len);
                handle->OnReceived(handle, frame, sourceAddr);
            }
        }
        else
        {
            TRACE_WARN("\n DSLP CRC Error! size=%d", len);
        }
    }
    else
    {
        TRACE_WARN("\n DSLP Frame Size Error! size=%d", len);
    }
}

Uint16 Dslp_OnSendQuery(Dslp* self)
{
    return Queue_GetCount(&self->transQueue);
}

void Dslp_OnSendRequest(Dslp* self)
{
    ILlHandle* handle;
    Frame* frame;
    Byte* data;
    Lai* lai = self->base.lai;
    Queue* queue = &self->transQueue;
    Uint8 idxUap;

    xSemaphoreTake(s_semDslp, portMAX_DELAY);

    // 把队列中的帧尽可能发送出去
    while (self->isSending == FALSE && (!Queue_IsEmpty(queue)) && lai->comm->IsSendable(lai))
    {
        self->isSending = TRUE;
        frame = Queue_Pop(queue);
        data = Frame_GetData(frame);
        lai->comm->Send(lai, data[0], data, frame->len);
        Frame_Delete(frame);
        self->isSending = FALSE;
    }

    // 剩余空间多于3帧时，才向上层请求新帧
    // 一旦队列满，即时退出
    /// @todo 改为后台定时请求新帧，以提高效率，减少调用层次

    xSemaphoreGive(s_semDslp);

    if (Queue_GetRemain(queue) >= 3 )
    {
        for (idxUap = 0; idxUap < LL_UAP_SIZE; idxUap++)
        {
            handle = self->base.handles[idxUap];
            if (handle && (!Queue_IsFull(queue)))
            {
                handle->OnSendRequest(handle);
            }
        }
    }
}


/** @} */
