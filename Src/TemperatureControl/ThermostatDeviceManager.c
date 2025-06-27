/*
 * ThermostatDeviceManager.c
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#include "ThermostatDeviceManager.h"
#include "TempDriver/PwmDriver.h"
#include <string.h>
#include "Tracer/Trace.h"

ThermostatDevice s_thermostatDevice[TOTAL_THERMOSTATDEVICE];

void ThermostatDeviceManager_Init(void)
{
    PwmDriver_Init();
    ThermostatDeviceMap_Init(s_thermostatDevice);
}

void RefrigeratorRelayDriver_Control(Bool status)
{
    if (TRUE == status)
    {
    	ThermostatDeviceManager_SetOutput(3, 1);
    }
    else
    {
    	ThermostatDeviceManager_SetOutput(3, 0);
    }
}

Bool ThermostatDeviceManager_GetRefrigeratorStatus(void)
{
	 Bool ret = FALSE;

	 if(ThermostatDeviceManager_IsOpen(3))
	 {
	     ret = TRUE;
	 }
	 else
	 {
	     ret = FALSE;
	 }

	 return ret;
}

void ThermostatDeviceManager_RestoreInit(void)
{
    if (TRUE == ThermostatDeviceManager_IsOpen(0))
    {
        ThermostatDeviceManager_SetOutput(0, 0);
    }

    if (TRUE == ThermostatDeviceManager_IsOpen(1))
    {
        ThermostatDeviceManager_SetOutput(1, 0);
    }

    if (FALSE == ThermostatDeviceManager_IsOpen(2))
    {
        ThermostatDeviceManager_SetOutput(2, 1);
    }

    if (FALSE == ThermostatDeviceManager_IsOpen(3))
    {
        ThermostatDeviceManager_SetOutput(3, 1);
    }

}

char* ThermostatDeviceManager_GetName(Uint8 index)
{
    return ThermostatDeviceMap_GetName(index);
}

Bool ThermostatDeviceManager_IsOpen(Uint8 index)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTATDEVICE)
    {
        if (THERMOSTATDEVICE_IDLE == ThermostatDevice_GetStatus(&s_thermostatDevice[index]))
        {
            TRACE_INFO("\n No. %d thermostatDevice idle", index);
        }
        else
        {
            ret = TRUE;
            TRACE_INFO("\n No. %d thermostatDevice busy", index);
        }
    }
    else
    {
        TRACE_ERROR("\n IsOpen  No. %d thermostatDevice.", index);
    }
    return ret;
}

Bool ThermostatDeviceManager_SetOutput(Uint8 index, float level)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTATDEVICE)
    {
        TRACE_MARK("\n No. %d thermostatDevice", index);
        ret = ThermostatDevice_SetOutput(&s_thermostatDevice[index], level);
    }
    else
    {
        TRACE_ERROR("\n SetOutput No. %d thermostatDevice.", index);
    }
    return ret;
}

float ThermostatDeviceManager_GetMaxDutyCycle(Uint8 index)
{
    float value = 0;
    if (index < TOTAL_THERMOSTATDEVICE)
    {
        value = ThermostatDevice_GetMaxDutyCycle(&s_thermostatDevice[index]);
    }
    else
    {
        TRACE_ERROR("\n GetMaxDutyCycle No. %d thermostatDevice.", index);
    }
    return value;
}

Bool ThermostatDeviceManager_SetMaxDutyCycle(Uint8 index, float value)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTATDEVICE)
    {
        ret = ThermostatDevice_SetMaxDutyCycle(&s_thermostatDevice[index], value);
    }
    else
    {
        TRACE_ERROR("\n SetMaxDutyCycle No. %d thermostatDevice.", index);
    }
    return ret;
}

