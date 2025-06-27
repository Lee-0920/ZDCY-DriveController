/**
#include <System.h>
 * @addtogroup module_Driver
 * @{
 */

/**
 * @file
 * @brief MCU 核心系统驱动。
 * @details 配置 MCU 的时钟、功耗模式、中断等系统核心参数，提供复位、延时、电源管理等功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2012-10-30
 */

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Tracer/Trace.h"
#include "SystemConfig.h"
#include <stdio.h>
/**
 * @brief 初始化优先级分组和节拍定时器
 */
void System_Init(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    //设置向量表的位置和偏移量
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
    //设置为优先级组4，4位都用于保存抢占式优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    //配置节拍频率
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / configTICK_RATE_HZ);
    //配置节拍定时器的时钟为最低优先级
    NVIC_SetPriority(SysTick_IRQn, 0xf);

}


/**
 * @brief 打开系统的总中断。
 */
void System_EnableMasterInterrupt(void)
{
//    __ASM volatile("cpsie i");
}

/**
 * @brief 关闭系统的总中断。
 */
void System_DisableMasterInterrupt(void)
{
//    __ASM volatile("cpsid i");
}

/**
 * @brief 软件延时（毫秒）。
 * @param mSec 要延时的毫秒数。
 */
void System_Delay(unsigned int mSec)
{
    vTaskDelay(mSec/portTICK_RATE_MS);
}

/**
 * @brief 软件延时（毫秒）。
 * @param mSec 要延时的毫秒数。
 */
void System_NonOSDelay(unsigned int mSec)
{
    for (Uint32 j = 0; j < mSec; j++)
    {
        for (Uint16 i = 0; i < 30000; i++);
    }
}


/**
 * @brief 软件延时（微秒）。(粗略)
 * @param uSec 要延时的微秒数。
 */
void System_DelayUs(unsigned int uSec)
{
    unsigned int i = 0;
    while(uSec--)
    {
        i = 32;
        while(i--);
    }
}

/**
 * @brief 软件延时（秒）。
 * @param sec 要延时的秒数。
 */
void System_DelaySec(unsigned int sec)
{

}


/**
 * @brief 打开电源开关。
 */
void System_PowerOn(void)
{
}

/**
 * @brief 关机。
 */
void System_PowerOff(void)
{
}

/**
 * @brief 系统复位。
 * @details 复位后，系统重新启动。
 */
void System_Reset(void)
{
    NVIC_SystemReset();
}

void System_PrintfFloat(unsigned char level, float value, unsigned char len)
{
    if(level <= g_traceLevel)
    {
        unsigned int IntegerPart;
        float DecimalPart;
        if(value < 0)
        {
            Printf("-");
            value *= -1;
        }

        IntegerPart = (unsigned int)value;
        Printf("%d.", IntegerPart);

        DecimalPart = value - IntegerPart;
        for(int i = 0; i < len; i++)
        {
            DecimalPart *= 10;
            Printf("%d", (unsigned int)DecimalPart % 10);
        }
    }
}

#define TIM_PERIOD         49999
#define TIM_PRESCALER      44999
#define TIMx               TIM7
#define TIM_RCC            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE)
#define TIM_IRQN           TIM7_IRQn
#define TIM_HANDLER        TIM7_IRQHandler

#define MAX_TASK_NUM  20
TaskStatus_t pxTaskStatusArray[MAX_TASK_NUM];

void System_StateTimerInit(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    memset(&TIM_TimeBaseStructure, 0, sizeof(TIM_TimeBaseStructure));
    TIM_RCC;

    TIM_TimeBaseStructure.TIM_Period = TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_Prescaler = TIM_PRESCALER;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIMx, TIM_IT_Update); //避免定时器启动时进入中断服务程序
    TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM_IRQN;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SYSTEM_STATE_TIMER_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIMx,ENABLE);
}

static Uint16 s_statetimer = 0;

Uint32 System_StateTimerValue(void)
{
    Uint32 time = s_statetimer * 50000 + TIM_GetCounter(TIMx);
//    System_PrintfFloat(1, time * 0.0005, 3);
//    Printf(" \n");
    return time;
}

void TIM_HANDLER(void)
{
    if(TIM_GetITStatus(TIMx, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        s_statetimer++;
    }
}

void System_TaskStatePrintf(void)
{
    const char task_state[]={'r','R','B','S','D'};
    volatile UBaseType_t uxArraySize, x;
    uint32_t ulTotalRunTime,ulStatsAsPercentage;

    /* 获取任务总数目 */
    uxArraySize = uxTaskGetNumberOfTasks();
   if(uxArraySize>MAX_TASK_NUM)
    {
       Printf("uxArraySize > MAX_TASK_NUM! %d \n", uxArraySize);
       return;
    }
    /*获取每个任务的状态信息 */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

#if (configGENERATE_RUN_TIME_STATS == 1)

    Printf("name               state  ID    PRIO  STK_SIZE    CPUUSE\n");

    /* 避免除零错误 */
    if( ulTotalRunTime > 0 )
    {
        /* 将获得的每一个任务状态信息部分的转化为程序员容易识别的字符串格式 */
        for( x = 0; x < uxArraySize; x++ )
        {
            char tmp[128];

            /* 计算任务运行时间与总运行时间的百分比。*/
            ulStatsAsPercentage =(uint64_t)(pxTaskStatusArray[ x ].ulRunTimeCounter)*100 / ulTotalRunTime;

            if( ulStatsAsPercentage > 0)
            {

                sprintf(tmp,"%-20s%-6c%-6d%-8d%-8d%d%%",pxTaskStatusArray[ x].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                                       pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                                       pxTaskStatusArray[ x ].usStackHighWaterMark,ulStatsAsPercentage);
            }
            else
            {
                /* 任务运行时间不足总运行时间的1%*/
                sprintf(tmp,"%-20s%-6c%-6d%-8d%-8dt<1%%",pxTaskStatusArray[x ].pcTaskName,task_state[pxTaskStatusArray[ x ].eCurrentState],
                                                                       pxTaskStatusArray[ x ].xTaskNumber,pxTaskStatusArray[ x].uxCurrentPriority,
                                                                       pxTaskStatusArray[ x ].usStackHighWaterMark);
            }
            Printf("%s\n",tmp);
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }
    Printf("task state:   r-run  R-ready B-block S-Suspend  D-del \n \n");
#endif //#if (configGENERATE_RUN_TIME_STATS==1)
}

/** @} */
