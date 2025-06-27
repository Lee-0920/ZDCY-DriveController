/*
 * ThermostatManager.h
 *
 *  Created on: 2017年11月17日
 *      Author: LIANG
 */

#ifndef SRC_TEMPERATURECONTROL_THERMOSTATMANAGER_H_
#define SRC_TEMPERATURECONTROL_THERMOSTATMANAGER_H_

#include "Common/Types.h"
#include "Thermostat.h"
#include "TempCollecterManager.h"

#define TOTAL_THERMOSTAT    1

void ThermostatManager_Init();
char* ThermostatManager_GetName(Uint8 index);
void ThermostatManager_RestoreInit(void);
ThermostatParam ThermostatManager_GetPIDParam(Uint8 index);
Bool ThermostatManager_SetPIDParam(Uint8 index, ThermostatParam thermostatParam);
ThermostatStatus ThermostatManager_GetStatus(Uint8 index);
int ThermostatManager_Start(Uint8 index, ThermostatMode mode, float targetTemp,
        float toleranceTemp, float timeout);
void ThermostatManager_SendEventClose(Uint8 index);
void ThermostatManager_SendEventOpen(Uint8 index);
Bool ThermostatManager_RequestStop(Uint8 index);
ThermostatParam ThermostatManager_GetCurrentPIDParam(Uint8 index);
Bool ThermostatManager_SetCurrentPIDParam(Uint8 index, ThermostatParam thermostatParam);
void ThermostatManager_SetTempReportPeriod(float reportparam);
Bool ThermostatManager_SetSingleRefrigeratorOutput(Uint8 thermostatIndex, Uint8 fanIndex, float level);
float ThermostatManager_GetHeaterMaxDutyCycle(Uint8 index);
Bool ThermostatManager_SetMaxDutyCycle(Uint8 index, float value);
TempCalibrateParam ThermostatManager_GetCalibrateFactor(Uint8 index);
Bool ThermostatManager_SetCalibrateFactor(Uint8 index, TempCalibrateParam tempCalibrateParam);
float ThermostatManager_GetCurrentTemp(Uint8 index);

#endif /* SRC_TEMPERATURECONTROL_THERMOSTATMANAGER_H_ */
