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

#ifndef DNCP_APP_DSCP_DEVICE_H_
#define DNCP_APP_DSCP_DEVICE_H_

#include "Container/Queue.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Net/Trp.h"
#include "AppDefine.h"
#include "Dscp.h"

#ifdef __cplusplus
extern "C" {
#endif

// 前置声明
struct DscpDeviceStruct;

/**
 * @brief DSCP 命令处理接口函数。
 * @param[in] dscp DSCP协议体对象。
 * @param[in] data 命令包的参数数据。
 * @param[in] len 命令包的参数数据。
 * @param[in] source 发送该命令包的源地址。
 * @note 协议栈会根据接收的命令自动调用该函数。
 *  <p> 如果需要回应，并且在命令处理函数返回前可以得到执行结果，
 *  那么可以直接调用 DscpDevice_SendResp() 发送回应，此时不用处理源地址，
 *  因为 DscpDevice_SendResp() 会自动处理这些细节。 DscpDevice_SendStatus() 类似。
 *  <p> 如果在命令处理函数返回前不能得到结果，应先调用 DscpDevice_GetSourceAddr()
 *  获取源地址并缓存，等到执行结果出来后，再调用 DscpDevice_SetDestAddr() 设置目的地址，
 *  再调用具体的发送函数。
 * @attention 从 @p param 中取参数时，请注意CPU字对齐问题。本协议栈不能保证
 *  @p param 指向的数据是16位或32位对齐的，为安全起见，请使用循环单字节操作。
 */
typedef void (*IfDscpCmdHandle)(struct DscpDeviceStruct* dscp, Byte* data, Uint16 len);


/**
 * @brief DSCP 命令表结点
 */
typedef struct DscpCmdEntryStruct
{
    CmdCode cmdCode;                ///< 命令码
    IfDscpCmdHandle handle;         ///< 命令处理函数
}
DscpCmdEntry;

typedef DscpCmdEntry* DscpCmdTable;    ///< 命令表类型。

/**
 * @brief 定义一个命令结点，指定命令码对应的处理函数。
 * @param[in] code 命令码。
 * @param[in] handle 处理函数。
 */
#define DSCP_CMD_ENTRY(code, handle)    {code, (IfDscpCmdHandle)handle}

/**
 * @brief 定义一个命令表的结束结点。
 */
#define DSCP_CMD_ENTRY_END              {0, 0}


//*******************************************************************
// DscpDevice 定义

/**
 * @brief DscpDevice 实体错误统计。
 * @details 统计 DscpDevice 协议实体的运行情况。
 */
typedef struct DscpDeviceStatisticsStruct
{
    unsigned int recvCmds;              ///< 接收的命令包统计
    unsigned char cmdQueueOverflows;    ///< 命令队列溢出造成命令被丢的次数
    unsigned char transQueueOverflows;  ///< 发送队列溢出造成命令被丢的次数
    unsigned char outOfRamError;        ///< 内存分配失败（不足）的错误数
    unsigned short sendFailures;        ///< 发送失败次数
}
DscpDeviceStatistics;

/**
 * @brief 被缓存的事件
 */
typedef struct DscpDeviceEventBufferStruct
{
    EventCode event;                    ///< event 要发送的事件码。
    Uint16 len;                         ///< len 事件的参数长度。
    Byte data[255];                     ///< sendBuf 事件的参数数据。
}DscpDeviceEventBuffer;

/**
 * @brief 应用层 DscpDevice 实体对象。
 */
typedef struct DscpDeviceStruct
{
    INetHandle netHandle;               ///< 下层处理接口，必须在结构初始化处定义。具体实现时必须初始化它
    Net* net;                           ///< 底层通信实体
    DscpCmdEntry* cmdTable;             ///< 应用命令表，以0结束尾表项
    DscpVersion version;                ///< DSCP接口的版本号
    Queue transQueue;                   ///< 发送报文队列，包括命令、回应、事件
    Frame* transFrames[32];             ///< 发送报文队列的缓冲
    Queue cmdQueue;                     ///< 接收的命令队列
    Frame* cmdFrames[16];               ///< 接收的命令队列的缓冲

    Uint16 curCmdCode;                  ///< 当前处理中的命令码
    Uint8 sourceAddr;                   ///< 当前处理中的命令的源地址，上行地址
    Uint8 packOffset;                   ///< 当前处理中的命令的包偏移

    DscpDeviceStatistics dscpStat;      ///< DSCP 统计信息

    DscpDeviceEventBuffer bufferedEvent;///< 被缓存的事件
}
DscpDevice;

//*******************************************************************
// DSCP 实现相关

extern void DscpDevice_Init(DscpDevice* self);
extern void DscpDevice_Uninit(DscpDevice* self);
extern void DscpDevice_Setup(DscpDevice* self, Net* netToUse, DscpCmdTable cmdTable, DscpVersion version);

extern void DscpDevice_Handle(DscpDevice* self);

extern Bool DscpDevice_SendResp(DscpDevice* self, void* data, Uint16 len);
extern Bool DscpDevice_SendRespEx(DscpDevice* self, RespCode resp, void* data, Uint16 len);
extern Bool DscpDevice_SendStatus(DscpDevice* self, StatusCode status);
extern Bool DscpDevice_SendStatusEx(DscpDevice* self, RespCode resp, StatusCode status);
extern Bool DscpDevice_SendEvent(DscpDevice* self, EventCode event, void* data, Uint16 len);
extern Bool DscpDevice_Send(DscpDevice* self, Uint8 dscpType, Uint16 code, void* data, Uint16 len, Bool needFollow);

extern NetAddress DscpDevice_GetSourceAddr(DscpDevice* self);
extern void DscpDevice_SetDestAddr(DscpDevice* self, NetAddress destAddr);

extern void DscpDevice_BufferEvent(DscpDevice* self, EventCode event, void* data, Uint16 len);
extern void DscpDevice_ClearBufferedEvent(DscpDevice* self);
#ifdef __cplusplus
}
#endif

#endif  // DNCP_APP_DSCP_DEVICE_H_

/** @} */
