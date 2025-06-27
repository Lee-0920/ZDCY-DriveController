/*
 * DigitalOutputDriver.c
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */
#include "DigitalOutputDriver.h"
#include "Tracer/Trace.h"

static DigitalIO s_digitalOutput[DIGITAL_OUT_NUM];

static void DigitalOutput_DriverInit(DigitalIO* io);

void DigitalOutput_Init(void)
{
	s_digitalOutput[0].pin = GPIO_Pin_1;
	s_digitalOutput[0].port = GPIOD;
	s_digitalOutput[0].rcc = RCC_AHB1Periph_GPIOD;
	DigitalOutput_DriverInit(&s_digitalOutput[0]);
}

void DigitalOutput_DriverInit(DigitalIO* io)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(io->rcc, ENABLE);

    GPIO_InitStructure.GPIO_Pin = io->pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(io->port, &GPIO_InitStructure);

    GPIO_ResetBits(io->port, io->pin);  //默认关闭
}

void DigitalOutput_TurnOnAll(void)
{
    TRACE_INFO("\n DO Turn On All");
    for(int index = 0; index < DIGITAL_OUT_NUM; index++)
    {
        GPIO_SetBits(s_digitalOutput[index].port, s_digitalOutput[index].pin);
    }
}

void DigitalOutput_TurnOffAll(void)
{
    TRACE_INFO("\n DO Turn Off All");
    for(int index = 0; index < DIGITAL_OUT_NUM; index++)
    {
        GPIO_ResetBits(s_digitalOutput[index].port, s_digitalOutput[index].pin);
    }
}

void DigitalOutput_TurnOn(int index)
{
    if(index >= 0 && index < DIGITAL_OUT_NUM)
    {
        GPIO_SetBits(s_digitalOutput[index].port, s_digitalOutput[index].pin);
        TRACE_INFO("\n DO %d on", index);
    }
    else
    {
        TRACE_WARN("\n DO %d invalid", index);
    }
}

void DigitalOutput_TurnOff(int index)
{
    if(index >= 0 && index < DIGITAL_OUT_NUM)
    {
        GPIO_ResetBits(s_digitalOutput[index].port, s_digitalOutput[index].pin);
        TRACE_INFO("\n DO %d off", index);
    }
    else
    {
        TRACE_WARN("\n DO %d invalid", index);
    }
}

Bool DigitalOutput_IsOpen(int index)
{
    Bool ret = FALSE;
    if(index >= 0 && index < DIGITAL_OUT_NUM)
    {
        if(GPIO_ReadOutputDataBit(s_digitalOutput[index].port, s_digitalOutput[index].pin))
        {
            ret = TRUE;
            TRACE_INFO("\n DO %d is on", index);
        }
        else
        {
            ret = FALSE;
            TRACE_INFO("\n DO %d is off", index);
        }
    }
    else
    {
        TRACE_WARN("\n DO %d invalid", index);
    }
    return ret;
}

Uint8 DigitalOutput_TotalNumber(void)
{
    return DIGITAL_OUT_NUM;
}

Uint32 DigitalOutput_GetMap(void)
{
    Uint32 map = 0;
    for(int i = 0; i < DIGITAL_OUT_NUM; i++)
    {
        map |= (DigitalOutput_IsOpen(i) << i);
    }

    return map;
}

void DigitalOutput_SetMap(Uint32 map)
{
	Uint8 i = 0;
	Uint32 mask = 1;
	if (map <= 0x1F)
	{
	    TRACE_INFO("\nSetMap: 0x%x", map);
	    for(i = 0; i < DIGITAL_OUT_NUM; i++)
	    {
	    	if(map & mask)
	        {
	        	DigitalOutput_TurnOn(i);
	        }
	        else
	        {
	        	DigitalOutput_TurnOff(i);
	        }
	        mask = mask << 1;
	    }
	}
	else
	{
		TRACE_ERROR("\n The map must be 0 to 0x%x.", 0x1F);
	}
}
