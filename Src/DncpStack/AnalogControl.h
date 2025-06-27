/*
 * AnalogControl.h
 *
 *  Created on: 2020年5月20日
 *      Author: Administrator
 */

#ifndef SRC_DNCPSTACK_ANALOGCONTROL_H_
#define SRC_DNCPSTACK_ANALOGCONTROL_H_

#include "Common/Types.h"
#include "DNCP/App/DscpDevice.h"
#include "LuipApi/AnalogControlInterface.h"

void AnalogControl_GetAINumber(DscpDevice* dscp, Byte* data, Uint16 len);
void AnalogControl_GetAIData(DscpDevice* dscp, Byte* data, Uint16 len);
void AnalogControl_GetAllAIData(DscpDevice* dscp, Byte* data, Uint16 len);
void AnalogControl_SetAIUploadPeriod(DscpDevice* dscp, Byte* data, Uint16 len);

// 命令入口，全局宏定义：每一条命令对应一个命令码和处理函数
#define CMD_TABLE_ANALOGCONTROL  \
    DSCP_CMD_ENTRY(DSCP_CMD_ACI_GET_AI_NUM, AnalogControl_GetAINumber), \
	DSCP_CMD_ENTRY(DSCP_CMD_ACI_GET_AI_DATA, AnalogControl_GetAIData), \
	DSCP_CMD_ENTRY(DSCP_CMD_ACI_ALL_AI_DATA, AnalogControl_GetAllAIData), \
	DSCP_CMD_ENTRY(DSCP_CMD_ACI_SET_AI_UPLOAD_PERIOD, AnalogControl_SetAIUploadPeriod)

#endif /* SRC_DNCPSTACK_ANALOGCONTROL_H_ */
