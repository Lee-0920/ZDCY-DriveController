/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 设备简单控制协议（控制器端）。
 * @details 
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-3-4
 */

#include <string.h>
#include "DscpSysDefine.h"
#include "DscpController.h"
#include "Driver/system.h"
#include "Common/Utils.h"
#include "Common/System.h"


// DSCP 包的偏移量（路由通道）。LL层2字节，TRP层5字节
#define DSCP_PACK_OFFSET        (2 + 5)

// 超时参数设置值，单们为毫秒
#define DSCP_SEND_TIME_OUT      1000
#define DSCP_RECV_TIME_OUT      1000

// 挂起睡眠间隔，单位为毫秒
#define DSCP_SLEEP_INTERVAL     10

// 发送状态
#define SEND_STATUS_SENDING              1      // 正在发送中
#define SEND_STATUS_SENDOUT              2      // 成功发送完
#define SEND_STATUS_SENDFAULT            3      // 网络层发送错误


//*******************************************************************
// ILaiHandle 接口声明

static void DscpController_OnReceived(DscpController* self, Frame* frame, NetAddress addr);
static void DscpController_OnSendReport(DscpController* self, Bool isSendout);


//*******************************************************************
// DscpController 实现相关

/**
 * @brief DscpController 协议实体初始化。
 * @param[in] self 自身对象。
 */
void DscpController_Init(DscpController* self)
{
    memset(self, 0, sizeof(DscpController));

    // 初始化各接口
    self->netHandle.OnReceived = (IfNetOnReceived) DscpController_OnReceived;
    self->netHandle.OnSendReport = (IfNetOnSendReport) DscpController_OnSendReport;

    // 队列初始化
    Queue_Init(&self->recvQueue, (void**)self->recvFrames, sizeof(self->recvFrames)/sizeof(Frame*));
}

/**
 * @brief DscpController 配置。
 * @param[in] self 自身对象。
 * @param[in] netToUse 使用的Net实体，请确保已经初始化（Init()调用）。
 */
void DscpController_Setup(DscpController* self, Net* netToUse)
{
    self->net = netToUse;
}

/**
 * @brief DscpController 协议实体结束化，释放相关资源。
 */
void DscpController_Uninit(DscpController* self)
{
}

/**
 * @brief 向DSCP设备发送命令。
 * @details 阻塞调用，发送成功或失败后才返回。
 * @param[in] self Dscp 实体对象。
 * @param[in] addr 目标设备的网络地址。
 * @param[in] cmd 要发送的命令应码。
 * @param[in] data 命令的参数数据。
 * @param[in] len 命令的参数长度。
 * @return 命令是否成功发送。
 *  <p> 该返回值只是指示命令是否成功被目标设备接收，而不指示该命令的执行情况（回应）。
 */
Bool DscpController_SendCmd(DscpController* self, NetAddress addr, CmdCode cmd, void* data, Uint16 len)
{
    Bool ret = FALSE;
    INetComm* comm = self->net->comm;
    Frame* frame;
    Byte* frameData;
    Byte* p;
    int timeOutCount = DSCP_SEND_TIME_OUT / DSCP_SLEEP_INTERVAL;

    self->dscpStat.sendCmds++;

    // 分配数据包内存
    frame = Frame_NewPack(DSCP_PACK_OFFSET, (Uint8)len + 3);
    if (frame)
    {
        // 建立包头
        frameData = Frame_GetPackData(frame);
        // 拷贝控制字
        *frameData = 0;
        ((DscpCtrlWord*) frameData)->type = DSCP_TYPE_CMD;
        // 拷贝码值
        p = (Byte*) &cmd;
        ++frameData;
        *frameData++ = *p++;
        *frameData++ = *p;
        // 拷贝参数
        p = data;
        while (len--)
        {
            *frameData++ = *p++;
        }

        // 一直尝试发送，直到超时
        while (timeOutCount--)
        {
            if (comm->IsSendable(self->net))
            {
                self->sendStatus = SEND_STATUS_SENDING;
                comm->Send(self->net, APP_PROTOCAL_DSCP, addr, frame);
                break;
            }
            System_Sleep(DSCP_SLEEP_INTERVAL);
        }

        // 未超时
        if (timeOutCount > 0)
        {
            // 已交由网络层发送中，等待发送报告
            while (timeOutCount--)
            {
                if (self->sendStatus != SEND_STATUS_SENDING)
                {
                    if (self->sendStatus== SEND_STATUS_SENDOUT)
                    {
                        // 成功发送出去
                        ret = TRUE;
                    }
                    break;
                }
                System_Sleep(DSCP_SLEEP_INTERVAL);
            }
        }
        else
        {
            // 发送超时
            Frame_Delete(frame);
        }
    }
    else
    {
        self->dscpStat.outOfRamError++;
    }

    if (ret == FALSE)
    {
        self->dscpStat.sendFailures++;
    }

    return ret;
}

/**
 * @brief 查询是否有数据包到达。
 * @details DscpController 内部有一个接收队列，收到的DSCP包将暂时被缓冲起来，
 *  客户程序调用本函数可以查询队列中是否有数据。如果是，再调用
 *  DscpController_Receive() 收取数据包。
 *  <p>非阻塞调用，立即返回结果。
 * @param[in] self Dscp 实体对象。
 * @return 是否有数据包到达。
 */
Bool DscpController_IsReceivable(DscpController* self)
{
    return (!Queue_IsEmpty(&self->recvQueue));
}

/**
 * @brief 从接收缓冲中读取一个数据包。
 * @param[in] self Dscp 实体对象。
 * @param[out] addr 发送该数据包的源地址。
 * @param[out] code 数据包的码值。
 * @param[out] data 用于保存参数的缓冲，必须大于等于256字节。
 * @param[out] len 数据包参数的长度。
 * @return 到达的数据包的类型，可能的值有：
 *  - @ref DSCP_PACK_TYPE_ERROR 出错，无数据包
 *  - @ref DSCP_PACK_TYPE_CMD 命令包
 *  - @ref DSCP_PACK_TYPE_RESP 回应包
 *  - @ref DSCP_PACK_TYPE_EVENT 事件包
 * @note 有线程安全性隐患，不能同时有多线程调用本函数。
 */
Uint8 DscpController_Receive(DscpController* self, NetAddress* addr, DscpCode* code, Byte* data, Uint16* len)
{
    Uint8 ret = (Uint8) DSCP_PACK_TYPE_ERROR;
    Frame* frame;
    DscpCtrlWord dcw;
    Byte* frameData;
    Byte* p;
    Uint16 count;

    if (!Queue_IsEmpty(&self->recvQueue))
    {
        frame = Queue_Pop(&self->recvQueue);

        // 包解析
        frameData = Frame_GetPackData(frame);

        // 源地址解析
        // 注意：包头往前的四个字节重新定义为源地址
        frameData -= 4;
        p = (Byte*) addr;
        count = 4;
        while (count--)
        {
            *p++ = *frameData++;
        }

        *((Byte*) &dcw) = *frameData++;

        // 命令码值
        p = (Byte*) code;
        *p++ = *frameData++;
        *p++ = *frameData++;

        // 参数长度
        count = Frame_GetPackSize(frame) - 3;
        *len = count;
        while (count--)
        {
            *data++ = *frameData++;
        }

        // 返回包类型
        ret = dcw.type;

        Frame_Delete(frame);
    }

    return ret;
}

//*******************************************************************
// INetHandle 接口实现

void DscpController_OnReceived(DscpController* self, Frame* frame, NetAddress addr)
{
    Byte* data;
    NetAddress sourceAddr;
    Byte* p;
    Uint16 len;

    // 包解析
    data = Frame_GetPackData(frame);

    // 因为所有数据包都是由DSCP设备向上发的，源地址只可能是下行地址
    sourceAddr = NET_ADDRESS_DOWNLINK_GET(addr);

    // 原来的数据包直接当作命令项插入命令队列，以减少堆内存操作
    // 注意：包头往前的四个字节重新定义为源地址
    data -= 4;
    p = (Byte*) &sourceAddr;
    len = 4;
    while (len--)
    {
        *data++ = *p++;
    }

    if (!Queue_IsFull(&self->recvQueue))
    {
        Queue_Push(&self->recvQueue, frame);
    }
    else
    {
        Frame_Delete(frame);

        // 统计信息
        self->dscpStat.recvQueueOverflows++;
    }
}

void DscpController_OnSendReport(DscpController* self, Bool isSendout)
{
    if (isSendout)
    {
        self->sendStatus = SEND_STATUS_SENDOUT;
    }
    else
    {
        // 未发出，UI通知
        self->dscpStat.sendFailures++;
        self->sendStatus = SEND_STATUS_SENDFAULT;
    }
}


/** @} */
