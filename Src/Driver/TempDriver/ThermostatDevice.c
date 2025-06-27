/*
 * ThermostatDevice.c
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#include "ThermostatDevice.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"

void ThermostatDevice_Init(ThermostatDevice *device)
{
    device->status = THERMOSTATDEVICE_IDLE;
    ThermostatDeviceDriver_Init(&device->deviceDriver);
}

ThermostatDeviceStatus ThermostatDevice_GetStatus(ThermostatDevice *device)
{
    return device->status;
}

Bool ThermostatDevice_SetOutput(ThermostatDevice *device, float level)
{
    Bool ret = FALSE;
    if (level <= 1 && level >= 0)
    {
        if (0 == level)
        {
            device->status = THERMOSTATDEVICE_IDLE;
        }
        else
        {
            device->status = THERMOSTATDEVICE_BUSY;
            level = device->maxDutyCycle * level;
        }
        ret = device->setOutputWayFunc(&device->deviceDriver, level);
    }
    else
    {
        TRACE_ERROR("\n SetOutput out of range, level: ");
        System_PrintfFloat(TRACE_LEVEL_ERROR, level, 3);
    }
    return ret;
}

float ThermostatDevice_GetMaxDutyCycle(ThermostatDevice *device)
{
    TRACE_INFO("\n GetMaxDutyCycle maxDutyCycle: ");
    System_PrintfFloat(TRACE_LEVEL_INFO, device->maxDutyCycle, 3);
    return device->maxDutyCycle;
}

Bool ThermostatDevice_SetMaxDutyCycle(ThermostatDevice *device, float value)
{
    if (value <= 1 && value >= 0)
    {
        TRACE_INFO("\n SetMaxDutyCycle maxDutyCycle: ");
        System_PrintfFloat(TRACE_LEVEL_INFO, value, 3);
        device->maxDutyCycle = value;
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n SetMaxDutyCycle out of range, value: ");
        System_PrintfFloat(TRACE_LEVEL_ERROR, value, 3);
        return FALSE;
    }
}
