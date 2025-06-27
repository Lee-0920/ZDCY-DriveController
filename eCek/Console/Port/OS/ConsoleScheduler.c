#include <CmdLine.h>
/**
 * @addtogroup module_EmConsole
 * @{
 */

/**
 * @file
 * @brief 控制台调度程序实现。
 * @details 创建一个 RTOS 任务，用于控制台命令的处理。
 * @version 1.1.1
 * @author kim.xiejinqiang
 * @date 2016-4-28
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "console/Console.h"
#include "systemConfig.h"
#include "console/ConsoleScheduler.h"
static xSemaphoreHandle s_semaphore;
static void ConsoleScheduler_TaskHandle(void *pvParameters);

/**
 * @brief 模块初始化。
 * @details 初始化时，将创建一个任务，用于监控用户输入的命令并执行。
 */
void ConsoleScheduler_Init(void)
{
    s_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(ConsoleScheduler_TaskHandle, "Console",
            CONSOLESCHEDULER_STK_SIZE, NULL, CONSOLESCHEDULER_TASK_PRIO, NULL);
}

/**
 * @brief 命令台控制处理任务。
 * @param pvParameters
 */
static void ConsoleScheduler_TaskHandle(void *pvParameters)
{
    while (1)
    {
        if ( xSemaphoreTake(s_semaphore, portMAX_DELAY) == pdTRUE)
        {
            Console_RoutineHandle();
        }
    }
}
/**
 * @brief 激活调度器。
 * @details 在中断程序中调用，用于唤醒任务进行命令的执行。
 */
void ConsoleScheduler_Active(void)
{
    static portBASE_TYPE isHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(s_semaphore, &isHigherPriorityTaskWoken);
    if (isHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR(isHigherPriorityTaskWoken);
    }
}

/** @} */
