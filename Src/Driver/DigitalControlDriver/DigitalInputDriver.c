/*
 * DigitalInputDriver.c
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */
#include "DigitalInputDriver.h"
#include "DigitalOutputDriver.h"
#include "Tracer/Trace.h"

static DigitalIO s_digitalInput[DIGITAL_IN_NUM];

void DigitalInput_DriverInit(DigitalIO* io);

void DigitalInput_Init(void)
{
    s_digitalInput[0].port = GPIOE;
    s_digitalInput[0].pin =  GPIO_Pin_6;
    s_digitalInput[0].rcc = RCC_AHB1Periph_GPIOE;
    DigitalInput_DriverInit(&s_digitalInput[0]);

    s_digitalInput[1].port = GPIOE;
    s_digitalInput[1].pin =  GPIO_Pin_5;
    s_digitalInput[1].rcc = RCC_AHB1Periph_GPIOE;
    DigitalInput_DriverInit(&s_digitalInput[1]);

    s_digitalInput[2].port = GPIOE;
    s_digitalInput[2].pin =  GPIO_Pin_4;
    s_digitalInput[2].rcc = RCC_AHB1Periph_GPIOE;
    DigitalInput_DriverInit(&s_digitalInput[2]);

    s_digitalInput[3].port = GPIOE;
    s_digitalInput[3].pin =  GPIO_Pin_3;
    s_digitalInput[3].rcc = RCC_AHB1Periph_GPIOE;
    DigitalInput_DriverInit(&s_digitalInput[3]);
}

void DigitalInput_DriverInit(DigitalIO* io)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(io->rcc, ENABLE);

    GPIO_InitStructure.GPIO_Pin = io->pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(io->port, &GPIO_InitStructure);
}

Bool DigitalInput_Read(Uint8 index)
{
    Bool ret = TRUE;
    if(index < DIGITAL_IN_NUM)
    {
        if(!GPIO_ReadInputDataBit(s_digitalInput[index].port, s_digitalInput[index].pin))
        {
        	ret = TRUE;
            //TRACE_INFO("\n DI %d is off", index);
        }
        else
        {
        	ret = FALSE;
            //TRACE_INFO("\n DI %d is on", index);
        }
    }
    else
    {
        TRACE_WARN("\n DI %d invalid", index);
    }
    return ret;
}

Uint8 DigitalInput_TotalNumber(void)
{
    return DIGITAL_IN_NUM;
}

Uint32 DigitalInput_GetMap(void)
{
    Uint32 map = 0;
    for(int i = 0; i < DIGITAL_IN_NUM; i++)
    {
        map |= (DigitalInput_Read(i) << i);
    }

    return map;
}
