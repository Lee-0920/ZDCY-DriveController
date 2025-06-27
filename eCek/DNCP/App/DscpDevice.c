/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 设备简单控制协议（设备服务端）。
 * @details 
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-26
 */

#include <string.h>
#include "DscpSysDefine.h"
#include "DscpDevice.h"
#include "Tracer/Trace.h"
#include "Common/Utils.h"
#include "OS/DscpScheduler.h"
#include "Common/System.h"

#include "FreeRTOS.h"
#include "semphr.h"

// DSCP 包的偏移量（路由通道）。LL层2字节，TRP层5字节
#define DSCP_PACK_OFFSET        (2 + 5)

//*******************************************************************
// ILaiHandle 接口声明

static void DscpDevice_OnReceived(DscpDevice* self, Frame* frame, NetAddress addr);
static void DscpDevice_OnSendReport(DscpDevice* self, Bool isSendout);

static xSemaphoreHandle s_SemapDscpDevice;

//*******************************************************************
// DscpDevice 实现相关


/**
 * @brief DscpDevice 协议实体初始化。
 * @param[in] self 自身对象。
 */
void DscpDevice_Init(DscpDevice* self)
{
    memset(self, 0, sizeof(DscpDevice));

    vSemaphoreCreateBinary(s_SemapDscpDevice);

    // 初始化各接口
    self->netHandle.OnReceived = (IfNetOnReceived) DscpDevice_OnReceived;
    self->netHandle.OnSendReport = (IfNetOnSendReport) DscpDevice_OnSendReport;

    // 队列初始化
    Queue_Init(&self->cmdQueue, (void**)self->cmdFrames, sizeof(self->cmdFrames)/sizeof(Frame*));
    Queue_Init(&self->transQueue, (void**)self->transFrames, sizeof(self->transFrames)/sizeof(Frame*));


}

/**
 * @brief DscpDevice 配置。
 * @param[in] self 自身对象。
 * @param[in] netToUse 使用的Net实体，请确保已经初始化（Init()调用）。
 * @param[in] cmdTable 使用的应用命令表，结束表项应为0值。
 *  DSCP收到命令时，将自动调用对应的命令处理函数。
 * @param[in] version 当前DSCP应用接口的版本号。
 */
void DscpDevice_Setup(DscpDevice* self, Net* netToUse, DscpCmdTable cmdTable, DscpVersion version)
{
    self->net = netToUse;

    self->cmdTable = cmdTable;
    self->version = version;
}

/**
 * @brief DscpDevice 协议实体结束化，释放相关资源。
 */
void DscpDevice_Uninit(DscpDevice* self)
{
}

/**
 * @brief 发送回应，简化版本，使用默认的命令码作为回应码。
 * @details 在发送回应时，该本函数可以不用指定回应码，这样系统将使用当前
 *  命令处理函数对应的命令码作为回应码，前提是只能在命令处理函数中调用。
 * @param[in] self Dscp 实体对象。
 * @param[in] data 回应的参数数据。
 * @param[in] len 回应的参数长度。
 * @note 本函数只能在命令处理函数中调用。如果在其它非命令处理线程中调用，
 *  或者需要指定其它的回应码，请调用 DscpDevice_SendRespEx() 。
 * @see DscpDevice_SendRespEx
 */
Bool DscpDevice_SendResp(DscpDevice* self, void* data, Uint16 len)
{
    TRACE_MARK("\n#DscpSendResp[%04x]L%d", self->curCmdCode, len);
    return DscpDevice_Send(self, DSCP_TYPE_RESP_INFO, self->curCmdCode, data, len, FALSE);
}

/**
 * @brief 发送回应，扩展版本，指定回应码。
 * @param[in] self Dscp 实体对象。
 * @param[in] resp 要返回的回应码。回应码通常情况下是命令码。
 * @param[in] data 回应的参数数据。
 * @param[in] len 回应的参数长度。
 * @note 本函数通常在命令处理函数中调用。在其它非命令处理线程中调用时，
 *  必须先调用 DscpDevice_SetDestAddr() 设置目的地址。
 * @see DscpDevice_SetDestAddr
 */
Bool DscpDevice_SendRespEx(DscpDevice* self, RespCode resp, void* data, Uint16 len)
{
    TRACE_MARK("\n#DscpSendResp[%04x]L%d", resp, len);
    return DscpDevice_Send(self, DSCP_TYPE_RESP_INFO, resp, data, len, FALSE);
}

/**
 * @brief 发送状态回应，简化版本，使用默认的命令码作为回应码。
 * @param[in] self Dscp 实体对象。
 * @param[in] status 要返回的状态码。
 * @note 本函数只能在命令处理函数中调用。如果在其它非命令处理线程中调用，
 *  或者需要指定其它的回应码，请调用 DscpDevice_SendStatusEx() 。
 * @see DscpDevice_SendStatusEx
 */
Bool DscpDevice_SendStatus(DscpDevice* self, StatusCode status)
{
    TRACE_MARK("\n#DscpSendStatus[%04x]S%04x", self->curCmdCode, status);
    return DscpDevice_Send(self, DSCP_TYPE_RESP_STATUS, self->curCmdCode, &status, sizeof(status), FALSE);
}

/**
 * @brief 发送状态回应，扩展版本，指定回应码。
 * @param[in] self Dscp 实体对象。
 * @param[in] resp 要返回的回应码。回应码通常情况下是命令码。
 * @param[in] status 要返回的状态码。
 * @note 本函数通常在命令处理函数中调用。在其它非命令处理线程中调用时，
 *  必须先调用 DscpDevice_SetDestAddr() 设置目的地址。
 * @see DscpDevice_SetDestAddr
 */
Bool DscpDevice_SendStatusEx(DscpDevice* self, RespCode resp, StatusCode status)
{
    TRACE_MARK("\n#DscpSendStatus[%04x]S%04x", resp, status);
    return DscpDevice_Send(self, DSCP_TYPE_RESP_STATUS, self->curCmdCode, &status, sizeof(status), FALSE);
}

/**
 * @brief 发送事件。
 * @param[in] self Dscp 实体对象。
 * @param[in] event 要发送的事件码。
 * @param[in] data 事件的参数数据。
 * @param[in] len 事件的参数长度。
 * @note 事件将发往最顶层DSCP控制器。
 */
Bool DscpDevice_SendEvent(DscpDevice* self, EventCode event, void* data, Uint16 len)
{
    // 发往顶层控制器
    self->sourceAddr = UPLINK_ADDR_MAKE(1, 0xF);    // 向最顶层广播
    self->packOffset = DSCP_PACK_OFFSET;
    TRACE_MARK("\n#DscpSendEvent[%04x]L%d", event, len);
    return DscpDevice_Send(self, DSCP_TYPE_EVENT, event, data, len, FALSE);
}

/**
 * @brief 缓存发送的事件。
 * @param[in] self Dscp 实体对象。
 * @param[in] event 要缓存的事件码。
 * @param[in] data 事件的参数数据。
 * @param[in] len 事件的参数长度。
 * @note 与 @ref DscpDevice_SendEvent 配合使用，常用于发送重要的事件，调用此函数缓存，只能缓存一个事件。
 *      在接收节点没有接收到事件时，使用 @ref DSCP_SYSCMD_ACQUIRE_EVENT 获取被缓存的事件。
 */
void DscpDevice_BufferEvent(DscpDevice* self, EventCode event, void* data, Uint16 len)
{
    self->bufferedEvent.event = event;
    self->bufferedEvent.len = len;
    memcpy(self->bufferedEvent.data, data, len);
    TRACE_MARK("\n#DscpBufferEvent[%04x]L%d", event, len);
}

/**
 * @brief 清空缓存事件的区域。
 * @param[in] self Dscp 实体对象。
 */
void DscpDevice_ClearBufferedEvent(DscpDevice* self)
{
    TRACE_MARK("\n#DscpClearBufferedEvent[%04x]L%d", self->bufferedEvent.event, self->bufferedEvent.len);
    memset(self->bufferedEvent.data, 0, self->bufferedEvent.len);
    self->bufferedEvent.event = 0;
    self->bufferedEvent.len = 0;
}

/**
 * @brief 系统命令：获取缓存的事件。
 * @details DscpDevice 在发送事件前，可先把事件缓冲起来，上位机等不到事件时（事件有丢失），
    可以主动调用本命令，让下位机再发一次事件。此时上位机应该及时再次等待事件，并作解析。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功，稍候将把缓冲的事件重发；
 *  - @ref DSCP_ERROR 操作失败，缓冲中无事件；
 *  - @ref DSCP_NOT_SUPPORTED 没有实现本命令，上下位机的版本兼容可能出现问题。
 * @note 本机制为可选机制，在需要更可靠的事件时可实现本机制。
 */
void DscpDevice_AcquireEvent(DscpDevice* self)
{
    Uint16 ret = DSCP_OK;
    if (self->bufferedEvent.event == 0 && self->bufferedEvent.len == 0)
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n#DscpAcquireEvent Buffer empty!");
    }
    DscpDevice_SendStatus(self, ret);
    if (DSCP_OK == ret)
    {
        System_Sleep(2000);
        DscpDevice_SendEvent(self, self->bufferedEvent.event, self->bufferedEvent.data, self->bufferedEvent.len);
    }
}

/**
 * @brief 发送回包，各参数可灵活设置。
 * @param[in] self Dscp 实体对象。
 * @param[in] dscpType DSCP 回包类型，可以是下面类型的一种：
 *  - @ref DSCP_TYPE_CMD
 *  - @ref DSCP_TYPE_RESP_INFO
 *  - @ref DSCP_TYPE_RESP_STATUS
 *  - @ref DSCP_TYPE_EVENT
 * @param[in] code 要返回的回包码值，通常是回应码/命令码。
 * @param[in] data 回包的参数数据。
 * @param[in] len 回包的参数长度。
 * @param[in] needFollow 后面是否跟随其它回包。该字段仅在发送回应包时有效。
 *  通常情况下，一条命令调用仅返回一个回应包，所以 @p needFollow 设为 FALSE 即可。
 *  但有些情况下，一条命令调用可能会连续返回多个数据包，在发送中间包时 @p needFollow
 *  应设置为 TRUE ，表示后面还有数据包待发送；发送这条命令调用的最后一个回包时，
 *  设置 @p needFollow 为FALSE，表示该包是最后一包。
 * @return 发送是否成功。
 */
Bool DscpDevice_Send(DscpDevice* self, Uint8 dscpType, Uint16 code, void* data, Uint16 len, Bool needFollow)
{
    Bool ret = TRUE;
    INetComm* comm = self->net->comm;
    Frame* frame;
    Byte* frameData;
    Queue* queue = &self->transQueue;

    xSemaphoreTake(s_SemapDscpDevice, portMAX_DELAY);

    // 分配数据包内存
    frame = Frame_NewPack(self->packOffset, (Uint8)len + 3);
    if (frame)
    {
        // 建立包头
        frameData = Frame_GetPackData(frame);
        // 拷贝控制字
        *frameData = 0;
        ((DscpCtrlWord*) frameData)->type = dscpType;
        ((DscpCtrlWord*) frameData)->needFollow = needFollow;
        // 拷贝码值
        memcpy(frameData + 1, &code, sizeof(code));
        // 拷贝参数
        memcpy(frameData + 3, data, len);

        // 按照队列的传输方式，先进先出
        if (!Queue_IsEmpty(queue))
        {
            // 队列中有数据待发送，本次数据压入队列，并尝试发送队列中的数据
            if (!Queue_IsFull(queue))
            {
                // 队列未满，本次数据包压入队列
                Queue_Push(queue, frame);
            }
            else
            {
                // 队列已满，数据包溢出
                ret = FALSE;
                self->dscpStat.transQueueOverflows++;
                TRACE_ERROR("\nDscp Device QueueOverflows:%d", self->dscpStat.transQueueOverflows);

                Frame_Delete(frame);
            }
            // 尝试发送队列中的数据包
            frame = Queue_Peek(queue);
            data = Frame_GetData(frame);
            if (comm->IsSendable(self->net))
            {
                frame = Queue_Pop(queue);
                comm->Send(self->net, APP_PROTOCAL_DSCP, self->sourceAddr, frame);
                TRACE_MARK("->%d", self->sourceAddr);
            }
        }
        else
        {
            // 队列为空，如果Trp空闲，则发送数据包
            if (comm->IsSendable(self->net))
            {
                comm->Send(self->net, APP_PROTOCAL_DSCP, self->sourceAddr, frame);
                TRACE_MARK("->%d", self->sourceAddr);
            }
            else
            {
                // 底层接口繁忙则保存到队列
                Queue_Push(&self->transQueue, frame);
            }
        }
    }
    else
    {
        ret = FALSE;
        self->dscpStat.outOfRamError++;
        TRACE_ERROR(" OutOfRAM:%d", self->dscpStat.outOfRamError);
    }

    xSemaphoreGive(s_SemapDscpDevice);

    return ret;
}

/**
 * @brief 获取当前命令包的源地址。
 * @param[in] self Dscp 实体对象。
 * @return 数据包源地址。
 */
NetAddress DscpDevice_GetSourceAddr(DscpDevice* self)
{
    NetAddress addr = NET_ADDRESS_UPLINK_MAKE(self->sourceAddr);
    return addr;
}

/**
 * @brief 设置回传数据包的目的地址。
 * @param[in] self Dscp 实体对象。
 * @param[in] destAddr 目的地址。
 */
void DscpDevice_SetDestAddr(DscpDevice* self, NetAddress destAddr)
{
    self->sourceAddr = (Uint8) NET_ADDRESS_UPLINK_GET(destAddr);
}

//*******************************************************************
// INetHandle 接口实现

void DscpDevice_OnReceived(DscpDevice* self, Frame* frame, NetAddress addr)
{
    Byte* data;
    DscpCtrlWord dcw;
    Uint8 layerAddr;

    TRACE_MARK("\n#DscpRecv");

    // 包解析
    data = Frame_GetPackData(frame);
    *((Byte*) &dcw) = *data;

    // DscpDevice 只处理命令，其它的都丢弃
    if (dcw.type == DSCP_TYPE_CMD)
    {
        // 因为所有命令都是由DSCP控制器向下发的，源地址只可能是上行地址
        layerAddr = (Uint8) NET_ADDRESS_UPLINK_GET(addr);
        TRACE_MARK("From%d", layerAddr);

        // 原来的数据包直接当作命令项插入命令队列，以减少堆内存操作
        // 注意：包头的首字节（原为控制字）重新定义为源地址
        *data = layerAddr;

        if (!Queue_IsFull(&self->cmdQueue))
        {
            Queue_Push(&self->cmdQueue, frame);
            // 让后台任务去处理具体的DSCP命令，最终将调用 DscpDevice_Handle()
            DscpScheduler_Active();
        }
        else
        {
            Frame_Delete(frame);

            // 统计信息
            self->dscpStat.cmdQueueOverflows++;
            TRACE_ERROR(" QueueOverflows:%d", self->dscpStat.cmdQueueOverflows);
        }
    }
    else
    {
        // 顶端释放
        Frame_Delete(frame);
    }
}

/**
 * @brief DSCP事务处理。
 * @details 调用该函数会处理队列中的所有命令，如无命令则立即返回。
 * @note 上层应该持续调用本函数，调用代码可放在系统主循环、定时中断或者其它任务中。
 * @attention 线程安全性：非线程安全。
 */
void DscpDevice_Handle(DscpDevice* self)
{
    Frame* frame;
    Byte* data;
    Byte* p;
    CmdCode cmd;
    Uint16 len;
    DscpCmdEntry* cmdEntry;
    // 处理队列中存在的所有命令
    while (!Queue_IsEmpty(&self->cmdQueue))
    {
        frame = Queue_Pop(&self->cmdQueue);
        // 解析命令包
        len = Frame_GetPackSize(frame) - 3;     // 参数长度
        data = Frame_GetPackData(frame);
        self->sourceAddr = *data++;             // 缓存源地址
        self->packOffset = frame->packOffset;   // 缓存包数据偏移
        p = (Byte*) &cmd;   // 解析命令
        *p++ = *data++;
        *p = *data++;
        self->curCmdCode = cmd;
        // 此时 data 指向参数域

        TRACE_MARK("\n#DscpCmd[%04x]", cmd);

        // 首先处理系统保留命令
        switch(cmd)
        {
        case DSCP_SYSCMD_ECHO:          // 回声测试命令
            // 原样返回
            DscpDevice_SendRespEx(self, cmd, data, len);
            break;

        case DSCP_SYSCMD_IFVER_GET:     // 接口版本查询命令
            DscpDevice_SendRespEx(self, cmd, &self->version, sizeof(self->version));
            break;

        case DSCP_SYSCMD_ACQUIRE_EVENT: //获取缓存的事件
            DscpDevice_AcquireEvent(self);
            break;

        default:
            // 查命令表，并执行命令处理函数
            cmdEntry = self->cmdTable;
            while(cmdEntry->handle)
            {
                if (cmdEntry->cmdCode == cmd)
                {
                    // 调用命令处理函数
                    cmdEntry->handle(self, data, len);
                    TRACE_MARK("Match");
                    break;
                }
                cmdEntry++;
            }
            /// @todo 未在命令表中找到命令，需要特殊处理
            if (0 == cmdEntry->handle)
                DscpDevice_SendStatus(self,(Uint16)DSCP_NOT_SUPPORTED);
            break;
        }

        // 顶端释放
        Frame_Delete(frame);
    }
}

void DscpDevice_OnSendReport(DscpDevice* self, Bool isSendout)
{
    Frame* frame;
    INetComm* comm = self->net->comm;
    Queue* queue = &self->transQueue;

    // 未发出，UI通知
    if (!isSendout)
    {
        self->dscpStat.sendFailures++;
    }

    xSemaphoreTake(s_SemapDscpDevice, portMAX_DELAY);

    // 已发送一帧，如果发送队列中有数据，则尝试将数据都发送
    while ((!Queue_IsEmpty(queue)) && (comm->IsSendable(self->net)))
    {
        TRACE_MARK("\n#DscpTry");
        frame = Queue_Pop(queue);

        comm->Send(self->net, APP_PROTOCAL_DSCP, self->sourceAddr, frame);
    }

    xSemaphoreGive(s_SemapDscpDevice);
}


/** @} */
