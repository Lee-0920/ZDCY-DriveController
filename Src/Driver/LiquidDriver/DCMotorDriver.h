/*
 * DCMotorDriver.h
 *
 *  Created on: 2018年2月28日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_DCMOTORDRIVER_H_
#define SRC_DRIVER_LIQUIDDRIVER_DCMOTORDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

typedef struct
{
    GPIO_TypeDef *port;
    Uint16 pin;
    uint32_t rcc;
}DCMotorDriver;

typedef enum
{
    DMMOTORDEVICEDRIVER_PWM = 0,
	DMMOTORDEVICEDRIVER_IO = 1,
}DCMotorDeviceDriverMode;

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
}DCPWMModeConfig;

typedef struct
{
    BitAction open;
    BitAction close;
}DCIOModeConfig;

typedef union
{
    DCPWMModeConfig PWMConfig;
    DCIOModeConfig IOConfig;
}DCMotorModeConfig;

typedef struct
{
    DCMotorDeviceDriverMode mode;
    GPIO_TypeDef * port;
    uint32_t pin;
    uint32_t gpioRcc;
    DCMotorModeConfig modeConfig;
}DCMotorDeviceDriver;

void DCMotorDriver_Init(DCMotorDriver *motorDriver);
void DCMotorDriver_Start(DCMotorDriver *motorDriver);
void DCMotorDriver_Stop(DCMotorDriver *motorDriver);
void DCMotorDeviceDriver_Init(DCMotorDeviceDriver *deviceDriver);
Bool DCMotorDeviceDriver_SetOutput(DCMotorDeviceDriver *deviceDriver, float level);

#endif /* SRC_DRIVER_LIQUIDDRIVER_DCMOTORDRIVER_H_ */
