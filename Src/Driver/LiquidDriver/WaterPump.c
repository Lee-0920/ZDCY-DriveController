/*
 * WaterPump.c
 *
 *  Created on: 2020年5月18日
 *      Author: Administrator
 */

#include "WaterPump.h"

#define WATER_PUMP_RCC		RCC_AHB1Periph_GPIOC
#define WATER_PUMP_PORT		GPIOC
#define WATER_PUMP_PIN		GPIO_Pin_10


void WaterPump_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(WATER_PUMP_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = WATER_PUMP_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(WATER_PUMP_PORT, &GPIO_InitStructure);
}

void WaterPumpDriver_Control(WaterPumpValueStatus status)
{
    if (PUMP_OPEN == status)
    {
        GPIO_SetBits(WATER_PUMP_PORT,WATER_PUMP_PIN);
    }
    else
    {
        GPIO_ResetBits(WATER_PUMP_PORT,WATER_PUMP_PIN);
    }
}

WaterPumpValueStatus WaterPumpDriver_ReadStatus(void)
{
    if(GPIO_ReadOutputDataBit(WATER_PUMP_PORT,WATER_PUMP_PIN))
    {
        return PUMP_OPEN;
    }
    else
    {
        return PUMP_CLOSE;
    }
}
