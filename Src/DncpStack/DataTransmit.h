/*
 * DataTransmit.h
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */

#ifndef SRC_DNCPSTACK_DATATRANSMIT_H_
#define SRC_DNCPSTACK_DATATRANSMIT_H_

#include "LuipApi/DataTransmitInterface.h"
#include "DNCP/App/DscpDevice.h"

void DataTransmit_GetPortNumber(DscpDevice* dscp, Byte* data, Uint16 len);
void DataTransmit_SendData(DscpDevice* dscp, Byte* data, Uint16 len);
void DataTransmit_GetPortParam(DscpDevice* dscp, Byte* data, Uint16 len);
void DataTransmit_SetPortParam(DscpDevice* dscp, Byte* data, Uint16 len);

#define CMD_TABLE_DATA_TRANSMIT \
    DSCP_CMD_ENTRY(DSCP_CMD_DTI_GET_PORT_NUM, DataTransmit_GetPortNumber), \
    DSCP_CMD_ENTRY(DSCP_CMD_DTI_SEND_DATA, DataTransmit_SendData), \
    DSCP_CMD_ENTRY(DSCP_CMD_DTI_GET_PORT_PARAM, DataTransmit_GetPortParam), \
    DSCP_CMD_ENTRY(DSCP_CMD_DTI_SET_PORT_PARAM, DataTransmit_SetPortParam)

#endif /* SRC_DNCPSTACK_DATATRANSMIT_H_ */
