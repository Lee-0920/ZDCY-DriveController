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
#include "DNCP/App/DscpDevice.h"
#include "systemConfig.h"

#include "LuipApi/DeviceStatusInterface.h"

static xSemaphoreHandle s_semScheduler;
static DscpDevice* s_dscpDevice;

static void DscpScheduler_TaskHandle(void *pvParameters);

/**
 * @brief
 * @details
 */
void DscpScheduler_Init(DscpDevice* dscp)
{
    s_dscpDevice = dscp;
    s_semScheduler = xSemaphoreCreateBinary();

    xTaskCreate(DscpScheduler_TaskHandle, "DscpScheduler",
            DNCPSTACKDSCPCMD_STK_SIZE, NULL, DNCPSTACKDSCPCMD_TASK_PRIO, NULL);
}

static void DscpScheduler_TaskHandle(void *pvParameters)
{
    while (1)
    {
        if ( xSemaphoreTake(s_semScheduler,
                portMAX_DELAY) == pdTRUE)
        {
            DscpDevice_Handle(s_dscpDevice);
        }
    }
}

void DscpScheduler_Active(void)
{
    xSemaphoreGive(s_semScheduler);
}

