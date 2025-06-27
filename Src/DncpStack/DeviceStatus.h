/**
 * @file
 * @brief 设备状态接口头文件
 * @details
 * @version 1.0.0
 * @author lemon.xiaoxun
 * @date 2013-5-18
 */

#ifndef DEVICESTATUS_H_
#define DEVICESTATUS_H_


#include "LuipApi/DeviceStatusInterface.h"
#include "DNCP/App/DscpDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

//声明处理函数
void DeviceStatus_BlinkDevice(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceStatus_GetRunMode(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceStatus_GetPowerSupplyType(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceStatus_Initialize(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceStatus_Reset(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceStatus_ReportResetEvent(DeviceResetCause resetCause);
//命令入口，全局宏定义：每一条命令对应一个命令码和处理函数
#define CMD_TABLE_DEVICE_STATUS \
        DSCP_CMD_ENTRY(DSCP_CMD_DSI_GET_RUN_MODE, DeviceStatus_GetRunMode), \
        DSCP_CMD_ENTRY(DSCP_CMD_DSI_BLINK_DEVICE, DeviceStatus_BlinkDevice), \
        DSCP_CMD_ENTRY(DSCP_CMD_DSI_GET_POWER_SUPPLY_TYPE, DeviceStatus_GetPowerSupplyType),\
        DSCP_CMD_ENTRY(DSCP_CMD_DSI_INITIALIZE, DeviceStatus_Initialize),\
        DSCP_CMD_ENTRY(DSCP_CMD_DSI_RESET, DeviceStatus_Reset)
#ifdef __cplusplus
}
#endif
#endif /* DEVICESTATUS_H_ */
