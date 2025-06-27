/*
 * DeviceIndicatorLED.c
 *
 *  Created on: 2016年7月23日
 *      Author: LIANG
 */

#include "stm32f4xx.h"

#include "System.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Tracer/Trace.h"
#include "Common/Types.h"
#include "SystemConfig.h"
#include "DeviceIndicatorLED.h"

static void LEDTask(void *argument);

static Uint16 s_onTime;
static Uint16 s_offTime;
static Uint32 s_duration;
static Uint32 s_alreadyTime = 0;
static Uint16 s_times = 0;
static Bool s_isTimeOutJudge = FALSE;
static xTaskHandle s_LEDHandle;

#define LED_PORT        GPIOC
#define LED_PIN         GPIO_Pin_13
#define LED_RCC         RCC_AHB1Periph_GPIOC
#define LED_ON          GPIO_ResetBits(LED_PORT, LED_PIN)
#define LED_OFF         GPIO_SetBits(LED_PORT, LED_PIN)

void DeviceIndicatorLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    GPIO_SetBits(LED_PORT,LED_PIN);
    xTaskCreate(LEDTask, "DeviceRunLED", DEVICEINDICATOR_LED_STK_SIZE, NULL,
            DEVICEINDICATOR_LED_TASK_PRIO, &s_LEDHandle);
}

static void LEDTask(void *argument)
{
    (void) argument;
    s_onTime = 50;
    s_offTime = 50;
    while (1)
    {
        vTaskDelay(10 / portTICK_RATE_MS);
        ++s_times;
        if (TRUE == s_isTimeOutJudge && (++s_alreadyTime == s_duration))
        {
            s_alreadyTime = 0;
            s_isTimeOutJudge = FALSE;
            s_onTime = 50;
            s_offTime = 50;
        }
        if (s_times == s_onTime)
        {
            LED_OFF;
        }
        else if (s_times >= s_onTime + s_offTime)
        {
            LED_ON;
            s_times = 0;
        }
    }
}

void DeviceIndicatorLED_SetBlink(Uint32 duration, Uint16 onTime, Uint16 offTime)
{
    if (0 != duration)
    {
        TRACE_INFO("SetBlink duration:%d, onTime:%d, offTime:%d", duration,
                onTime, offTime);
        if ((Uint32) (-1) == duration)
        {
            s_isTimeOutJudge = FALSE;
        }
        else
        {
            s_duration = duration;
            s_isTimeOutJudge = TRUE;
            s_alreadyTime = 0;
        }

        if (0 == onTime)
        {
            LED_OFF;
            vTaskSuspend(s_LEDHandle);
        }
        else if (0 == offTime)
        {
            LED_ON;
            vTaskSuspend(s_LEDHandle);
        }
        else
        {
            s_times = 0;
            LED_ON;
            s_onTime = onTime;
            s_offTime = offTime;
            vTaskResume(s_LEDHandle);
        }
    }
}
