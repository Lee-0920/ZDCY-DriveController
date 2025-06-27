/*
 * ThermostatDeviceDriver.h
 *
 *  Created on: 2017年11月16日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICEDRIVER_H_
#define SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICEDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

typedef enum
{
    THERMOSTATDEVICEDRIVER_PWM = 0,
    THERMOSTATDEVICEDRIVER_IO = 1,
    THERMOSTATDEVICEDRIVER_VIRTUAL_PWM = 2,
}ThermostatDeviceDriverMode;

typedef void(*TimerRccInitFunction)(uint32_t RCC_Periph, FunctionalState NewState);

typedef struct
{
    uint8_t pinSource;
    uint8_t goipAF;
    TimerRccInitFunction timerRccInitFunction;
    uint32_t timerRcc;
    uint16_t timerPrescaler;
    uint32_t timerPeriod;
    uint16_t timerChannel;
    TIM_TypeDef* timer;
    uint16_t timerOCPolarity;
    uint16_t timerOCMode;
}PWMModeConfig;

typedef struct
{
    BitAction open;
    BitAction close;
}IOModeConfig;

typedef union
{
    PWMModeConfig PWMConfig;
    IOModeConfig IOConfig;
}ModeConfig;

typedef struct
{
    ThermostatDeviceDriverMode mode;
    GPIO_TypeDef * port;
    uint32_t pin;
    uint32_t gpioRcc;
    ModeConfig modeConfig;
}ThermostatDeviceDriver;

void ThermostatDeviceDriver_Init(ThermostatDeviceDriver *deviceDriver);
Bool ThermostatDeviceDriver_SetOutput(ThermostatDeviceDriver *deviceDriver, float level);
void ThermostatDeviceDriver_PwmFunc(void *device, Bool state);

#endif /* SRC_DRIVER_TEMPDRIVER_THERMOSTATDEVICEDRIVER_H_ */
