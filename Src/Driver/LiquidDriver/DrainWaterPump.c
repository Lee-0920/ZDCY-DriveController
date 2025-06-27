/*
 * DrainWaterPump.c
 *
 *  Created on: 2022年6月10日
 *      Author: hyz
 */

#include "DrainWaterPump.h"

#define DRAIN_WATER_PUMP_RCC		RCC_AHB1Periph_GPIOE
#define DRAIN_WATER_PUMP_PORT		GPIOE
#define DRAIN_WATER_PUMP_PIN		GPIO_Pin_9


void DrainWaterPump_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(DRAIN_WATER_PUMP_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = DRAIN_WATER_PUMP_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(DRAIN_WATER_PUMP_PORT, &GPIO_InitStructure);
}

void DrainWaterPumpDriver_Control(WaterPumpValueStatus status)
{
    if (PUMP_OPEN == status)
    {
        GPIO_SetBits(DRAIN_WATER_PUMP_PORT,DRAIN_WATER_PUMP_PIN);
    }
    else
    {
        GPIO_ResetBits(DRAIN_WATER_PUMP_PORT,DRAIN_WATER_PUMP_PIN);
    }
}

WaterPumpValueStatus DrainWaterPumpDriver_ReadStatus(void)
{
    if(GPIO_ReadOutputDataBit(DRAIN_WATER_PUMP_PORT,DRAIN_WATER_PUMP_PIN))
    {
        return PUMP_OPEN;
    }
    else
    {
        return PUMP_CLOSE;
    }
}
