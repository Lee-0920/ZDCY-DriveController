/*
 * TempCollecterManager.h
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#ifndef SRC_TEMPERATURECONTROL_TEMPCOLLECTERMANAGER_H_
#define SRC_TEMPERATURECONTROL_TEMPCOLLECTERMANAGER_H_

#include "Driver/TempDriver/TempCollecter.h"
#include "Driver/TempDriver/TempADCollect.h"

void TempCollecterManager_Init();
char* TempCollecterManager_GetName(Uint8 index);
TempCalibrateParam TempCollecterManager_GetCalibrateFactor(Uint8 index);
Bool TempCollecterManager_SetCalibrateFactor(Uint8 index, TempCalibrateParam tempCalibrateParam);
float TempCollecterManager_GetTemp(Uint8 index);
float TempCollecterManager_GetEnvironmentTemp();
Bool TempCollecterManager_AutoTempCalibrate(Uint8 index, float realTemp);

#endif /* SRC_TEMPERATURECONTROL_TEMPCOLLECTERMANAGER_H_ */
