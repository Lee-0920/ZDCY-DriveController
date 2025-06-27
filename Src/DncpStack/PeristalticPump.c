/*
 * PeristalticPump.c
 *
 *  Created on: 2016年6月4日
 *      Author: Administrator
 */
#include "Tracer/Trace.h"
#include "Dncp/App/DscpSysDefine.h"
#include <string.h>
#include "PeristalticPump/PeristalticPumpManager.h"
#include "System.h"
#include "PeristalticPump.h"
#include "SystemConfig.h"
#include <stdio.h>
#include "PeristalticPump/DCMotorManager.h"
#include "LiquidDriver/WaterPump.h"
#include "LiquidDriver/DrainWaterPump.h"
#include "LuipApi/PeristalticPumpInterface.h"
#include "AnalogControl/AnalogControl.h"

typedef enum
{
    SYRINGE,
    PERISTALTICPUMP1,
	PERISTALTICPUMP2,
    MAX_PERISTALTICPUMPTYPE,
}PeristalticPumpType;

static PeristalticPumpType PeristalticPump_GetType(Uint8 index)
{
    PeristalticPumpType peristalticPumpType;
    if (index < DCMOTOR_OFFSET)//蠕动泵1
    {
        peristalticPumpType = PERISTALTICPUMP1;
    }
    else if (index < DNCP_PUMP_TOTAL)//直流电机2
    {
        peristalticPumpType = PERISTALTICPUMP2;
    }
    else//不存在
    {
        peristalticPumpType = MAX_PERISTALTICPUMPTYPE;
    }
    return peristalticPumpType;
}

static char* PeristalticPump_GetInfo(Uint8 index)
{
    static char name[25] = "";
    memset(name, 0, sizeof(name));

    switch(PeristalticPump_GetType(index))
    {
    case PERISTALTICPUMP1:
        {
            Uint8 number = index - PERISTALTICPUMP_OFFSET;
            sprintf(name, "PeristalticPump %d", number);
        }
        break;
    case PERISTALTICPUMP2:
    	{
            Uint8 number = index - PERISTALTICPUMP_OFFSET;
            sprintf(name, "PeristalticPump %d", number);
        }
        break;
    default:
        strcpy(name, "NULL");
        break;
    }
    return name;
}

/**
 * @brief 查询系统支持的总泵数目。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_GetTotalPumps(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 totalPumps = DNCP_PUMP_TOTAL;
    TRACE_INFO("\n totalPumps: %d", totalPumps);
    DscpDevice_SendResp(dscp, &totalPumps, sizeof(Uint16));
}

/**
 * @brief 查询指定泵的运动参数。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_GetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    StepperMotorParam param;
    memcpy(&index, data, sizeof(Uint8));

    param = PeristalticPumpManager_GetDefaultMotionParam(index - PERISTALTICPUMP_OFFSET);

    TRACE_INFO("\n index:%d, %s acc:", index, PeristalticPump_GetInfo(index));
    System_PrintfFloat(TRACE_LEVEL_INFO, param.acceleration, 4);
    TRACE_INFO(" ml/(s^2), maxSpeed:");
    System_PrintfFloat(TRACE_LEVEL_INFO, param.maxSpeed, 4);
    TRACE_INFO(" ml/s");

    DscpDevice_SendResp(dscp, &param, sizeof(StepperMotorParam));
}

/**
 * @brief 设置指定泵的运动参数。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_SetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    StepperMotorParam param;
    Uint16 size = 0;
    Uint8 index;

    //设置数据正确性判断
    size = sizeof(StepperMotorParam) + sizeof(Uint8);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(&index, data, sizeof(Uint8));
        memcpy(&param, data + sizeof(Uint8), sizeof(StepperMotorParam));

        ret = PeristalticPumpManager_SetDefaultMotionParam(index - PERISTALTICPUMP_OFFSET, param);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询指定泵的校准系数。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_GetFactor(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    float factor = 0;
    memcpy(&index, data, sizeof(Uint8));

    factor = PeristalticPumpManager_GetFactor(index - PERISTALTICPUMP_OFFSET);

    TRACE_INFO("\n index:%d, %s get factor:", index, PeristalticPump_GetInfo(index));
    System_PrintfFloat(TRACE_LEVEL_INFO, factor, 8);
    TRACE_INFO(" ml/step");

    DscpDevice_SendResp(dscp, &factor, sizeof(float));
}

/**
 * @brief 设置指定泵的校准系数。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_SetFactor(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    float factor;
    Uint16 size = 0;
    Uint8 index;

    //设置数据正确性判断
    size = sizeof(float) + sizeof(Uint8);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(&index, data, sizeof(Uint8));
        memcpy(&factor, data + sizeof(Uint8), sizeof(float));

        ret = PeristalticPumpManager_SetFactor(index - PERISTALTICPUMP_OFFSET, factor);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询指定泵的工作状态。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_GetStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    Uint16 ret = DSCP_IDLE;
    memcpy(&index, data, sizeof(Uint8));


    if (StepperMotor_IDLE != PeristalticPumpManager_GetStatus(index - PERISTALTICPUMP_OFFSET))
    {
        ret = DSCP_BUSY;
    }

    if (ret == DSCP_IDLE)
    {
        TRACE_INFO("\n index:%d, %s idle", index, PeristalticPump_GetInfo(index));
    }
    else
    {
        TRACE_INFO("\n index:%d, %s busy", index, PeristalticPump_GetInfo(index));
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询泵出的体积。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_GetVolume(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index;
    float volume = 0;
    memcpy(&index, data, sizeof(Uint8));

    volume = PeristalticPumpManager_GetVolume(index - PERISTALTICPUMP_OFFSET);

    TRACE_INFO("\n index:%d, %s Volume", index, PeristalticPump_GetInfo(index));
    System_PrintfFloat(TRACE_LEVEL_INFO, volume, 4);
    TRACE_INFO(" ml");
    DscpDevice_SendResp(dscp, &volume, sizeof(float));
}

/**
 * @brief 启动泵。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_Start(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Direction dir;
    float volume;
    float speed;
    Uint16 size = 0;
    Uint8 index;
    Uint8 sendEvent = TRUE;

    //设置数据正确性判断
    size = sizeof(Uint8) + sizeof(Direction) + sizeof(float) + sizeof(float);
    if ((len < size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else if(len >= size)
    {
        memcpy(&index, data, sizeof(Uint8));
        size = sizeof(Uint8);
        memcpy(&dir, data + size, sizeof(Direction));
        size += sizeof(Direction);
        memcpy(&volume, data + size, sizeof(float));
        size += sizeof(float);
        memcpy(&speed, data + size, sizeof(float));
        size += sizeof(float);

		if(len > size)
		{
			memcpy(&sendEvent, data + size, sizeof(Uint8));
		}

		Uint8 number = index;//把电机编号转换为数组下标
		if(TRUE == sendEvent || len == size)
		{
			PeristalticPumpManager_SendEventOpen(number);
		}
		if (speed > 0)
		{

			StepperMotorParam param;
			param = PeristalticPumpManager_GetDefaultMotionParam(number);
			param.maxSpeed = speed;
			ret = PeristalticPumpManager_SetCurrentMotionParam(number, param);
			if (DSCP_OK == ret)
			{
				ret = PeristalticPumpManager_Start(number, dir, volume, FALSE);
			}
		}
		else
		{
			ret = PeristalticPumpManager_Start(number, dir, volume, TRUE);
		}
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 停止泵。
 * @param dscp
 * @param data
 * @param len
 */
void PeristalticPump_Stop(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint8 index;
    Uint8 sendEvent = TRUE;
    Uint16 size = 0;

    size = sizeof(Uint8);
    if(len < size)
    {
    	ret = DSCP_ERROR;
    	TRACE_ERROR("Parame Len Error\n");
    	TRACE_ERROR("%d \n", size);
    }
    else if(len >= size)
    {
        memcpy(&index, data, sizeof(Uint8));
        size = sizeof(Uint8);
        if(len > size)
        {
        	memcpy(&sendEvent, data + size, sizeof(Uint8));
        	size += sizeof(Uint8);
        }

    	if(TRUE == sendEvent || len == size)
    	{
    		PeristalticPumpManager_SendEventOpen(index - PERISTALTICPUMP_OFFSET);
    	}
    	TRACE_ERROR("\n ********************** Dncp PeristalticPumpManager_Stop.");
    	ret = PeristalticPumpManager_Stop(index - PERISTALTICPUMP_OFFSET);
    }

    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 开启关闭采水泵
 * @param dscp
 * @param data
 * @param len
 */
void WaterPump_SetStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 ret = DSCP_OK;
	Uint8 index = 0;
	Uint8 Data = 0;

	memcpy(&index, data, sizeof(index));
	memcpy(&Data, data+sizeof(index), sizeof(Data));

	if(index == 0)
	{
		if(TRUE == Data)
		{
			WaterPumpDriver_Control(PUMP_OPEN);
		}
		else
		{
			WaterPumpDriver_Control(PUMP_CLOSE);
		}
	}
	else if(index == 1)
	{
		if(TRUE == Data)
		{
			DrainWaterPumpDriver_Control(PUMP_OPEN);
		}
		else
		{
			DrainWaterPumpDriver_Control(PUMP_CLOSE);
		}
	}
	else
	{
		ret = DSCP_ERROR_PARAM;
		Printf("\n DSCP_ERROR_PARAM");
		DscpDevice_SendStatus(dscp, DSCP_ERROR_PARAM);
		return;
	}


	DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 获取采水泵的状态
 * @param dscp
 * @param data
 * @param len
 */
void WaterPump_GetStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 ret = DSCP_IDLE;
	Uint8 index;
	memcpy(&index, data, sizeof(index));
	if(index == 0)
	{
		ret = WaterPumpDriver_ReadStatus();
	}
	else if(index == 1)
	{
		ret = DrainWaterPumpDriver_ReadStatus();
	}
	else
	{
		ret = DSCP_ERROR_PARAM;
		Printf("\n DSCP_ERROR_PARAM");
		DscpDevice_SendResp(dscp, &ret, sizeof(ret));
		return;
	}


	if(PUMP_OPEN == ret)
	{
		ret = DSCP_BUSY;
	}
	else
	{
		ret = DSCP_IDLE;
	}
	DscpDevice_SendResp(dscp, &ret, sizeof(ret));
}

/**
 * @brief 泵至检测点。
 * @param[in] index Uint8，要操作的泵索引，0号泵为光学定量泵。
 * @param[in] dir Uint8，泵转动方向，0为正向转动（抽取），1为反向转动（排空）。
 * @param[in] destStatus CheckPointStatus, 目标检测点需要达到的状态。
 * @param[in] speed float，速度，单位为 ml/s。
 * @param[in] maxVolume float，最大限制体积，单位为 ml。
 * @return 返回状态，操作是否成功。
 * @note 该命令将立即返回，泵转动完成将以事件的形式上报。
 */
void PeristalticPump_StartToPoint(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Direction dir;
    CheckPointStatus targetStatus;
    float maxVolume;
    float speed;
    Uint16 size = 0;
    Uint8 index;

    //设置数据正确性判断
    size = sizeof(Uint8) + sizeof(Direction) + sizeof(float) + sizeof(float) + sizeof(CheckPointStatus);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(&index, data, sizeof(Uint8));
        size = sizeof(Uint8);
        memcpy(&dir, data + size, sizeof(Direction));
        size += sizeof(Direction);
        memcpy(&targetStatus, data + size, sizeof(CheckPointStatus));
        size += sizeof(CheckPointStatus);
        memcpy(&speed, data + size, sizeof(float));
        size += sizeof(float);
        memcpy(&maxVolume, data + size, sizeof(float));
        size += sizeof(float);

        Uint8 number = index - PERISTALTICPUMP_OFFSET;//把电机编号转换为数组下标
        //PeristalticPumpManager_SendEventOpen(number);
        if (speed > 0)
        {
            StepperMotorParam param;
            param = PeristalticPumpManager_GetDefaultMotionParam(number);
            param.maxSpeed = speed;
            ret = PeristalticPumpManager_SetCurrentMotionParam(number, param);
            if (DSCP_OK == ret)
            {
            	ret = PeristalticPumpManager_StartToPoint(number, dir, targetStatus, maxVolume, FALSE);
            }
        }
        else
        {
        	ret = PeristalticPumpManager_StartToPoint(number, dir, targetStatus, maxVolume, TRUE);
        }
    }
    DscpDevice_SendStatus(dscp, ret);
}
