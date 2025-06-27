/*
 * DCMotorManager.c
 *
 *  Created on: 2018年3月9日
 *      Author: LIANG
 */
#include "string.h"
#include "DCMotorManager.h"
#include "Driver/LiquidDriver/DCMotorMap.h"
#include "LiquidDriver/ConstantDCMotorDriver.h"
#include "SystemConfig.h"
#include "DNCP/App/DscpSysDefine.h"
#include "PumpEventScheduler.h"
#include "DncpStack/DncpStack.h"
#include "LuipApi/MotorControlInterface.h"
//试剂泵

Bool s_isSendEvent = FALSE;

static ConstantDCMotor s_DCMotorDevice;
static DCMotorDeviceStatus s_DCMotorStatus = DCMOTORDEVICE_IDLE;
static DCMotorHandleStatus s_DCMotorHandleStatus = DCMOTORHANDLE_IDLE;
static void DCMotor_TaskHandle(void *pvParameters);
static xTaskHandle s_DCMotorHandle;

static Uint8 s_dir = 0;

//运行时间  毫秒
static Uint16 s_turnOnTime = 0;

static MoveResult s_dcMotorResult;

void DCMotorManager_Init()
{
	DCMotorMap_Init(&s_DCMotorDevice);
    xTaskCreate(DCMotor_TaskHandle, "DCMotor", DCMOTOR_STK_SIZE, NULL,
    		DCMOTOR_TASK_PRIO, &s_DCMotorHandle);
}

void DCMotorManager_Restore()
{
	DCMotorManager_Stop(0);
}

Uint16 DCMotorManager_Start(Uint8 index, Uint8 dir, float time)
{
	Uint16 ret = DSCP_OK;
	if (s_DCMotorStatus == DCMOTORDEVICE_IDLE)
	{
		if (time > 0 && dir <= 1)
		{
			s_turnOnTime = (int)time*1000;
			s_dir = dir;
			s_DCMotorStatus = DCMOTORDEVICE_BUSY;
			s_DCMotorHandleStatus = DCMOTORHANDLE_START;
			vTaskResume(s_DCMotorHandle);
			TRACE_INFO("\n DCMotor Start dir = %d , time = %f s ",dir,time);
		}
		else
		{
			ret = DSCP_ERROR_PARAM;
		}
	}
	else
	{
		ret = DSCP_BUSY;
	}
	return ret;
}

static void DCMotorManager_SendEvent()
{
    if(TRUE == s_isSendEvent)
    {
        Uint8 data[5] = {0};
        data[0] =  DCMOTOR_INDEX;
        memcpy(data + sizeof(Uint8), &s_dcMotorResult, sizeof(s_dcMotorResult));
        DncpStack_SendEvent(DSCP_EVENT_MCI_MOTOR_RESULT, data , sizeof(Uint8) + sizeof(s_dcMotorResult));
        DncpStack_BufferEvent(DSCP_EVENT_MCI_MOTOR_RESULT, data , sizeof(Uint8) + sizeof(s_dcMotorResult));
    }
    s_isSendEvent = FALSE;
}

Uint16 DCMotorManager_Stop(Uint8 index)
{
	if (DCMOTORHANDLE_BUSY == s_DCMotorHandleStatus)
	{
		TRACE_INFO("\n DCMotor Request Stop");
		s_dcMotorResult = RESULT_STOPPED;
		s_DCMotorStatus = DCMOTORDEVICE_IDLE;
		s_DCMotorHandleStatus = DCMOTORHANDLE_FINISH;
	}
	return DSCP_OK;
}

Bool DCMotorManager_SendEventOpen(Uint8 index)
{
    if (index == DCMOTOR_INDEX)
    {
    	s_isSendEvent = TRUE;
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n No. %d DCMotor.", index);
        return FALSE;
    }
}

DCMotorDeviceStatus DCMotorManager_GetState(Uint8 index)
{
	return s_DCMotorStatus;
}

static void DCMotor_TaskHandle(void *pvParameters)
{
	static Uint16 timeCount = 0;
    vTaskSuspend(NULL);
	while(1)
	{
		vTaskDelay( 1 /portTICK_RATE_MS);
		timeCount++;
		switch(s_DCMotorHandleStatus)
		{
		case DCMOTORHANDLE_IDLE:
			timeCount = 0;
			s_DCMotorStatus = DCMOTORDEVICE_IDLE;
		    vTaskSuspend(NULL);
			break;
		case DCMOTORHANDLE_START:
			timeCount = 0;
			if (0 == s_dir)
			{
				DisplacementMotorDriver_Right(&s_DCMotorDevice);
			}
			else if(1 == s_dir)
			{
				DisplacementMotorDriver_Lift(&s_DCMotorDevice);
			}
			s_DCMotorHandleStatus = DCMOTORHANDLE_BUSY;
			break;
		case DCMOTORHANDLE_BUSY:
			if (timeCount >= s_turnOnTime)
			{
				s_dcMotorResult = RESULT_FINISHED;
				s_DCMotorHandleStatus = DCMOTORHANDLE_FINISH;
			}
			break;
		case DCMOTORHANDLE_FINISH:
			TRACE_INFO("\n DCMotor Stop timeCount = %d Ms",timeCount);
			DisplacementMotorDriver_Stop(&s_DCMotorDevice);
			DCMotorManager_SendEvent();
			s_DCMotorHandleStatus = DCMOTORHANDLE_IDLE;
			break;
		}
	}
}

