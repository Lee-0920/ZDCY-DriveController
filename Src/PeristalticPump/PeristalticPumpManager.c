/*
 * PeristalticPumpManager.c
 *
 *  Created on: 2016年5月31日
 *      Author: Administrator
 */
#include "DNCP/App/DscpSysDefine.h"
#include "System.h"
#include "PumpEventScheduler.h"
#include "LiquidDriver/StepperMotorMap.h"
#include "StepperMotor.h"
#include "SystemConfig.h"
#include "McuFlash.h"
#include "DncpStack/DncpStack.h"
#include "PeristalticPumpManager.h"
#include "Driver/LiquidDriver/PositionSensorMap.h"

#define WATERSENSORNUM 2

PeristalticPump g_peristalticPumps[PERISTALTICPUMPMANAGER_TOTAL_PUMP];
PositionSensor g_waterCheckSensor[WATERSENSORNUM];

static void PeristalticPumpManager_FinishHandler(void *obj);
static void Pump1ToCheckPoint_TaskHandle(void *pvParameters);
static void PumpStartToCheckPoint_SendEvent(PeristalticPump *motor, enum PumpResult pumpResult);
static void Pump2ToCheckPoint_TaskHandle(void *pvParameters);

void PeristalticPumpManager_Init(void)
{
    Uint8 i;

    memset(g_peristalticPumps, 0, sizeof(PeristalticPump) * PERISTALTICPUMPMANAGER_TOTAL_PUMP);
    memset(g_waterCheckSensor, 0, sizeof(g_waterCheckSensor));

    PositionSensorMap_WaterCheckSensorInit(g_waterCheckSensor);//液位检测传感器初始化
    StepperMotorMap_PeristalticPumpInit(g_peristalticPumps); //配置泵用到的引脚

    //初始化各个泵的实体参数
    for (i = 0; i < PERISTALTICPUMPMANAGER_TOTAL_PUMP; i++)
    {
        StepperMotor_SetSubdivision(&g_peristalticPumps[i].stepperMotor, 8);//设置运动参数时需要提前设置细分数

        Uint8 buffer[PUMP_SIGN_FLASH_LEN] = { 0 };
        Uint32 flashFactorySign = 0;
        StepperMotorParam param;

        McuFlash_Read(PUMP_SIGN_FLASH_BASE_ADDR + PUMP_SIGN_FLASH_LEN * i,
                PUMP_SIGN_FLASH_LEN, buffer); //读取出厂标志位
        memcpy(&flashFactorySign, buffer, PUMP_SIGN_FLASH_LEN);
        if (FLASH_FACTORY_SIGN == flashFactorySign) //表示已经过出厂设置
        {
            g_peristalticPumps[i].factor = PeristalticPumpManager_GetFactor(i);
            param = PeristalticPumpManager_GetDefaultMotionParam(i);
        }
        else
        {

            float factor = 0;

            if (i==0)
            {
            	factor = 0.022;
            }
            else
            {
            	factor = 0.023;
            }
            PeristalticPumpManager_SetFactor(i, factor);//需要先设置factor,不然factor为， motionParam无法设置成功

            //param.acceleration = 100 * factor;
            //param.maxSpeed = 200 * factor;
            param.acceleration = 400 * factor;
            param.maxSpeed = 400 * factor;
            PeristalticPumpManager_SetDefaultMotionParam(i, param);

            flashFactorySign = FLASH_FACTORY_SIGN;
            memcpy(buffer, &flashFactorySign, PUMP_SIGN_FLASH_LEN);
            McuFlash_Write(
                    PUMP_SIGN_FLASH_BASE_ADDR + PUMP_SIGN_FLASH_LEN * i,
                    PUMP_SIGN_FLASH_LEN, buffer);
        }
        param.acceleration = param.acceleration / g_peristalticPumps[i].factor;
        param.maxSpeed = param.maxSpeed / g_peristalticPumps[i].factor;
        StepperMotor_SetNumber(&g_peristalticPumps[i].stepperMotor, i);//设置步进电机的变化编号,设置为1和2
        StepperMotor_Init(&g_peristalticPumps[i].stepperMotor, param);
        StepperMotor_RegisterFinishHandle(&g_peristalticPumps[i].stepperMotor, PeristalticPumpManager_FinishHandler);//蠕动泵结束后处理回调函数

        g_peristalticPumps[i].isSendEvent = FALSE;
        g_peristalticPumps[i].checkPoint = CHECKPOINT_IDLE;
        g_peristalticPumps[i].waitPumpStop = FALSE;
    }

    xTaskCreate(Pump1ToCheckPoint_TaskHandle, "Pump1Point", PUMPTOCHECKPOINT_STK_SIZE, (void *)&g_peristalticPumps[0],
    		PUMPTOCHECKPOINT_TASK_PRIO, &g_peristalticPumps[0].xHandle);

    xTaskCreate(Pump2ToCheckPoint_TaskHandle, "Pump2Point", PUMPTOCHECKPOINT_STK_SIZE, (void *)&g_peristalticPumps[1],
        		PUMPTOCHECKPOINT_TASK_PRIO, &g_peristalticPumps[1].xHandle);
}

void Pump1ToCheckPoint_TaskHandle(void *pvParameters)
{
	enum PumpResult pumpResult;
	PeristalticPump *g_peristalticPumps = (PeristalticPump *)pvParameters;
    while(1)
    {
    	vTaskDelay(5 / portTICK_RATE_MS);
    	g_peristalticPumps->currentCheckPointStatus = (CheckPointStatus)WaterCheckSensorBlocked(0);//监测液位传感器状态
		switch(g_peristalticPumps->checkPoint)
		{
		case CHECKPOINT_IDLE://空闲
			vTaskSuspend(NULL);
			break;
		case CHECKPOINT_START://开始
			if(g_peristalticPumps->targetStatus < MaxCheckPointStatus)
			{
				if(g_peristalticPumps->currentCheckPointStatus)//遮挡
				{
					g_peristalticPumps->checkPoint = HIGHLEVEL_STATUS;
				}
				else//未遮挡
				{
					g_peristalticPumps->checkPoint = LOWLEVEL_STATUS;
				}
			}
			else
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_FAILED;
			}
			break;
		case LOWLEVEL_STATUS://液位传感器为低电平,未遮住
			if(TRUE == g_peristalticPumps->waitPumpStop)
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_STOPPED;
			}
			if(g_peristalticPumps->currentCheckPointStatus == g_peristalticPumps->targetStatus)//传感器未遮住并且目标状态为未遮住，切换为完成
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_FINISHED;
			}
			else
			{
				if(Coverd == g_peristalticPumps->targetStatus)//目标为遮住状态
				{
					if(g_peristalticPumps->currentCheckPointStatus == Coverd)
					{
						g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
						pumpResult = PUMP_RESULT_FINISHED;
					}
					else//判断体积是否超出
					{
						if(g_peristalticPumps->maxVolume <= (PeristalticPumpManager_GetVolume(0) + 2) && (StepperMotor_IDLE == PeristalticPumpManager_GetStatus(0)))
						{
							TRACE_INFO("\n ## Pump 2 has Out maxVolume");
							g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
							pumpResult = PUMP_RESULT_FAILED;
						}
					}
				}
			}
			break;
		case HIGHLEVEL_STATUS://液位传感器为高电平,遮住
			if(TRUE == g_peristalticPumps->waitPumpStop)
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_STOPPED;
			}
			if(g_peristalticPumps->currentCheckPointStatus == g_peristalticPumps->targetStatus)//传感器遮住并且目标状态为遮住状态，切换为完成
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_FINISHED;
			}
			else
			{
				if(NotCoverd == g_peristalticPumps->targetStatus)//目标状态为未遮住状态
				{
					if(g_peristalticPumps->currentCheckPointStatus == NotCoverd)
					{
						g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
						pumpResult = PUMP_RESULT_FINISHED;
					}
					else//判断体积是否超出
					{
						if(g_peristalticPumps->maxVolume <= (PeristalticPumpManager_GetVolume(0) + 2) && (StepperMotor_IDLE == PeristalticPumpManager_GetStatus(0)))
						{
							TRACE_INFO("\n ## Pump 2 has Out maxVolume");
							g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
							pumpResult = PUMP_RESULT_FAILED;
						}
					}
				}
			}
			break;
		case CHECKPOINT_FINISH://完成
			if(pumpResult == PUMP_RESULT_FINISHED)
			{
				TRACE_INFO("\n Pump 1 completion target status");
			}
			else
			{
				TRACE_INFO("\n Pump 1 has not completed the target status");
			}
			StepperMotor_ImmediatelyStop(&g_peristalticPumps->stepperMotor);
			vTaskDelay(1000);
			if(TRUE != g_peristalticPumps->waitPumpStop)
			{
				TRACE_INFO("\n Pump 1 Send Event %d",pumpResult);
				PumpStartToCheckPoint_SendEvent(g_peristalticPumps,pumpResult);
			}
			g_peristalticPumps->waitPumpStop = FALSE;
			g_peristalticPumps->checkPoint = CHECKPOINT_IDLE;
			break;
		default:
			vTaskSuspend(NULL);
			break;
		}
    }
}

void Pump2ToCheckPoint_TaskHandle(void *pvParameters)
{
	enum PumpResult pumpResult;
	PeristalticPump *g_peristalticPumps = (PeristalticPump *)pvParameters;
    while(1)
    {
    	vTaskDelay(5 / portTICK_RATE_MS);
    	g_peristalticPumps->currentCheckPointStatus = (CheckPointStatus)WaterCheckSensorBlocked(1);//监测液位传感器状态
		switch(g_peristalticPumps->checkPoint)
		{
		case CHECKPOINT_IDLE://空闲
			vTaskSuspend(NULL);
			break;
		case CHECKPOINT_START://开始
			if(g_peristalticPumps->targetStatus < MaxCheckPointStatus)
			{
				if(g_peristalticPumps->currentCheckPointStatus)//遮挡
				{
					g_peristalticPumps->checkPoint = HIGHLEVEL_STATUS;
				}
				else//未遮挡
				{
					g_peristalticPumps->checkPoint = LOWLEVEL_STATUS;
				}
			}
			else
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_FAILED;
			}
			break;
		case LOWLEVEL_STATUS://液位传感器为低电平,未遮住
			if(TRUE == g_peristalticPumps->waitPumpStop)
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_STOPPED;
			}
			if(g_peristalticPumps->currentCheckPointStatus == g_peristalticPumps->targetStatus)//传感器未遮住并且目标状态为未遮住，切换为完成
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_FINISHED;
			}
			else
			{
				if(Coverd == g_peristalticPumps->targetStatus)//目标为遮住状态
				{
					if(g_peristalticPumps->currentCheckPointStatus == Coverd)
					{
						g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
						pumpResult = PUMP_RESULT_FINISHED;
					}
					else//判断体积是否超出
					{
						if(g_peristalticPumps->maxVolume <= (PeristalticPumpManager_GetVolume(1) + 2) && (StepperMotor_IDLE == PeristalticPumpManager_GetStatus(1)))
						{
							TRACE_INFO("\n ## Pump 2 has Out maxVolume");
							g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
							pumpResult = PUMP_RESULT_FAILED;
						}
					}
				}
			}
			break;
		case HIGHLEVEL_STATUS://液位传感器为高电平,遮住
			if(TRUE == g_peristalticPumps->waitPumpStop)
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_STOPPED;
			}
			if(g_peristalticPumps->currentCheckPointStatus == g_peristalticPumps->targetStatus)//传感器遮住并且目标状态为遮住状态，切换为完成
			{
				g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
				pumpResult = PUMP_RESULT_FINISHED;
			}
			else
			{
				if(NotCoverd == g_peristalticPumps->targetStatus)//目标状态为未遮住状态
				{
					if(g_peristalticPumps->currentCheckPointStatus == NotCoverd)
					{
						g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
						pumpResult = PUMP_RESULT_FINISHED;
					}
					else//判断体积是否超出
					{
						if(g_peristalticPumps->maxVolume <= (PeristalticPumpManager_GetVolume(1) + 2) && (StepperMotor_IDLE == PeristalticPumpManager_GetStatus(1)))
						{
							TRACE_INFO("\n ## Pump 2 has Out maxVolume");
							g_peristalticPumps->checkPoint = CHECKPOINT_FINISH;
							pumpResult = PUMP_RESULT_FAILED;
						}
					}
				}
			}
			break;
		case CHECKPOINT_FINISH://完成
			if(pumpResult == PUMP_RESULT_FINISHED)
			{
				TRACE_INFO("\n Pump 2 completion target status");
			}
			else
			{
				TRACE_INFO("\n Pump 2 has not completed the target status");
			}
			StepperMotor_ImmediatelyStop(&g_peristalticPumps->stepperMotor);
			vTaskDelay(1000);
			if(TRUE != g_peristalticPumps->waitPumpStop)
			{
				TRACE_INFO("\n Pump 2 Send Event %d",pumpResult);
				PumpStartToCheckPoint_SendEvent(g_peristalticPumps,pumpResult);
			}
			g_peristalticPumps->waitPumpStop = FALSE;
			g_peristalticPumps->checkPoint = CHECKPOINT_IDLE;
			break;
		default:
			vTaskSuspend(NULL);
			break;
    }
	}
}

//定标发送事件
static void PumpStartToCheckPoint_SendEvent(PeristalticPump *Pump, enum PumpResult pumpResult)
{
    Uint8 data[2] = {0};
    data[0] = Pump->stepperMotor.number;
    data[1] = pumpResult;
    if (1 == Pump->stepperMotor.number)
    {
        PumpEventScheduler_SendEvent(data[0], pumpResult, TRUE);
        DncpStack_BufferEvent(DSCP_EVENT_PPI_PUMP_1_RESULT, data , sizeof(data));
    }
    else
    {
        PumpEventScheduler_SendEvent(data[0], pumpResult, TRUE);
        DncpStack_BufferEvent(DSCP_EVENT_PPI_PUMP_0_RESULT, data , sizeof(data));
    }

    Pump->isSendEvent = FALSE;
}

void PeristalticPumpManager_FinishHandler(void *obj)
{
    StepperMotor *stepperMotor = (StepperMotor *)obj;
    Uint8 number = StepperMotor_GetNumber(stepperMotor);//蠕动泵号=步进电机号=slaveAddr //- PUMP_STEP_MOTOR_OFFSET;
    if(TRUE == g_peristalticPumps[number].isSendEvent)
    {
        enum PumpResult pumpResult = (enum PumpResult)StepperMotor_GetMoveResult(stepperMotor);
        TRACE_INFO("\n Pump FinishHandler %d",pumpResult);
        Uint8 data[2] = {0};
        data[0] = number;
        data[1] = pumpResult;
        if (1 == data[0])
        {
            PumpEventScheduler_SendEvent(data[0], pumpResult, TRUE);
            DncpStack_BufferEvent(DSCP_EVENT_PPI_PUMP_1_RESULT, data , sizeof(data));
        }
        else
        {
            PumpEventScheduler_SendEvent(data[0], pumpResult, TRUE);
            DncpStack_BufferEvent(DSCP_EVENT_PPI_PUMP_0_RESULT, data , sizeof(data));
        }
    }
    g_peristalticPumps[number].isSendEvent = FALSE;
    float alreadyVolume = (float) (StepperMotor_GetAlreadyStep(stepperMotor) * g_peristalticPumps[number].factor);
    TRACE_INFO("\n Peristaltic Pump stop, alreadyVolume:");
    System_PrintfFloat(TRACE_LEVEL_INFO, alreadyVolume, 4);
    TRACE_INFO(" ml");
}

void PeristalticPumpManager_Restore(void)
{
    for(Uint8 i = 0; i < PERISTALTICPUMPMANAGER_TOTAL_PUMP; i++)
    {
		g_peristalticPumps[i].isSendEvent = FALSE;
		PeristalticPumpManager_Stop(i);
    }
}

StepperMotor* PeristalticPumpManager_GetStepperMotor(Uint8 index)
{
    return &g_peristalticPumps[index].stepperMotor;
}

/**
 * @brief 查询泵出的体积。
 * @note 启动泵到停止泵的过程中，泵的转动体积（步数）。
 * @param index 要查询的泵索引，0号泵为光学定量泵。
 * @return 泵出的体积，单位为 ml
 */
float PeristalticPumpManager_GetVolume(Uint8 index)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
        float alreadyVolume = StepperMotor_GetAlreadyStep(&g_peristalticPumps[index].stepperMotor) * g_peristalticPumps[index].factor;
        return alreadyVolume;
    }
    else
    {
        TRACE_ERROR("\n No. %d pump.", index);
    }
    return 0;
}

/**
 * @brief 查询指定泵的运动参数。
 * @note 系统将根据设定的运动参数进行运动控制和规划。
 * @param index 要查询的泵索引，0号泵为光学定量泵
 * @return 运动参数(参数错误为0)，数据格式为：
 *                  acceleration Float32，加速度，单位为 ml/平方秒。
 *                  speed Float32，最大速度，单位为 ml/秒。
 */
StepperMotorParam PeristalticPumpManager_GetDefaultMotionParam(Uint8 index)
{
    StepperMotorParam param;
    memset(&param, 0, sizeof(StepperMotorParam));

    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
        Uint8 readData[PUMP_MOTIONPARAM_FLASH_LEN] ={ 0 };

        McuFlash_Read(PUMP_MOTIONPARAM_FLASH_BASE_ADDR + PUMP_MOTIONPARAM_FLASH_LEN * index,
                PUMP_MOTIONPARAM_FLASH_LEN, readData);

        memcpy(&param, readData, sizeof(StepperMotorParam));
        return param;
    }
    else
    {
        TRACE_ERROR("\n No. %d pump.", index);
        return param;
    }
}

StepperMotorParam PeristalticPumpManager_GetCurrentMotionParam(Uint8 index)
{
    StepperMotorParam param;
    memset(&param, 0, sizeof(StepperMotorParam));

    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
        param = StepperMotor_GetCurrentMotionParam(&g_peristalticPumps[index].stepperMotor);
        param.acceleration = param.acceleration * g_peristalticPumps[index].factor;
        param.maxSpeed = param.maxSpeed * g_peristalticPumps[index].factor;
        return param;
    }
    else
    {
        TRACE_ERROR("\n No. %d pump.", index);
        return param;
    }
}

/**
 * @brief 查询指定泵的校准系数。
 * @param index 要查询的泵索引，0号泵为光学定量泵。
 * @return 校准系数， 每步泵出的体积，单位为 ml/步。 参数错误为0
 */
float PeristalticPumpManager_GetFactor(Uint8 index)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
        Uint8 readData[PUMP_FACTOR_FLASH_LEN] ={ 0 };
        float factor;

        //获取FLASH的系数
        McuFlash_Read(PUMP_FACTOR_FLASH_BASE_ADDR + PUMP_FACTOR_FLASH_LEN * index,
                PUMP_FACTOR_FLASH_LEN, readData);
        memcpy(&factor, readData, sizeof(float));
        g_peristalticPumps[index].factor = factor;
        return factor;
    }
    else
    {
        TRACE_ERROR("\n No. %d pump.", index);
    }
    return 0;
}

/**
 * @brief 查询系统支持的总泵数目。
 * @return 总泵数目， Uint16。
 */
Uint16 PeristalticPumpManager_GetTotalPumps(void)
{
    return PERISTALTICPUMPMANAGER_TOTAL_PUMP;
}

/**
 * @brief 设置指定泵的运动参数。
 * @note 系统将根据设定的运动参数进行运动控制和规划。运动参数将永久保存。
 * @param index 要设置的泵索引，0号泵为光学定量泵
 * @param acceleration 加速度，单位为 ml/平方秒
 * @param maxSpeed 最大速度，单位为 ml/秒
 * @return 状态回应 TRUE 操作成功; FALSE 操作失败(参数错误)
 */
Uint16 PeristalticPumpManager_SetDefaultMotionParam(Uint8 index, StepperMotorParam param)
{
    float lowSpeed = STEPPERMOTOR_MIN_SUBDIVISION_SPEED  / StepperMotor_GetSubdivision(&g_peristalticPumps[index].stepperMotor) * g_peristalticPumps[index].factor;
    float highSpeed = STEPPERMOTOR_MAX_SUBDIVISION_SPEED / StepperMotor_GetSubdivision(&g_peristalticPumps[index].stepperMotor) * g_peristalticPumps[index].factor;

    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP && 0 != param.acceleration
            && (param.maxSpeed >= lowSpeed)
            && (param.maxSpeed <= highSpeed)
            && 0 != g_peristalticPumps[index].factor)
    {
        TRACE_INFO("\n Peristaltic Pump %d set default param acc :", index);
        System_PrintfFloat(TRACE_LEVEL_INFO, param.acceleration, 4);
        TRACE_INFO(" ml/(s^2),maxSpeed:");
        System_PrintfFloat(TRACE_LEVEL_INFO, param.maxSpeed, 4);
        TRACE_INFO(" ml/s,factor");
        System_PrintfFloat(TRACE_LEVEL_INFO, g_peristalticPumps[index].factor, 4);
        TRACE_INFO(" ml/step");

        StepperMotorParam setparam;
        setparam.acceleration = param.acceleration / g_peristalticPumps[index].factor;
        setparam.maxSpeed = param.maxSpeed / g_peristalticPumps[index].factor;

        if (TRUE == StepperMotor_SetDefaultMotionParam(&g_peristalticPumps[index].stepperMotor, setparam))
        {
            Uint8 writeData[PUMP_MOTIONPARAM_FLASH_LEN] ={ 0 };
            memcpy(writeData, &param, sizeof(StepperMotorParam));
            McuFlash_Write(
                    PUMP_MOTIONPARAM_FLASH_BASE_ADDR + PUMP_MOTIONPARAM_FLASH_LEN * index,
                    PUMP_MOTIONPARAM_FLASH_LEN, writeData);
            return DSCP_OK;
        }
        else
        {
            return DSCP_ERROR;
        }
    }
    else
    {
        TRACE_ERROR(
                "\n The pump %d motion default parameter setting failed because of the parameter error.", index);

        TRACE_ERROR("\n Pump %d set default param maxSpeed :", index);
        System_PrintfFloat(TRACE_LEVEL_ERROR, param.maxSpeed, 4);
        TRACE_ERROR(" ml/s,acc:");
        System_PrintfFloat(TRACE_LEVEL_ERROR, param.acceleration, 4);
        TRACE_ERROR(" ml/(s^2)\n factor:");
        System_PrintfFloat(TRACE_LEVEL_ERROR, g_peristalticPumps[index].factor, 4);
        TRACE_ERROR(" ml/step\n lowSpeed :");
        System_PrintfFloat(TRACE_LEVEL_ERROR, lowSpeed, 4);
        TRACE_ERROR(" ml/s\n highSpeed :");
        System_PrintfFloat(TRACE_LEVEL_ERROR, highSpeed, 4);
        TRACE_ERROR(" ml/s\n");

        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

Uint16 PeristalticPumpManager_SetCurrentMotionParam(Uint8 index, StepperMotorParam param)
{
    float lowSpeed = STEPPERMOTOR_MIN_SUBDIVISION_SPEED  / StepperMotor_GetSubdivision(&g_peristalticPumps[index].stepperMotor) * g_peristalticPumps[index].factor;
    float highSpeed = STEPPERMOTOR_MAX_SUBDIVISION_SPEED / StepperMotor_GetSubdivision(&g_peristalticPumps[index].stepperMotor) * g_peristalticPumps[index].factor;

    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP && 0 != param.acceleration
            && (param.maxSpeed >= lowSpeed)
            && (param.maxSpeed <= highSpeed)
            && 0 != g_peristalticPumps[index].factor)
    {
        TRACE_INFO("\n Pump %d set current param maxSpeed:", index);
        System_PrintfFloat(TRACE_LEVEL_INFO, param.maxSpeed, 4);
        TRACE_INFO(" ml/s,acc:");
        System_PrintfFloat(TRACE_LEVEL_INFO, param.acceleration, 4);
        TRACE_INFO(" ml/(s^2) ");
        System_PrintfFloat(TRACE_LEVEL_INFO, g_peristalticPumps[index].factor, 4);
        TRACE_INFO(" ml/step\n");

        StepperMotorParam setparam;
        setparam.acceleration = param.acceleration / g_peristalticPumps[index].factor;
        setparam.maxSpeed = param.maxSpeed / g_peristalticPumps[index].factor;

        StepperMotor_SetCurrentMotionParam(&g_peristalticPumps[index].stepperMotor, setparam);
        return DSCP_OK;
    }
    else
    {
        TRACE_ERROR(
                "\n The pump %d motion current parameter setting failed because of the parameter error.", index);

        TRACE_ERROR("\n Pump %d set current param maxSpeed :", index);
        System_PrintfFloat(TRACE_LEVEL_ERROR, param.maxSpeed, 4);
        TRACE_ERROR(" ml/s,acc:");
        System_PrintfFloat(TRACE_LEVEL_ERROR, param.acceleration, 4);
        TRACE_ERROR(" ml/(s^2)\n factor:");
        System_PrintfFloat(TRACE_LEVEL_ERROR, g_peristalticPumps[index].factor, 4);
        TRACE_ERROR(" ml/step\n lowSpeed :");
        System_PrintfFloat(TRACE_LEVEL_ERROR, lowSpeed, 4);
        TRACE_ERROR(" ml/s\n highSpeed :");
        System_PrintfFloat(TRACE_LEVEL_ERROR, highSpeed, 4);
        TRACE_ERROR(" ml/s\n");

        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

/**
 * @brief 设置指定泵的校准系数。
 * @note 因为蠕动泵的生产工艺及工作时长，每个泵转一步对应体积都不同，出厂或使用时需要校准。 该参数将永久保存。
 * @param index 要设置的泵索引，0号泵为光学定量泵。
 * @param factor 要设置的校准系数， 每步泵出的体积，单位为 ml/步。
 * @return 状态回应 TRUE 操作成功; FALSE 操作失败(参数错误)
 */
Uint16 PeristalticPumpManager_SetFactor(Uint8 index, float factor)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP && 0 != factor)
    {
        if (StepperMotor_IDLE == StepperMotor_GetStatus(&g_peristalticPumps[index].stepperMotor))
        {
            TRACE_INFO("\n Peristaltic Pump %d set factor:", index);
            System_PrintfFloat(TRACE_LEVEL_INFO, factor, 8);
            TRACE_INFO(" ml/step");

            Uint8 writeData[PUMP_FACTOR_FLASH_LEN] ={ 0 };

            g_peristalticPumps[index].factor = factor;
            //系数写入FLASH
            memcpy(writeData, &factor, sizeof(float));
            McuFlash_Write(
                    PUMP_FACTOR_FLASH_BASE_ADDR + PUMP_FACTOR_FLASH_LEN * index,
                    PUMP_FACTOR_FLASH_LEN, writeData);

            StepperMotorParam setparam = PeristalticPumpManager_GetDefaultMotionParam(index);
            setparam.acceleration = setparam.acceleration / g_peristalticPumps[index].factor;
            setparam.maxSpeed = setparam.maxSpeed / g_peristalticPumps[index].factor;
            //将param拷贝给stepperMotor->flashParam
            StepperMotor_SetDefaultMotionParam(&g_peristalticPumps[index].stepperMotor, setparam);

            return DSCP_OK;
        }
        else
        {
            TRACE_ERROR(
                    "\n The pump %d is running and can not change the calibration factor.", index);
            return DSCP_ERROR;
        }
    }
    else
    {
        TRACE_ERROR(
                "\n The pump %d calibration factor setting failed because of the parameter error.", index);
        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

/**
 * @brief 启动泵。
 * @note 启动后，不管成功与否，操作结果都将以事件的形式上传给上位机。
 * @param index 要操作的泵索引，0号泵为光学定量泵
 * @param dir 泵转动方向，0为正向转动（抽取），1为反向转动（排空）。
 * @param volume Float32，泵取/排空体积，单位为 ml。
 * @return 状态回应 TRUE 操作成功; FALSE 操作失败如泵正在工作，无法启动泵，需要先停止
 */
Uint16 PeristalticPumpManager_Start(Uint8 index, Direction dir, float volume, Bool isUseDefaultParam)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP && volume > 0 && dir < MAX_DIRECTION)
    {
        if (StepperMotor_IDLE == StepperMotor_GetStatus(&g_peristalticPumps[index].stepperMotor))
        {
            Uint32 step = (Uint32) (volume / g_peristalticPumps[index].factor + 0.5);
            if (FORWARD == dir)
            {
                TRACE_INFO("\n PeristalticPump start extract volume ");
            }
            else
            {
                TRACE_INFO("\n PeristalticPump start drain volume ");
            }
            System_PrintfFloat(TRACE_LEVEL_INFO, volume, 3);
            TRACE_INFO(" ml");
            if  (TRUE == StepperMotor_Start(&g_peristalticPumps[index].stepperMotor, dir, step, isUseDefaultParam, NULL/*StepperMotor_DiagnosticCheck*/))  ///<驱动诊断
            {
                DncpStack_ClearBufferedEvent();
                return DSCP_OK;
            }
            else
            {
                TRACE_ERROR("\n Because no idle timer , the pump %d starts to fail.", index);
                return DSCP_ERROR;
            }
        }
        else
        {
            TRACE_ERROR("\n Because the pump is running, the pump %d starts to fail.", index);
            return DSCP_BUSY;
        }
    }
    else
    {
        TRACE_ERROR(
                "\n Because of the parameter error, the pump %d starts to fail. ", index);
        TRACE_ERROR(
                "\n index %d volume %f dir %d ", index,volume,dir);
        if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
        {
            g_peristalticPumps[index].isSendEvent = FALSE;
        }
        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

Uint16 PeristalticPumpManager_StartToPoint(Uint8 index, Direction dir, CheckPointStatus targetStatus, float volume, Bool isUseDefaultParam)
{
	TRACE_INFO("\n StartToPoint index = %d , dir = %d ,CheckPointStatus = %d ,volume = %f ",index,dir,targetStatus,volume);
	TRACE_INFO("\n");
	if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP && volume > 0 && dir < MAX_DIRECTION)
	{
		g_peristalticPumps[index].targetStatus = targetStatus;
		g_peristalticPumps[index].maxVolume = volume;
		g_peristalticPumps[index].checkPoint = CHECKPOINT_START;//状态转换为开始检测状态
		g_peristalticPumps[index].waitPumpStop = FALSE;
		g_peristalticPumps[index].isSendEvent = FALSE;
		vTaskResume(g_peristalticPumps[index].xHandle);//恢复对应的液位传感器检测任务

		Uint32 step = (Uint32) (volume / g_peristalticPumps[index].factor + 0.5);
		if (FORWARD == dir)
		{
		     TRACE_INFO("\n PeristalticPump start extract volume ");
		}
		else
		{
		     TRACE_INFO("\n PeristalticPump start drain volume ");
		}
		System_PrintfFloat(TRACE_LEVEL_INFO, volume, 3);
		TRACE_INFO(" ml");
		if  (TRUE == StepperMotor_Start(&g_peristalticPumps[index].stepperMotor, dir, step, isUseDefaultParam, NULL/*StepperMotor_DiagnosticCheck*/))  ///<驱动诊断
		{
		     DncpStack_ClearBufferedEvent();
		     return DSCP_OK;
		}
		else
		{
		     TRACE_ERROR("\n Because no idle timer , the pump %d starts to fail.", index);
		     return DSCP_ERROR;
		}
	}
	else
	{
		TRACE_ERROR(
				     "\n Because of the parameter error, the pump %d starts to fail. ", index);
		return ((Uint16) DSCP_ERROR_PARAM);
	}
}
/**
 * @brief 停止泵。
 * @param index 要操作的泵索引，0号泵为光学定量泵。
 * @return 状态回应 TRUE 操作成功; FALSE 操作失败如泵已停止
 */
Uint16 PeristalticPumpManager_Stop(Uint8 index)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
    	if(CHECKPOINT_IDLE != g_peristalticPumps[index].checkPoint)
    	{
    		TRACE_ERROR("\n ********************** PeristalticPumpManager_Stop.");
    		g_peristalticPumps[index].waitPumpStop = TRUE;
    	}
        if(StepperMotor_BUSY == StepperMotor_GetStatus(&g_peristalticPumps[index].stepperMotor) && TRUE == StepperMotor_RequestStop(&g_peristalticPumps[index].stepperMotor))
        {
            return DSCP_OK;
        }
        else
        {
            TRACE_ERROR("\n pump %d not run, stop failure.", index);
            return DSCP_ERROR;
        }
    }
    else
    {
        TRACE_ERROR(
                "\n parameter error, the pump %d stop fail. ", index);
        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

void PeristalticPumpManager_ChangeVolume(Uint8 index, float volume)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP && volume > 0)
    {
        if (StepperMotor_BUSY == StepperMotor_GetStatus(&g_peristalticPumps[index].stepperMotor))
        {
            Uint32 step = (Uint32) (volume / g_peristalticPumps[index].factor + 0.5);
            StepperMotor_ChangeStep(&g_peristalticPumps[index].stepperMotor, step);
        }
        else
        {
            TRACE_ERROR("\n Because the pump %d does not run, change volume failure.", index);
        }
    }
    else
    {
        TRACE_ERROR(
                "\n Because of the parameter error, the pump %d change volume to fail. ", index);
    }
}

Bool PeristalticPumpManager_SendEventOpen(Uint8 index)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
        g_peristalticPumps[index].isSendEvent = TRUE;
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n No. %d pump.", index);
    }
    return FALSE;
}

/**
 * @brief  查询指定泵的工作状态。
 * @param index 要设置的泵索引，0号泵为光学定量泵。
 * @return  状态回应  空闲，忙碌，需要停止后才能做下一个动作
 */
StepperMotorStatus PeristalticPumpManager_GetStatus(Uint8 index)
{
    if (index < PERISTALTICPUMPMANAGER_TOTAL_PUMP)
    {
        return StepperMotor_GetStatus(&g_peristalticPumps[index].stepperMotor);
    }
    else
    {
        TRACE_ERROR("\n No. %d pump.", index);
    }
    return StepperMotor_IDLE;
}

//查询液位传感器的状态
Bool WaterCheckSensorBlocked(Uint8 index)
{
    Bool ret;
    if (index < WATERSENSORNUM)//液位传感器的数量
    {
        if (SENSOR_LOW_LEVEL == PositionSensor_ReadInputStatus(&g_waterCheckSensor[index]))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        TRACE_ERROR("\n No. %d Water Check Sensor.", index);
        ret = FALSE;
    }
    return ret;
}

