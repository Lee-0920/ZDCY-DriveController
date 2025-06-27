/*
 * DNCPScheduler.c
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "DNCP/Lai/LaiRS485Handler.h"
#include "Tracer/Trace.h"
#include "systemConfig.h"

static xSemaphoreHandle s_SemapLaiRS485SendRequest,
        s_SemaphLaiRS485CommitToUpper;

static void LaiRS485CommitToUpper_Task(void *pvParameters);
static void LaiRS485SendRequest_Task(void *pvParameters);
static void LaiRS485Scheduler_MonitorHostTask(void *pvParameters);
/**
 * @brief
 * @details
 */
void LaiRS485Scheduler_Init(void)
{
    s_SemaphLaiRS485CommitToUpper = xSemaphoreCreateBinary();
    s_SemapLaiRS485SendRequest = xSemaphoreCreateBinary();

    xTaskCreate(LaiRS485CommitToUpper_Task, "CommitToUpper",
            LAIRS485COMMITTOUPPER_STK_SIZE, NULL,
            LAIRS485COMMITTOUPPER_TASK_PRIO, NULL);

    xTaskCreate(LaiRS485SendRequest_Task, "SendRequest",
            LAIRS485SENDREQUEST_STK_SIZE, NULL, LAIRS485SENDREQUEST_TASK_PRIO,
            NULL);

    xTaskCreate(LaiRS485Scheduler_MonitorHostTask, "MonitorHost",
            LAIRS485MONITORHOST_STK_SIZE, NULL, LAIRS485MONITORHOST_TASK_PRIO,
            NULL);
}

static void LaiRS485CommitToUpper_Task(void *pvParameters)
{
//    unsigned long int RxCnt = 0;
    while (1)
    {
        if ( xSemaphoreTake(s_SemaphLaiRS485CommitToUpper,
                portMAX_DELAY) == pdTRUE)
        {
//            RxCnt++;
//            TRACE_INFO("\r\n<<<<<<<<<<<<<<<<<<<<ReceCount%d\r\n", RxCnt);
            LaiRS485_CommitToUpper();
        }
    }
}

static void LaiRS485SendRequest_Task(void *pvParameters)
{
    while (1)
    {
        if ( xSemaphoreTake(s_SemapLaiRS485SendRequest,
                portMAX_DELAY) == pdTRUE)
        {
            LaiRS485_SendRequest();
        }
    }
}

static void LaiRS485Scheduler_MonitorHostTask(void *pvParameters)
{
    while (1)
    {
        LaiRS485_PollingTimeOut();
    }
}
void LaiRS485CommitToUpperTask_Active(void)
{
    static portBASE_TYPE isHigherPriorityTaskWoken_Rec = pdFALSE;

    xSemaphoreGiveFromISR(s_SemaphLaiRS485CommitToUpper,
            &isHigherPriorityTaskWoken_Rec);
    if (isHigherPriorityTaskWoken_Rec == pdTRUE)
    {
        portYIELD_FROM_ISR(isHigherPriorityTaskWoken_Rec);
    }
}

void LaiRS485SendRequestTask_Active(void)
{
    static portBASE_TYPE isHigherPriorityTaskWoken_Send = pdFALSE;

    xSemaphoreGiveFromISR(s_SemapLaiRS485SendRequest,
            &isHigherPriorityTaskWoken_Send);
    if (isHigherPriorityTaskWoken_Send == pdTRUE)
    {
        portYIELD_FROM_ISR(isHigherPriorityTaskWoken_Send);
    }
}

