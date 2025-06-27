/*
 * DeviceUpdate.h
 *
 *  Created on: 2016年7月19日
 *      Author: LIANG
 */

#ifndef SRC_DNCPSTACK_DEVICEUPDATE_H_
#define SRC_DNCPSTACK_DEVICEUPDATE_H_

#include "DNCP/App/DscpDevice.h"
#include "LuipApi/DeviceUpdateInterface.h"
//声明处理函数
void DeviceUpdate_GetVersion(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceUpdate_GetMaxFragmentSize(DscpDevice* dscp, Byte* data, Uint16 len);
#ifdef _CS_APP
void DeviceUpdate_EnterUpdater(DscpDevice* dscp, Byte* data, Uint16 len);
#else
void DeviceUpdate_EnterApplication(DscpDevice* dscp, Byte* data, Uint16 len);
#endif
void DeviceUpdate_Erase(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceUpdate_WriteProgram(DscpDevice* dscp, Byte* data, Uint16 len);
void DeviceUpdate_CheckIntegrity(DscpDevice* dscp, Byte* data, Uint16 len);

//命令入口，全局宏定义：每一条命令对应一个命令码和处理函数
#ifdef _CS_APP
#define CMD_TABLE_DEVICE_UPDATE \
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_GET_VERSION, DeviceUpdate_GetVersion),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_GET_MAX_FRAGMENT_SIZE, DeviceUpdate_GetMaxFragmentSize),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_ENTER_UPDATER, DeviceUpdate_EnterUpdater), \
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_ERASE, DeviceUpdate_Erase),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_WRITE_PROGRAM, DeviceUpdate_WriteProgram),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_CHECK_INTEGRITY, DeviceUpdate_CheckIntegrity)
#else
#define CMD_TABLE_DEVICE_UPDATE \
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_GET_VERSION, DeviceUpdate_GetVersion),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_GET_MAX_FRAGMENT_SIZE, DeviceUpdate_GetMaxFragmentSize),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_ENTER_APPLICATION, DeviceUpdate_EnterApplication), \
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_ERASE, DeviceUpdate_Erase),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_WRITE_PROGRAM, DeviceUpdate_WriteProgram),\
        DSCP_CMD_ENTRY(DSCP_CMD_DUI_CHECK_INTEGRITY, DeviceUpdate_CheckIntegrity)
#endif

#endif /* SRC_DNCPSTACK_DEVICEUPDATE_H_ */
