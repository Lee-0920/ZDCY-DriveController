/********** Copyright (c) 2022-2022 SZLABSUN.Co.Ltd. All rights reserved.**********
* File Name          : StirMotorDriver.c
* Author             : hyz
* Date               : 06/28/2022
* Description        : This file provides all the StirMotorDriver functions.
*******************************************************************************/
#include "StirMotorDriver.h"

StirMotorDriver s_StirMotor[2];
static void StirMotorControl1_TaskHandle(void *pvParameters);
static void StirMotorControl2_TaskHandle(void *pvParameters);


void StirMotorDriver_InitMap()
{
	s_StirMotor[0].rcc = RCC_AHB1Periph_GPIOD;
	s_StirMotor[0].port = GPIOD;
	s_StirMotor[0].pin = GPIO_Pin_4;
	s_StirMotor[0].workLevel = 0;

	s_StirMotor[1].rcc = RCC_AHB1Periph_GPIOD;
	s_StirMotor[1].port = GPIOD;
	s_StirMotor[1].pin = GPIO_Pin_3;
	s_StirMotor[1].workLevel = 0;
}

void StirMotorDriver_InitDriver()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(
    		s_StirMotor[0].rcc | s_StirMotor[1].rcc,
            ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

    GPIO_InitStructure.GPIO_Pin = s_StirMotor[0].pin;
    GPIO_Init(s_StirMotor[0].port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = s_StirMotor[1].pin;
    GPIO_Init(s_StirMotor[1].port, &GPIO_InitStructure);
}

void StirMotorDriver_StatusSet(Uint8 index, Bool Status)
{
	if (TRUE == Status)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_13);
		GPIO_SetBits(s_StirMotor[index].port,s_StirMotor[index].pin);
	}
	else
	{
		GPIO_ResetBits(GPIOE,GPIO_Pin_13);
		GPIO_ResetBits(s_StirMotor[index].port,s_StirMotor[index].pin);
	}
}

void StirMotorDriver_Init()
{
	StirMotorDriver_InitMap();
	StirMotorDriver_InitDriver();

    xTaskCreate(StirMotorControl1_TaskHandle, "Stir1", 64, NULL,
    		5, NULL);

    xTaskCreate(StirMotorControl2_TaskHandle, "Stir2", 64, NULL,
    		5, NULL);
}

Bool StirMotorDriver_SetLevel(Uint8 index ,float level)
{
	Bool ret = TRUE;
	if(index<STIRMOTOR_MAX_NUM && level>=0 && level<=1)
	{
		TRACE_INFO("Stir No.%d Set level %f",index,level);
		if (s_StirMotor[index].workLevel > 0)
		{
			s_StirMotor[index].workLevel = 0;
			vTaskDelay( 10 /portTICK_RATE_MS);
		}

		if(level == 0)
		{
			StirMotorDriver_StatusSet(index,FALSE);
		}
		else
		{
			s_StirMotor[index].workLevel = level;
		}
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

static void StirMotorControl1_TaskHandle(void *pvParameters)
{
	static int subdivide = 10;
	static int currentCount = 0;
	static Bool MotorStatus = FALSE;
	while(1)
	{
		vTaskDelay( 1 /portTICK_RATE_MS);
		if (0 < s_StirMotor[0].workLevel)
		{
			if(FALSE == MotorStatus && 0 == currentCount)
			{
				StirMotorDriver_StatusSet(0,TRUE);
				MotorStatus = TRUE;
				currentCount++;
			}
			else if (MotorStatus == TRUE && s_StirMotor[0].workLevel*subdivide <= currentCount*1.0)
			{
				MotorStatus = FALSE;
				if (1.0 != s_StirMotor[0].workLevel)
				{
					StirMotorDriver_StatusSet(0,FALSE);
				}
				currentCount++;
			}
			else if (MotorStatus == FALSE  && 0 < currentCount && subdivide <= currentCount)
			{
				currentCount = 0;
			}
			else
			{
				currentCount++;
			}

		}
		else
		{
			MotorStatus = FALSE;
			currentCount = 0;
		}
	}
}

static void StirMotorControl2_TaskHandle(void *pvParameters)
{
	static int subdivide = 10;
	static int currentCount = 0;
	static Bool MotorStatus = FALSE;
	while(1)
	{
		vTaskDelay( 1 /portTICK_RATE_MS);
		if (0 < s_StirMotor[1].workLevel)
		{
			if(FALSE == MotorStatus && 0 == currentCount)
			{
				StirMotorDriver_StatusSet(1,TRUE);
				MotorStatus = TRUE;
				currentCount++;
			}
			else if (MotorStatus == TRUE && s_StirMotor[1].workLevel*subdivide <= currentCount*1.0)
			{
				MotorStatus = FALSE;
				if (1.0 != s_StirMotor[1].workLevel)
				{
					StirMotorDriver_StatusSet(1,FALSE);
				}
				currentCount++;
			}
			else if (MotorStatus == FALSE  && 0 < currentCount && subdivide <= currentCount)
			{
				currentCount = 0;
			}
			else
			{
				currentCount++;
			}

		}
		else
		{
			MotorStatus = FALSE;
			currentCount = 0;
		}
	}
}
