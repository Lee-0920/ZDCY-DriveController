/*
 * ValveManager.c
 *
 *  Created on: 2016年6月7日
 *      Author: Administrator
 */
#include "Tracer/Trace.h"
#include "LiquidDriver/ValveDriver.h"
#include "LiquidDriver/ValveMap.h"
#include "string.h"
#include "ValveManager.h"
//系统中使用的继电器总数目

static Valve s_valves[SOLENOIDVALVECONF_TOTALVAlVES];

void ValveManager_Init(void)
{
    memset(s_valves, 0, sizeof(Valve) * SOLENOIDVALVECONF_TOTALVAlVES);

    ValveMap_Init(s_valves);
}

Uint16 ValveManager_GetTotalValves(void)
{
    return (SOLENOIDVALVECONF_TOTALVAlVES);
}

Bool ValveManager_SetValvesMap(Uint32 map)
{
    Uint8 i;
    Uint32 mask = 1;
    if (map <= SOLENOIDVALVE_MAX_MAP)
    {
        TRACE_INFO("\nSetMap: 0x%x", map);
        for (i = 0; i < SOLENOIDVALVECONF_TOTALVAlVES; i++)
        {
            if (map & mask)
            {
                ValveDriver_Control(&s_valves[i], VAlVE_OPEN);
            }
            else
            {
                ValveDriver_Control(&s_valves[i], VAlVE_CLOSE);
            }
            mask = mask << 1;
        }
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n The map must be 0 to 0x%x.", SOLENOIDVALVE_MAX_MAP);
        return FALSE;
    }
}

Uint32 ValveManager_GetValvesMap(void)
{
    Uint8 i;
    Uint32 mask = 0;
    Uint32 Temp = 1;

    for (i = 0; i < SOLENOIDVALVECONF_TOTALVAlVES; i++)
    {
        if (VAlVE_OPEN == ValveDriver_ReadStatus(&s_valves[i]))
        {
            mask |= Temp;
        }
        Temp = Temp << 1;
    }
    return mask;
}

