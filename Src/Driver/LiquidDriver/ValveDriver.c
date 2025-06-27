/*
 * ValveDriver.c
 *
 *  Created on: 2016年6月7日
 *      Author: Administrator
 */

#include "ValveDriver.h"

void ValveDriver_Init(Valve *valve)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(valve->rcc, ENABLE);

    GPIO_InitStructure.GPIO_Pin = valve->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(valve->port, &GPIO_InitStructure);
    GPIO_ResetBits(valve->port, valve->pin);
}

void ValveDriver_Control(Valve *valve, SolenoidValueStatus status)
{
    if (VAlVE_OPEN == status)
    {
        GPIO_SetBits(valve->port,valve->pin);
    }
    else
    {
        GPIO_ResetBits(valve->port,valve->pin);
    }
}

SolenoidValueStatus ValveDriver_ReadStatus(Valve *valve)
{
    if(GPIO_ReadOutputDataBit(valve->port,valve->pin))
    {
        return VAlVE_OPEN;
    }
    else
    {
        return VAlVE_CLOSE;
    }
}
