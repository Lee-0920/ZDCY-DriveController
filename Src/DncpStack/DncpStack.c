/**
 * @addtogroup module_DncpStack
 * @{
 */

/**
 * @file
 * @brief DNCP 协议栈。
 * @details 为设备构建一个静态全局的协议栈栈体。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-21
 */

#include "DncpStack.h"
#include "Driver/System.h"
#include "DNCP/Ll/LinkFrame.h"
#include "DNCP/Ll/Dslp.h"
#include "DNCP/Net/Trp.h"
#include "DNCP/App/DscpDevice.h"
#include "DNCP/App/OS/DscpScheduler.h"
#include "DevCmdTable.h"
#include "DNCP/Lai/LaiRS485Handler.h"
#include "DNCP/Lai/Driver/LaiRS485SlaveAddr.h"
#include "DNCP/Lai/Driver/LaiRS485Adapter.h"
#include "DNCP/Lai/OS/LaiRS485Scheduler.h"

// 协议栈各层实体对象
static LaiRS485 s_laiDeviceUp;
static Dslp s_llDeviceUp;
static Trp s_netDevice;
static DscpDevice s_dscpDevice;

Bool DncpStack_Init()
{
    // 本地端口号
    Uint8 port;

    // 各层实体初始化
    LaiRS485SlaveAddr_GPIOConfig();
    LaiRS485Adapter_Init();
    LaiRS485Scheduler_Init();

    port = LaiRS485SlaveAddr_GetAddr();

    LaiRS485_Init(&s_laiDeviceUp);
    Dslp_Init(&s_llDeviceUp, (Lai*) &s_laiDeviceUp);
    Trp_Init(&s_netDevice);
    DscpDevice_Init(&s_dscpDevice);

    // 栈结构配置
    Lai_Setup((Lai*) &s_laiDeviceUp, (LinkAddress) port);
    Lai_Register((Lai*) &s_laiDeviceUp, (ILaiHandle*) &s_llDeviceUp);
    Lai_SetMaxTransNum((Lai*) &s_laiDeviceUp, 4);
    Ll_Setup((Ll*) &s_llDeviceUp, (Lai*) &s_laiDeviceUp, port);
    Ll_Register((Ll*) &s_llDeviceUp, NET_PROTOCAL_TRP, (ILlHandle*) &s_netDevice);

    // 网络层：本层级为第3层
    Trp_Setup(&s_netDevice, 1, 1, UPLINK_ADDR_MAKE(3,  0), DOWNLINK_MASK_MAKE(0, 0, 0, 0xFF));
    Trp_Register(&s_netDevice, APP_PROTOCAL_DSCP, (INetHandle*) &s_dscpDevice);
    Trp_AddInterface(&s_netDevice, 0,       // 上行到主控
        UPLINK_ADDR_MAKE(0, 0),
        UPLINK_MASK_MAKE(0, 0),
        (Ll*) &s_llDeviceUp);
    // 网络层：本层级为第2层
//    Trp_Setup(&s_netDevice, 1, 1, UPLINK_ADDR_MAKE(2,  0), DOWNLINK_MASK_MAKE(0, 0, 0xff, 0xFF));
//    Trp_Register(&s_netDevice, APP_PROTOCAL_DSCP, (INetHandle*) &s_dscpDevice);
//    Trp_AddInterface(&s_netDevice, 0,       // 上行到主控
//        UPLINK_ADDR_MAKE(1, 1),
//        UPLINK_MASK_MAKE(0xF, 1),
//        (Ll*) &s_llDeviceUp);
    // 应用层： Device 设备
    DscpDevice_Setup(&s_dscpDevice, (Net*) &s_netDevice, DevCmdTable_GetTable(), DevCmdTable_GetVersion());
    DscpScheduler_Init(&s_dscpDevice);

    // 开启服务
    LaiRS485_Start(&s_laiDeviceUp);
    return TRUE;
}

void DncpStack_Uninit(void)
{
    // 结束各层服务
    LaiRS485_Stop(&s_laiDeviceUp);
    DscpDevice_Uninit(&s_dscpDevice);
    Trp_Uninit(&s_netDevice);
    Dslp_Uninit(&s_llDeviceUp);
    LaiRS485_Uninit(&s_laiDeviceUp);
}

void DncpStack_SetDestAddr(NetAddress destAddr)
{
    DscpDevice_SetDestAddr(&s_dscpDevice, destAddr);
}

Bool DncpStack_SendResp(RespCode resp, void* data, Uint16 len)
{
    return DscpDevice_SendRespEx(&s_dscpDevice, resp, data, len);
}

Bool DncpStack_SendStatus(RespCode resp, StatusCode status)
{
    return DscpDevice_SendStatusEx(&s_dscpDevice, resp, status);
}

Bool DncpStack_SendEvent(EventCode event, void* data, Uint16 len)
{
    return DscpDevice_SendEvent(&s_dscpDevice, event, data, len);
}

void DncpStack_BufferEvent(EventCode event, void* data, Uint16 len)
{
    DscpDevice_BufferEvent(&s_dscpDevice, event, data, len);
}

void DncpStack_ClearBufferedEvent(void)
{
    DscpDevice_ClearBufferedEvent(&s_dscpDevice);
}
/** @} */
