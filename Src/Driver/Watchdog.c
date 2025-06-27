/*
 * Watchdog.c
 *
 *  Created on: 2016年10月14日
 *      Author: LIANG
 */

#include "stm32F4xx.h"
#include "Common/Types.h"
#include "SystemConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "System.h"
#include "Tracer/Trace.h"

#define WATCHDOG_PRER    4//分频数:低3位有效
//分频因子 = 4 * 2 ^ PRER 最大值为256
#define WATCHDOG_RLR     1250//重装载值:低11位有效
//时间计算:tout = ((4 * 2 ^ PRER) * RLR) / 40 (ms)
static void Watchdog_FeedHandle(void *argument);

static void Watchdog_Config(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//使能对寄存器 I 写操作
    IWDG_SetPrescaler(WATCHDOG_PRER);//设置 IWDG 预分频值
    IWDG_SetReload(WATCHDOG_RLR);//设置 IWDG 重装载值
    IWDG_ReloadCounter();//按照 IWDG 重装载寄存器的值重装载 IWDG 计数器
    IWDG_Enable();

    xTaskCreate(Watchdog_FeedHandle, "Feed Watchdog", FEED_WATCHDOG_STK_SIZE, NULL,
            FEED_WATCHDOG_TASK_PRIO, 0);
}

void Watchdog_Init(void)
{
    Watchdog_Config();
}

void Watchdog_FeedHandle(void *argument)
{
    while(1)
    {
        System_Delay(250);
        IWDG_ReloadCounter();
    }
}



