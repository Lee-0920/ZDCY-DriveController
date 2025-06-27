/*
 * DCMotorDriver.c
 *
 *  Created on: 2018年2月28日
 *      Author: LIANG
 */

#include "DCMotorDriver.h"
#include "PeristalticPump/DCMotorManager.h"
#include "stm32f4xx.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"

TIM_TypeDef* g_alreadyInitTimer[15] = { 0 };
uint8_t g_index = 0;

static void DCMotorDeviceDriver_AddTimer(TIM_TypeDef* timer)
{
    g_alreadyInitTimer[g_index] = timer;
    g_index++;
}

static Bool DCMotorDeviceDriver_IsAlreadyInitTimer(TIM_TypeDef* timer)
{
    Bool ret = FALSE;
    for (int i = 0; i < g_index; i++)
    {
        if (g_alreadyInitTimer[i] == timer)
        {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

void DCMotorDriver_Init(DCMotorDriver *motorDriver)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(motorDriver->rcc, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = motorDriver->pin;
    GPIO_Init(motorDriver->port, &GPIO_InitStructure);
    DCMotorDriver_Stop(motorDriver);
}

void DCMotorDriver_Start(DCMotorDriver *motorDriver)
{
    GPIO_SetBits(motorDriver->port, motorDriver->pin);
}

void DCMotorDriver_Stop(DCMotorDriver *motorDriver)
{
    GPIO_ResetBits(motorDriver->port, motorDriver->pin);
}

void DCMotorDeviceDriver_Init(DCMotorDeviceDriver *deviceDriver)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(deviceDriver->gpioRcc, ENABLE);

    if (DMMOTORDEVICEDRIVER_PWM == deviceDriver->mode)
    {
        GPIO_InitStructure.GPIO_Pin = deviceDriver->pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init(deviceDriver->port, &GPIO_InitStructure);

        GPIO_PinAFConfig(deviceDriver->port, deviceDriver->modeConfig.PWMConfig.pinSource, deviceDriver->modeConfig.PWMConfig.goipAF);

        if (FALSE == DCMotorDeviceDriver_IsAlreadyInitTimer(deviceDriver->modeConfig.PWMConfig.timer))
        {
            TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

            deviceDriver->modeConfig.PWMConfig.timerRccInitFunction(deviceDriver->modeConfig.PWMConfig.timerRcc, ENABLE);

            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
            TIM_TimeBaseStructure.TIM_Period = deviceDriver->modeConfig.PWMConfig.timerPeriod;
            TIM_TimeBaseStructure.TIM_Prescaler = deviceDriver->modeConfig.PWMConfig.timerPrescaler;
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
            TIM_TimeBaseInit(deviceDriver->modeConfig.PWMConfig.timer, &TIM_TimeBaseStructure);
        }
        TIM_OCInitTypeDef TIM_OCInitStructure;
        TIM_OCInitStructure.TIM_OCMode = deviceDriver->modeConfig.PWMConfig.timerOCMode;
        TIM_OCInitStructure.TIM_OCPolarity = deviceDriver->modeConfig.PWMConfig.timerOCPolarity;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        ///TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 0;

        switch(deviceDriver->modeConfig.PWMConfig.timerChannel)
        {
            case 1:
                TIM_OC1Init(deviceDriver->modeConfig.PWMConfig.timer, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(deviceDriver->modeConfig.PWMConfig.timer, TIM_OCPreload_Enable);
                break;
            case 2:
                TIM_OC2Init(deviceDriver->modeConfig.PWMConfig.timer, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(deviceDriver->modeConfig.PWMConfig.timer, TIM_OCPreload_Enable);
                break;
            case 3:
                TIM_OC3Init(deviceDriver->modeConfig.PWMConfig.timer, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(deviceDriver->modeConfig.PWMConfig.timer, TIM_OCPreload_Enable);
                break;
            case 4:
                TIM_OC4Init(deviceDriver->modeConfig.PWMConfig.timer, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(deviceDriver->modeConfig.PWMConfig.timer, TIM_OCPreload_Enable);
                break;
        }
        TIM_CCxCmd(deviceDriver->modeConfig.PWMConfig.timer, deviceDriver->modeConfig.PWMConfig.timerChannel, ENABLE);

        if (FALSE == DCMotorDeviceDriver_IsAlreadyInitTimer(deviceDriver->modeConfig.PWMConfig.timer))
        {
        	DCMotorDeviceDriver_AddTimer(deviceDriver->modeConfig.PWMConfig.timer);
            TIM_ARRPreloadConfig(deviceDriver->modeConfig.PWMConfig.timer, ENABLE);
            TIM_Cmd(deviceDriver->modeConfig.PWMConfig.timer, ENABLE);
            TIM_CtrlPWMOutputs(deviceDriver->modeConfig.PWMConfig.timer, ENABLE);
        }
    }
    else
    {
        GPIO_InitStructure.GPIO_Pin = deviceDriver->pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(deviceDriver->port, &GPIO_InitStructure);
    }
    DCMotorDeviceDriver_SetOutput(deviceDriver, 0);
}

Bool DCMotorDeviceDriver_SetOutput(DCMotorDeviceDriver *deviceDriver, float level)
{
    if (DMMOTORDEVICEDRIVER_PWM == deviceDriver->mode)
    {
        switch(deviceDriver->modeConfig.PWMConfig.timerChannel)
        {
            case 1:
                TIM_SetCompare1(deviceDriver->modeConfig.PWMConfig.timer, (uint16_t) ((deviceDriver->modeConfig.PWMConfig.timerPeriod) * level));
                break;
            case 2:
                TIM_SetCompare2(deviceDriver->modeConfig.PWMConfig.timer, (uint16_t) ((deviceDriver->modeConfig.PWMConfig.timerPeriod) * level));
                break;
            case 3:
                TIM_SetCompare3(deviceDriver->modeConfig.PWMConfig.timer, (uint16_t) ((deviceDriver->modeConfig.PWMConfig.timerPeriod) * level));
                break;
            case 4:
                TIM_SetCompare4(deviceDriver->modeConfig.PWMConfig.timer, (uint16_t) ((deviceDriver->modeConfig.PWMConfig.timerPeriod) * level));
                break;
        }
    }
    else
    {
        if(level != 0)
        {
            TRACE_MARK("\n IO %d", deviceDriver->modeConfig.IOConfig.open);
            GPIO_WriteBit(deviceDriver->port, deviceDriver->pin, deviceDriver->modeConfig.IOConfig.open);
        }
        else
        {
            TRACE_MARK("\n IO %d", deviceDriver->modeConfig.IOConfig.close);
            GPIO_WriteBit(deviceDriver->port, deviceDriver->pin, deviceDriver->modeConfig.IOConfig.close);
        }
    }
    TRACE_DEBUG("\n SetOutput level ");
    System_PrintfFloat(TRACE_LEVEL_DEBUG, level * 100, 3);
    TRACE_DEBUG(" %%");
    return TRUE;
}
