/*
 * ThermostatDevice.h
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICE_H_
#define SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICE_H_

#include "ThermostatDeviceDriver.h"

typedef enum
{
    THERMOSTATDEVICE_IDLE = 0,
    THERMOSTATDEVICE_BUSY = 1,
}ThermostatDeviceStatus;

typedef Bool (*SetOutputWayFunc)(ThermostatDeviceDriver *deviceDriver, float level);

typedef struct
{
    ThermostatDeviceStatus status;
    float maxDutyCycle;
    ThermostatDeviceDriver deviceDriver;
    SetOutputWayFunc setOutputWayFunc;
}ThermostatDevice;

void ThermostatDevice_Init(ThermostatDevice *device);
ThermostatDeviceStatus ThermostatDevice_GetStatus(ThermostatDevice *device);
Bool ThermostatDevice_SetOutput(ThermostatDevice *device, float level);
float ThermostatDevice_GetMaxDutyCycle(ThermostatDevice *device);
Bool ThermostatDevice_SetMaxDutyCycle(ThermostatDevice *device, float value);
#endif /* SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICE_H_ */
