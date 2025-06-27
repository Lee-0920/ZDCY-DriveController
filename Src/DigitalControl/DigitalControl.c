/*
 * DigitalControl.c
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */
#include "FreeRTOS.h"
#include "task.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"
#include "SystemConfig.h"
#include "DigitalControl.h"
#include "Driver/DigitalControlDriver/DigitalOutputDriver.h"
#include "Driver/DigitalControlDriver/DigitalInputDriver.h"
#include "Driver/LiquidDriver/DoorLock.h"
#include "DncpStack/DncpStack.h"
#include "LuipApi/DigitalControlInterface.h"

typedef struct
{
    Uint32 lastDIMap;
    Uint32 curDIMap;
    Bool lastDoorStatus;
    Bool curDoorStatus;
    Bool taskInit;
}DigitalController;

TaskHandle_t s_diCheckTask;

static DigitalController s_digitalController;
void DigitalController_DICheckTask(void* argument);

void DigitalController_Init(void)
{
	DigitalOutput_Init();
	DigitalInput_Init();

	s_digitalController.lastDIMap = DigitalInput_GetMap();
	s_digitalController.curDIMap = DigitalInput_GetMap();
	s_digitalController.lastDoorStatus = DoorDetection_ReadStatus();
	s_digitalController.curDoorStatus = DoorDetection_ReadStatus();
	s_digitalController.taskInit = FALSE;
				//数字输入信号检测任务
	xTaskCreate(DigitalController_DICheckTask, "DICheck",
	            DIGITALCONTROLLER_DI_CHECK_STK_SIZE, (void *)&s_digitalController,
	            DIGITALCONTROLLER_DI_CHECK_TASK_PRIO, &s_diCheckTask);
}

Uint16 DigitalController_GetDINumber(void)
{
    Uint16 num = DigitalInput_TotalNumber();

    return num;
}

Bool DigitalController_GetDIStatus(Uint16 index)
{
    Bool status = DigitalInput_Read(index);

    return status;
}

Uint32 DigitalController_GetDIStatusMap(void)
{
    Uint32 map = DigitalInput_GetMap();

    return map;
}

Uint16 DigitalController_GetDONumber(void)
{
    Uint16 num = DigitalOutput_TotalNumber();

    return num;
}

Bool DigitalController_GetDOStatus(Uint16 index)
{
    Bool status = DigitalOutput_IsOpen(index);

    return status;
}

void DigitalController_SetDOStatus(Uint16 index, Bool status)
{
    if(status != FALSE)
    {
        DigitalOutput_TurnOn(index);
    }
    else
    {
        DigitalOutput_TurnOff(index);
    }
}

Uint32 DigitalController_GetDOStatusMap(void)
{
    Uint32 map = DigitalOutput_GetMap();

    return map;
}

void DigitalController_SetDOStatusMap(Uint32 map)
{
    DigitalOutput_SetMap(map);
}

//数字输入信号检测任务
void DigitalController_DICheckTask(void* argument)
{
    DigitalController *digitalController = (DigitalController *)argument;

    while(1)
    {
        digitalController->curDIMap = DigitalInput_GetMap();
        if(digitalController->curDIMap != digitalController->lastDIMap || digitalController->taskInit == FALSE)
        {
            TRACE_INFO("\nDI map changed from 0x%08x to 0x%08x", digitalController->lastDIMap, digitalController->curDIMap);
            DncpStack_SendEvent(DSCP_EVENT_DCI_DI_UPLOAD, (void*)&digitalController->curDIMap, sizeof(digitalController->curDIMap));
            digitalController->lastDIMap = digitalController->curDIMap;
        }
        digitalController->curDoorStatus = DoorDetection_ReadStatus();
        if(digitalController->curDoorStatus != digitalController->lastDoorStatus || digitalController->taskInit == FALSE)
        {
            TRACE_INFO("\nDoor status changed from 0x%08x to 0x%08x", digitalController->lastDoorStatus, digitalController->curDoorStatus);
            DncpStack_SendEvent(DSCP_EVENT_DCI_GATE_STATUS_UPLOAD, (void*)&digitalController->curDoorStatus, sizeof(digitalController->curDoorStatus));
            digitalController->lastDoorStatus = digitalController->curDoorStatus;

            digitalController->taskInit = TRUE;
        }
        System_Delay(100);
    }
}
