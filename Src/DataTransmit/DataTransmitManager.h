/*
 * DataTransmitManager.h
 *
 *  Created on: 2020年3月17日
 *      Author: Administrator
 */

#ifndef SRC_DATATRANSMIT_DATATRANSMITMANAGER_H_
#define SRC_DATATRANSMIT_DATATRANSMITMANAGER_H_

#include "Tracer/Trace.h"
#include "Driver/System.h"
#include "LuipApi/DataTransmitInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

void DataTransmitManager_Init(void);
void DataTransmitManager_PortReset(Uint8 index);
Bool DataTransmitManager_GetPortParam(Uint8 index, SerialPortParam* param);
Bool DataTransmitManager_CheckIndex(Uint8 index);
Bool DataTransmitManager_SetPortParam(Uint8 index, SerialPortParam param);
Bool DataTransmitManager_SetAllPortParam(Uint16 size,SerialPortParam* param);
void DataTransmitManager_Config(void);
Bool DataTransmitManager_SendData(Uint8 index, Uint8* data, Uint16 len);
Uint8 DataTransmitManager_TotalPortNumber(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DATATRANSMIT_DATATRANSMITMANAGER_H_ */
