/*
 * ThermostatDeviceMap.c
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#include "ThermostatDeviceMap.h"
#include "Driver/System.h"
#include <string.h>
#include "Tracer/Trace.h"

static Bool ThermostatDeviceMap_SetOutputWay1(ThermostatDeviceDriver *deviceDriver, float level);
static Bool ThermostatDeviceMap_SetOutputWay2(ThermostatDeviceDriver *deviceDriver, float level);

void ThermostatDeviceMap_Init(ThermostatDevice* device)
{
    //冰箱电源继电器
    device[0].maxDutyCycle = 1;
    device[0].setOutputWayFunc = ThermostatDeviceMap_SetOutputWay1;
    device[0].deviceDriver.mode = THERMOSTATDEVICEDRIVER_IO;
    device[0].deviceDriver.port = GPIOC;
    device[0].deviceDriver.pin = GPIO_Pin_11;
    device[0].deviceDriver.gpioRcc = RCC_AHB1Periph_GPIOC;
    device[0].deviceDriver.modeConfig.IOConfig.open = Bit_SET;
    device[0].deviceDriver.modeConfig.IOConfig.close = Bit_RESET;
    ThermostatDevice_Init(&device[0]);

    //冰箱内部制冷风扇1和2
    device[1].maxDutyCycle = 1;
    device[1].setOutputWayFunc = ThermostatDeviceMap_SetOutputWay1;
    device[1].deviceDriver.mode = THERMOSTATDEVICEDRIVER_VIRTUAL_PWM;
    device[1].deviceDriver.port = GPIOD;
    device[1].deviceDriver.pin = GPIO_Pin_0;
    device[1].deviceDriver.gpioRcc = RCC_AHB1Periph_GPIOD;
    device[1].deviceDriver.modeConfig.IOConfig.open = Bit_SET;
    device[1].deviceDriver.modeConfig.IOConfig.close = Bit_RESET;
    ThermostatDevice_Init(&device[1]);

    //冰箱外部制冷风扇1和2
    device[2].maxDutyCycle = 1;
    device[2].setOutputWayFunc = ThermostatDeviceMap_SetOutputWay1;
    device[2].deviceDriver.mode = THERMOSTATDEVICEDRIVER_IO;
    device[2].deviceDriver.port = GPIOC;
    device[2].deviceDriver.pin = GPIO_Pin_12;
    device[2].deviceDriver.gpioRcc = RCC_AHB1Periph_GPIOC;
    device[2].deviceDriver.modeConfig.IOConfig.open = Bit_SET;
    device[2].deviceDriver.modeConfig.IOConfig.close = Bit_RESET;
    ThermostatDevice_Init(&device[2]);

    //机箱风扇
    device[3].maxDutyCycle = 1;
    device[3].setOutputWayFunc = ThermostatDeviceMap_SetOutputWay1;
    device[3].deviceDriver.mode = THERMOSTATDEVICEDRIVER_IO;
    device[3].deviceDriver.port = GPIOA;
    device[3].deviceDriver.pin = GPIO_Pin_12;
    device[3].deviceDriver.gpioRcc = RCC_AHB1Periph_GPIOA;
    device[3].deviceDriver.modeConfig.IOConfig.open = Bit_SET;
    device[3].deviceDriver.modeConfig.IOConfig.close = Bit_RESET;
    ThermostatDevice_Init(&device[3]);
}

static Bool ThermostatDeviceMap_SetOutputWay1(ThermostatDeviceDriver *deviceDriver, float level)
{
    TRACE_CODE("\n Output way 1");
    return ThermostatDeviceDriver_SetOutput(deviceDriver, level);
}

static Bool ThermostatDeviceMap_SetOutputWay2(ThermostatDeviceDriver *deviceDriver, float level)
{
    TRACE_CODE("\n Output way 2");
    if (0 != level)
    {
        level = 0.5 * level + 0.5;
        if (level < 0.75)
        {
            ThermostatDeviceDriver_SetOutput(deviceDriver, 1);
            System_Delay(200);
        }
    }
    return ThermostatDeviceDriver_SetOutput(deviceDriver, level);
}

char* ThermostatDeviceMap_GetName(Uint8 index)
{
    static char name[35] = "";
    memset(name, 0, sizeof(name));
    switch(index)
    {
    case MEAROOM_COOLER:
        strcpy(name, "RefrigeratorRelay");
        break;
    case Internal_Fan:
        strcpy(name, "RefrigeratorInternalFan");
        break;
    case External_Fan:
        strcpy(name, "RefrigeratorExternalFan");
        break;
    case BOX_FAN:
        strcpy(name, "BOX_FAN");
        break;
    default:
        strcpy(name, "NULL");
        break;
    }
    return name;
}
