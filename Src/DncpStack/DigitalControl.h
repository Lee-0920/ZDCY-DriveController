/*
 * DigitalControl.h
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */

#ifndef SRC_DNCPSTACK_DIGITALCONTROL_H_
#define SRC_DNCPSTACK_DIGITALCONTROL_H_

#include "LuipApi/DigitalControlInterface.h"
#include "DNCP/App/DscpDevice.h"

void DigitalControl_GetDINumber(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_GetDIStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_GetDIStatusMap(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_GetDONumber(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_GetDOStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_SetDOStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_GetDOStatusMap(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_SetDOStatusMap(DscpDevice* dscp, Byte* data, Uint16 len);
void DigitalControl_GetGateStatus(DscpDevice* dscp, Byte* data, Uint16 len);//查询门禁状态
void DigitalControl_SetLockState(DscpDevice* dscp, Byte* data, Uint16 len);//开关电子锁
void DigitalControl_GetLockStatus(DscpDevice* dscp, Byte* data, Uint16 len);//查询电子锁状态
void DigitalControl_SetFlowMeter(DscpDevice* dscp, Byte* data, Uint16 len);//查询开始流量计时

#define CMD_TABLE_DIGITAL_CONTROL \
	DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_DI_NUM, DigitalControl_GetDINumber), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_DI_STATUS, DigitalControl_GetDIStatus), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_DI_MAP, DigitalControl_GetDIStatusMap), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_DO_NUM, DigitalControl_GetDONumber), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_DO_STATUS, DigitalControl_GetDOStatus), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_SET_DO_STATUS, DigitalControl_SetDOStatus), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_DO_MAP, DigitalControl_GetDOStatusMap), \
    DSCP_CMD_ENTRY(DSCP_CMD_DCI_SET_DO_MAP, DigitalControl_SetDOStatusMap),\
	DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_GATE_STATUS, DigitalControl_GetGateStatus),\
	DSCP_CMD_ENTRY(DSCP_CMD_DCI_SET_LOCK_STATE, DigitalControl_SetLockState),\
	DSCP_CMD_ENTRY(DSCP_CMD_DCI_GET_LOCK_STATUS, DigitalControl_GetLockStatus),\
	DSCP_CMD_ENTRY(DSCP_CMD_DCI_SET_FLOW_METER, DigitalControl_SetFlowMeter)

#endif /* SRC_DNCPSTACK_DIGITALCONTROL_H_ */
