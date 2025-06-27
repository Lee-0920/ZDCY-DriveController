/*
 * DoorLock.c
 *
 *  Created on: 2020年5月18日
 *      Author: Administrator
 */
#include "DoorLock.h"
#include "tracer/trace.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"

//冰箱门电子锁控制接口
#define LOCK_OUT_RCC	RCC_AHB1Periph_GPIOD
#define LOCK_OUT_PORT	GPIOD
#define LOCK_OUT_PIN	GPIO_Pin_2

//冰箱门当前开关状态
#define LOCK_IN_RCC		RCC_AHB1Periph_GPIOE
#define LOCK_IN_PORT	GPIOE
#define LOCK_IN_PIN		GPIO_Pin_10

//电子锁打开后经过一段试剂自动关闭，防止上位机忘记关闭导致锁损坏
#define OPEN_WAIT_TIME 	60000

static void ElectronicLockControl_TestTask(void *argument);

void ElectronicLockControl_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(LOCK_OUT_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LOCK_OUT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(LOCK_OUT_PORT, &GPIO_InitStructure);
    xTaskCreate(ElectronicLockControl_TestTask, "ElectronicLockControl_TestTask", 128, NULL,
                4, NULL);
}

void DoorDetection_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(LOCK_IN_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LOCK_IN_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LOCK_IN_PORT, &GPIO_InitStructure);
}

void ElectronicLockControl_TestTask(void *argument)
{
	static Uint32 waitTime = 0;
	static Uint16 interval = 5000;
	while(1)
	{
		vTaskDelay(interval / portTICK_RATE_MS);
		if(ElectronicLock_ReadStatus())
		{
			waitTime = waitTime + interval;
			if(waitTime >= OPEN_WAIT_TIME)
			{
				ElectronicLock_Control(FALSE);
				waitTime = 0;
			}
		}
		else
		{
			waitTime = 0;
		}

	}
}

Bool ElectronicLock_Control(Bool status)
{
    if (TRUE == status)
    {
    	Printf("\n ElectronicLock Open");
        GPIO_SetBits(LOCK_OUT_PORT,LOCK_OUT_PIN);
    }
    else
    {
    	Printf("\n ElectronicLock Close");

        GPIO_ResetBits(LOCK_OUT_PORT,LOCK_OUT_PIN);
    }

    return TRUE;
}

Bool ElectronicLock_ReadStatus(void)
{
	if(GPIO_ReadOutputDataBit(LOCK_OUT_PORT,LOCK_OUT_PIN))
	{
	    return TRUE;
	}
	else
	{
	    return FALSE;
	}
}

DoorStatus DoorDetection_ReadStatus(void)
{
    if(GPIO_ReadInputDataBit(LOCK_IN_PORT,LOCK_IN_PIN))
    {
        return DOOR_CLOSE;
    }
    else
    {
        return DOOR_OPEN;
    }
}


