/*
 * ThermostatDeviceMap.h
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICEMAP_H_
#define SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICEMAP_H_

#include "ThermostatDevice.h"

#define MEAROOM_COOLER     			0
#define Internal_Fan         		1
#define External_Fan               	2
#define BOX_FAN                		3

#define TOTAL_THERMOSTATDEVICE          4

void ThermostatDeviceMap_Init(ThermostatDevice* device);
char* ThermostatDeviceMap_GetName(Uint8 index);

#endif /* SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICEMAP_H_ */
