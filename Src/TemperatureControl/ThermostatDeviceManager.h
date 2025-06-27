/*
 * ThermostatDeviceManager.h
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#ifndef SRC_TEMPERATURECONTROL_THERMOSTATDEVICEMANAGER_H_
#define SRC_TEMPERATURECONTROL_THERMOSTATDEVICEMANAGER_H_

#include "TempDriver/ThermostatDevice.h"
#include "TempDriver/ThermostatDeviceMap.h"
#include "Common/Types.h"

void ThermostatDeviceManager_Init(void);
char* ThermostatDeviceManager_GetName(Uint8 index);
Bool ThermostatDeviceManager_IsOpen(Uint8 index);
Bool ThermostatDeviceManager_SetOutput(Uint8 index, float level);
float ThermostatDeviceManager_GetMaxDutyCycle(Uint8 index);
Bool ThermostatDeviceManager_SetMaxDutyCycle(Uint8 index, float value);
void ThermostatDeviceManager_RestoreInit(void);

Bool ThermostatDeviceManager_GetRefrigeratorStatus(void);
void RefrigeratorRelayDriver_Control(Bool status);

#endif /* SRC_TEMPERATURECONTROL_THERMOSTATDEVICEMANAGER_H_ */
