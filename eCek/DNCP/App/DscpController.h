/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 设备简单控制协议（控制器端）。
 * @details 本模块仅适用于部署有分时操作系统的上位机。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-3-4
 */

#ifndef DNCP_APP_DSCP_CONTROLLER_H_
#define DNCP_APP_DSCP_CONTROLLER_H_

#include "Container/Queue.h"
#include "DNCP/Base/Frame.h"
#include "DNCP/Net/Trp.h"
#include "AppDefine.h"
#include "Dscp.h"

#ifdef __cplusplus
extern "C" {
#endif



//*******************************************************************
// DscpController 定义


/**
 * @brief DscpController 实体错误统计。
 * @details 统计 DscpController 协议实体的运行情况。
 */
typedef struct DscpControllerStatisticsStruct
{
    unsigned int sendCmds;              ///< 发送的命令包统计
    unsigned int recvPacks;             ///< 接收到的数据包统计
    unsigned char recvQueueOverflows;   ///< 发送队列溢出造成命令被丢的次数
    unsigned char outOfRamError;        ///< 内存分配失败（不足）的错误数
    unsigned short sendFailures;        ///< 发送失败次数
}
DscpControllerStatistics;


/**
 * @brief 应用层 DscpController 实体对象。
 */
typedef struct DscpControllerStruct
{
    INetHandle netHandle;           ///< 下层处理接口，必须在结构初始化处定义。具体实现时必须初始化它
    Net* net;                       ///< 底层通信实体
    Queue recvQueue;                ///< 接收的数据包队列
    Frame* recvFrames[32];          ///< 接收的数据包队列的缓冲
    Uint8 sendStatus;               ///< 发送状态

    DscpControllerStatistics dscpStat;  ///< DSCP 统计信息
}
DscpController;

//*******************************************************************
// DSCP 实现相关

#define DSCP_PACK_TYPE_ERROR            -1
#define DSCP_PACK_TYPE_CMD              0   ///< Dscp包类型：命令包
#define DSCP_PACK_TYPE_RESP_INFO        1   ///< Dscp包类型：信息回应包，数据字段的格式由应用决定
#define DSCP_PACK_TYPE_RESP_STATUS      2   ///< Dscp包类型：状态回应包，数据字段是一个16位的状态值
#define DSCP_PACK_TYPE_EVENT            3   ///< Dscp包类型：事件包

extern void DscpController_Init(DscpController* self);
extern void DscpController_Uninit(DscpController* self);
extern void DscpController_Setup(DscpController* self, Net* netToUse);

extern Bool DscpController_SendCmd(DscpController* self, NetAddress addr, CmdCode cmd, void* data, Uint16 len);
extern Bool DscpController_IsReceivable(DscpController* self);
extern Uint8 DscpController_Receive(DscpController* self, NetAddress* addr, DscpCode* code, Byte* data, Uint16* len);

#ifdef __cplusplus
}
#endif

#endif  // DNCP_APP_DSCP_CONTROLLER_H_

/** @} */
