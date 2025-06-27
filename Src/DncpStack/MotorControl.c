/*
 * MotorControl.c
 *
 *  Created on: 2018年3月8日
 *      Author: LIANG
 */

#include "Tracer/Trace.h"
#include "Dncp/App/DscpSysDefine.h"
#include <string.h>
#include "PeristalticPump/DisplacementMotorManager.h"
#include "PeristalticPump/DisplacementSteperMotorManager.h"
#include "MotorControl.h"
#include "PeristalticPump/PeristalticPumpManager.h"
#include "PeristalticPump/TMCConfig.h"
#include "PeristalticPump/DCMotorManager.h"
#include "PeristalticPump/PeristalticPumpManager.h"
#include "PeristalticPump/TMC2160.h"
#include "PeristalticPump/StirMotorManager.h"



void MotorControl_GetTotalPumps(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 totalPumps = DISPLACEMENTMOTOR_TOTAL_PUMP;
    TRACE_INFO("\n MotorControl totalPumps: %d", totalPumps);
    DscpDevice_SendResp(dscp, &totalPumps, sizeof(Uint16));
}

void MotorControl_GetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    StepperMotorParam param ;
    memcpy(&index, data, sizeof(Uint8));
    memset(&param, 0, sizeof(StepperMotorParam));

    DscpDevice_SendResp(dscp, &param, sizeof(StepperMotorParam));
}

void MotorControl_SetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    DscpDevice_SendStatus(dscp, DSCP_OK);
}

void MotorControl_GetStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    Uint16 ret = DSCP_IDLE;
    memcpy(&index, data, sizeof(Uint8));

    if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type()) 
    {

        if (MOTOR_IDLE != DisplacementMotor_GetStatus(index))
        {
            ret = DSCP_BUSY;
        }

        if (ret == DSCP_IDLE)
        {
            TRACE_INFO("\n %s idle", DisplacementMotor_GetName(index));
        }
        else
        {
            TRACE_INFO("\n %s busy", DisplacementMotor_GetName(index));
        }
    }
    else
    {
        if (STEPERMOTOR_IDLE != DisplacementSteperMotor_GetStatus(index))
        {
            ret = DSCP_BUSY;
        }

        if (ret == DSCP_IDLE)
        {
            TRACE_INFO("\n %s idle", DisplacementSteperMotor_GetName(index));
        }
        else
        {
            TRACE_INFO("\n %s busy", DisplacementSteperMotor_GetName(index));
        }
    }

    DscpDevice_SendStatus(dscp, ret);
}

void MotorControl_GetMaxSteps(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    float degrees ;
    memcpy(&index, data, sizeof(Uint8));
    degrees = DisplacementMotor_GetMaxDegrees(index);
    
    if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type()) 
    {
        TRACE_INFO("\n %s Degrees: %f", DisplacementMotor_GetName(index), degrees);
    }
    else
    {
        TRACE_INFO("\n %s Degrees: %f", DisplacementSteperMotor_GetName(index), degrees);
    }

    DscpDevice_SendResp(dscp, &degrees, sizeof(degrees));
}

void MotorControl_GetInitSteps(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    Uint16 step ;
    memcpy(&index, data, sizeof(Uint8));
    step = 0;
    
    if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type()) 
    { 
        TRACE_INFO("\n %s step: %d", DisplacementMotor_GetName(index), step);
    }
    else
    {
        TRACE_INFO("\n %s step: %d", DisplacementSteperMotor_GetName(index), step);
    }

    DscpDevice_SendResp(dscp, &step, sizeof(step));
}

void MotorControl_GetCurrentSteps(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    float degrees ;
    memcpy(&index, data, sizeof(Uint8));
    
    if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type()) 
    {
        degrees = DisplacementMotor_GetCurrentDegrees(index);
        TRACE_INFO("\n %s Degrees: %f", DisplacementMotor_GetName(index), degrees);
    }
    else
    {
        degrees = DisplacementMotor_GetCurrentSteps(index);
        degrees = STEP_TO_DEGREES(degrees);
        TRACE_INFO("\n %s Degrees: %f", DisplacementSteperMotor_GetName(index), degrees);
    }

    DscpDevice_SendResp(dscp, &degrees, sizeof(degrees));
}

void MotorControl_Start(DscpDevice* dscp, Byte* data, Uint16 len)//开启水留样定位电机
{
    Uint16 ret = DSCP_OK;
    Uint16 size = 0;
    Uint8 index;
    float degree;

    //设置数据正确性判断
    size = sizeof(Uint8) + sizeof(float);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(&index, data, sizeof(Uint8));//获取步进电机号
        size = sizeof(Uint8);
        memcpy(&degree, data + size, sizeof(float));//获取度数 
        
        if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type()) 
        {
            DisplacementMotor_SendEventOpen(index);
            ret = DisplacementMotor_Start(index, degree);//启动电机
        }
        else
        {
            //角度换算为度数
            DisplacementSteperMotor_SendEventOpen(index);
            ret = DisplacementSteperMotor_Start(index, DEGREES_TO_STEP(degree),MOTOR_MOVE_ABSOLUTE_MODE, TRUE);//启动电机
        }
    }
    DscpDevice_SendStatus(dscp, ret);
}

void MotorControl_Stop(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint8 index;

    memcpy(&index, data, sizeof(Uint8));
    
    if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type()) 
    {
        DisplacementMotor_SendEventOpen(index);
        ret = DisplacementMotor_RequestStop(1);
    }
    else
    {
        DisplacementSteperMotor_SendEventOpen(0);
        ret = DisplacementSteperMotor_RequestStop(0);
    }

    DscpDevice_SendStatus(dscp, ret);
}

void MotorControl_Reset(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint8 index;

    memcpy(&index, data, sizeof(Uint8));
    
    if (DISPLACEMENTMOTOR_DC == GetDisplacementMotor_type())
    {
        DisplacementMotor_SendEventOpen(index);
        ret = DisplacementMotor_Reset(index);//电机复位
    }
    else 
    {
        DisplacementSteperMotor_SendEventOpen(0);
        ret = DisplacementSteperMotor_Reset(index);//电机复位
    }

    DscpDevice_SendStatus(dscp, ret);
}

//定位电机复位传感器和液位检测传感器状态查询
void MotorControl_GetSensorStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    Uint8 status = 0;
    memcpy(&index, data, sizeof(Uint8));

    switch(index)
    {
    	case 0://定位电机复位传感器1
	    	if (TRUE == DisplacementMotor_IsSensorBlocked(0,0))
	    	{
	    		TRACE_INFO("\n cDisplacementMotor Sensor 1 Blocked");
	    		status = Coverd;
	    	}
	    	else
	    	{
	    		status = NotCoverd;
	    	}
			break;
    	case 1://定位电机复位传感器2
    		if (TRUE == DisplacementMotor_IsSensorBlocked(0,1))
    		{
    			TRACE_INFO("\n cDisplacementMotor Sensor 0 Blocked");
    			status = Coverd;
    		}
    		else
    		{
    			status = NotCoverd;
    		}
    		break;
    	case 2://1号液位传感器
    		if (TRUE == WaterCheckSensorBlocked(0))
    		{
    		    TRACE_INFO("\n Water Sensor1 Blocked");
    			status = Coverd;
    		}
    		else
    		{
    			status = NotCoverd;
    		}
    		break;
    	case 3://2号液位传感器
    		if (TRUE == WaterCheckSensorBlocked(1))
    		{
    		    TRACE_INFO("\n Water Sensor2 Blocked");
    			status = Coverd;
    		}
    		else
    		{
    			status = NotCoverd;
    		}
    		break;
    	default:
    	    break;
    }

    DscpDevice_SendResp(dscp, &status, sizeof(status));
}

void MotorControl_DriverReinit(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    DscpDevice_SendStatus(dscp, ret);
}

void MotorControl_DriverCheck(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 ret = DSCP_OK;
    Uint8 index;
    memcpy(&index, data, sizeof(Uint8));
    DscpDevice_SendResp(dscp, &ret, sizeof(Uint8));
}

//开关直流电机
void DC_Motor_Start(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint8 ret = DSCP_OK;
	Uint8 index;
	Uint8 dir;
	float time;
	Uint16 size = 0;

	memcpy(&index, data, sizeof(Uint8));
	size = sizeof(Uint8);
	memcpy(&dir, data + size, sizeof(Uint8));
	size = sizeof(Uint8) + sizeof(Uint8);
	memcpy(&time, data + size, sizeof(float));
	TRACE_INFO("\n********");
	TRACE_INFO("\n DNCP Get Cmd DCMotor Start dir = %d , time = %f s ",dir,time);
	if(DCMotorManager_SendEventOpen(index))
	{
		DCMotorManager_Start(index ,dir ,time);
	}
	else
	{
		Uint8 ret = DSCP_ERROR_PARAM;
	}

	DscpDevice_SendStatus(dscp, ret);
}

//获取直流电机的状态
void DC_Motor_Get_State(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint8 ret = DSCP_IDLE;
	Uint8 index;

	memcpy(&index, data, sizeof(Uint8));

	if (DCMotorManager_GetState(index) == DCMOTORDEVICE_BUSY)
	{
		ret = DSCP_BUSY;
	}

	DscpDevice_SendStatus(dscp, ret);
}

void DC_Motor_Stop(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint8 ret = DSCP_OK;
	Uint8 index;

	memcpy(&index, data, sizeof(Uint8));

	DCMotorManager_Stop(index);

	ret = DSCP_OK;

	DscpDevice_SendStatus(dscp, ret);
}

//设置搅拌电机功率
void DC_StirMotor_SetLevel(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint8 ret = DSCP_OK;
	Uint8 index;
	float level;
	Uint16 size = 0;

	memcpy(&index, data, sizeof(Uint8));
	size = sizeof(Uint8);
	memcpy(&level, data + size, sizeof(float));

	if(StirMotorManager_Setlevel(index,level))
	{
		Uint8 ret = DSCP_ERROR_PARAM;
	}

	DscpDevice_SendStatus(dscp, ret);
}
