/*
 * PumpEventScheduler.c
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "FreeRTOS.h"
#include "task.h"
#include "DncpStack/DncpStack.h"
#include "systemConfig.h"
#include "PumpEventScheduler.h"

/**
 * @brief 泵实体
 */
typedef struct
{
    //基本参数
    Uint8 number;
    Bool isSendEvent;
    enum PumpResult result;
}PumpEvent;

static PumpEvent s_pumpEvents[DNCP_PUMP_TOTAL];
static void PumpEventScheduler_TaskHandle(void *pvParameters);

/**
 * @brief
 * @details
 */
void PumpEventScheduler_Init(void)
{
    memset(s_pumpEvents, 0, sizeof(s_pumpEvents));

    xTaskCreate(PumpEventScheduler_TaskHandle, "PumpEvent",
                PUMP_EVENT_STK_SIZE, NULL, PUMP_EVENT_TASK_PRIO, NULL);
}

static void PumpEventScheduler_TaskHandle(void *pvParameters)
{
    Uint8 data[2] = {0};

    while (1)
    {
        System_Delay(10);

        for(int i = 0; i < DNCP_PUMP_TOTAL; i++)
        {
            if (TRUE == s_pumpEvents[i].isSendEvent)
            {
                memset(data, 0, sizeof(data));

                data[0] = s_pumpEvents[i].number;
                data[1] = s_pumpEvents[i].result;

                if (1 == data[0])
                {
                	DncpStack_SendEvent(DSCP_EVENT_PPI_PUMP_1_RESULT, data , sizeof(data));
                }
                else
                {
                	DncpStack_SendEvent(DSCP_EVENT_PPI_PUMP_0_RESULT, data , sizeof(data));
                }

                s_pumpEvents[i].isSendEvent = FALSE;
            }
        }
    }
}

void PumpEventScheduler_SendEvent(Uint8 pumpNumber, enum PumpResult pumpResult, Bool isSendEvent)
{
    s_pumpEvents[pumpNumber].number = pumpNumber;
    s_pumpEvents[pumpNumber].result = pumpResult;
    s_pumpEvents[pumpNumber].isSendEvent = isSendEvent;

    TRACE_INFO("\n Pump event Scheduler send Event: number  = ");
    System_PrintfFloat(TRACE_LEVEL_INFO, pumpNumber, 4);
    TRACE_INFO(" result  = ");
    System_PrintfFloat(TRACE_LEVEL_INFO, pumpResult, 4);
}
