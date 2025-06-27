/*
 * ValveMap.c
 *
 *  Created on: 2016年6月7日
 *      Author: Administrator
 */

#include "stm32f4xx.h"
#include "ValveMap.h"
#include "SolenoidValve/ValveManager.h"

void ValveMap_Init(Valve *valve)
{
    Uint8 i;

    valve[0].pin = GPIO_Pin_8;
    valve[0].port = GPIOD;
    valve[0].rcc = RCC_AHB1Periph_GPIOD;
    ValveDriver_Init(&valve[0]);

    valve[1].pin = GPIO_Pin_15;
    valve[1].port = GPIOB;
    valve[1].rcc = RCC_AHB1Periph_GPIOB;
    ValveDriver_Init(&valve[1]);

    valve[2].pin = GPIO_Pin_14;
    valve[2].port = GPIOB;
    valve[2].rcc = RCC_AHB1Periph_GPIOB;
    ValveDriver_Init(&valve[2]);

    valve[3].pin = GPIO_Pin_13;
    valve[3].port= GPIOB;
    valve[3].rcc =  RCC_AHB1Periph_GPIOB;
    ValveDriver_Init(&valve[3]);

    valve[4].pin = GPIO_Pin_12;
    valve[4].port = GPIOB;
    valve[4].rcc = RCC_AHB1Periph_GPIOB;
    ValveDriver_Init(&valve[4]);

    valve[5].pin = GPIO_Pin_14;
    valve[5].port = GPIOE;
    valve[5].rcc = RCC_AHB1Periph_GPIOE;
    ValveDriver_Init(&valve[5]);

    valve[6].pin = GPIO_Pin_13;
    valve[6].port = GPIOE;
    valve[6].rcc = RCC_AHB1Periph_GPIOE;
    ValveDriver_Init(&valve[6]);

    for(i = 0; i < SOLENOIDVALVECONF_TOTALVAlVES; i++)
    {
        ValveDriver_Control(&valve[i], VAlVE_CLOSE);
    }
}


