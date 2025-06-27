/**
 * @file
 * @brief   设备信息头文件
 * @details 
 * @version 1.0.0
 * @author lemon.xiaoxun
 * @date 2013-5-17
 */

#ifndef DEVICEINFO_H_
#define DEVICEINFO_H_

#include "LuipApi/DeviceInfoInterface.h"
#include "DNCP/App/DscpDevice.h"

#ifdef __cplusplus
extern "C" {
#endif
void DeviceInfo_Init(void);
void DeviceInfo_GetType(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_SetType(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetSn(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_SetSn(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetModel(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_SetModel(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetSoftwareVersion(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetHardwareVersion(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetSoftwareLabel(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetDate(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_SetDate(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_GetManufacter(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceInfo_SetManufacter(DscpDevice* dscp, Byte* data, Uint16 len);


#define CMD_TABLE_DEVICE_INFO \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_TYPE, DeviceInfo_GetType), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_SET_TYPE, DeviceInfo_SetType), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_MODEL, DeviceInfo_GetModel), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_SET_MODEL, DeviceInfo_SetModel), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_SN, DeviceInfo_GetSn),\
    DSCP_CMD_ENTRY(DSCP_CMD_DII_SET_SN, DeviceInfo_SetSn), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_MANUF_DATE, DeviceInfo_GetDate), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_SET_MANUF_DATE, DeviceInfo_SetDate), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_MANUFACTURER, DeviceInfo_GetManufacter), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_SET_MANUFACTURER, DeviceInfo_SetManufacter), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_SOFTWARE_VERSION, DeviceInfo_GetSoftwareVersion), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_SOFTWARE_LABEL, DeviceInfo_GetSoftwareLabel), \
    DSCP_CMD_ENTRY(DSCP_CMD_DII_GET_HARDWARE_VERSION, DeviceInfo_GetHardwareVersion)


#ifdef __cplusplus
}
#endif
#endif /* DEVICEINFO_H_ */

