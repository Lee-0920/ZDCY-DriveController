/*
 * DisplacementMotorManager.c
 *
 *  Created on: 2018年3月7日
 *      Author: LIANG
 */
#include "DNCP/App/DscpSysDefine.h"
#include "System.h"
#include "Driver/LiquidDriver/PositionSensorMap.h"
#include "Driver/LiquidDriver/StepperMotorMap.h"
#include "StepperMotor.h"
#include "SystemConfig.h"
#include "Driver/McuFlash.h"
#include "DncpStack/DncpStack.h"
#include "DisplacementSteperMotorManager.h"
#include <stdlib.h>
#include "LuipApi/MotorControlInterface.h"
//位移电机
DisplacementSteperMotor g_displacementMotors[DISPLACEMENTMOTOR_TOTAL_PUMP];

#define LIMITSTEPMAX 15
#define LIMITSTEPMAX_TWO 4
#define MAXOFFLIMITSTEP 10
#define SUBDIVISION_LOCATION 4
#define MAXIMUMOFFLIMITSTEP 1350


static DisplacementMotorType s_displacementMotorType = DISPLACEMENTMOTOR_DC;


/*********Task*********/
static void DisplacementSteperMotor_TaskHandle(void *pvParameters);
/*********Function*********/
static Bool DisplacementSteperMotor_IsSensorBlocked(Uint8 index);

static void DisplacementMotor_ResetHandle(void* obj);
static void DisplacementMotor_MoveToTargetLocationHandle(void* obj);
static void DisplacementMotor_MoveToSensorOffLimitHandle(void *obj);
static void DisplacementMotor_CForWardMoveOffLimitHandle(void *obj);
static void DisplacementMotor_CMoveToZerosOffLimitHandle(void *obj);
static Bool DisplacementMotor_CMoveCheckPosition(DisplacementSteperMotor *displacementMotor, Direction dir);
static void DisplacementSteperMotor_SendEvent(DisplacementSteperMotor *displacementMotor, MoveResult moveResult);
static void DisplacementSteperMotor_Stop(DisplacementSteperMotor *displacementMotor, MoveResult moveResult);
static void DisplacementMotor_Check(void* obj);


/*********Variable*********/
static StepperMotorParam s_resetCParam = {200, 800};


 

void DisplacementSteperMotor_Init()
{
    memset(g_displacementMotors, 0, sizeof(DisplacementSteperMotor) * DISPLACEMENTMOTOR_TOTAL_PUMP);

    PositionSensorMap_DisplacementSteperMotorInit(g_displacementMotors);//位置传感器初始化
    StepperMotorMap_DisplacementMotorInit(g_displacementMotors);
    //TRACE_INFO("file:%s -> line: %d\n",__FILE__,__LINE__);

    for (Uint8 i = 0; i < DISPLACEMENTMOTOR_TOTAL_PUMP; i++)
    {
        if(i == C_DISPLACEMENTMOTOR)
        {
            StepperMotor_SetSubdivision(&g_displacementMotors[i].stepperMotor, SUBDIVISION_LOCATION);
        }

        Uint8 buffer[DISPLACEMENTMOTOR_SIGN_FLASH_LEN] = { 0 };
        Uint32 flashFactorySign = 0;
        StepperMotorParam param;

        McuFlash_Read(DISPLACEMENTMOTOR_SIGN_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_SIGN_FLASH_LEN * i,
                DISPLACEMENTMOTOR_SIGN_FLASH_LEN, buffer); //读取出厂标志位
        memcpy(&flashFactorySign, buffer, DISPLACEMENTMOTOR_SIGN_FLASH_LEN);

        if (FLASH_FACTORY_SIGN == flashFactorySign) //表示已经过出厂设置
        {
            //TRACE_INFO("%s:%d\n",__FILE__,__LINE__);
            param = DisplacementSteperMotor_GetDefaultMotionParam(i);
            if(param.acceleration != 400)
            {
            	param.acceleration = 400;
            	DisplacementSteperMotor_SetDefaultMotionParam(i, param);
            	flashFactorySign = FLASH_FACTORY_SIGN;
            	memcpy(buffer, &flashFactorySign, DISPLACEMENTMOTOR_SIGN_FLASH_LEN);
            	McuFlash_Write(DISPLACEMENTMOTOR_SIGN_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_SIGN_FLASH_LEN * i,
								DISPLACEMENTMOTOR_SIGN_FLASH_LEN, buffer);
            }
        }
        else
        {
            param.acceleration = 400;
            param.maxSpeed = 400;
            //TRACE_INFO("%s:%d\n",__FILE__,__LINE__);
            DisplacementSteperMotor_SetDefaultMotionParam(i, param);

            flashFactorySign = FLASH_FACTORY_SIGN;
            memcpy(buffer, &flashFactorySign, DISPLACEMENTMOTOR_SIGN_FLASH_LEN);
            McuFlash_Write(
                    DISPLACEMENTMOTOR_SIGN_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_SIGN_FLASH_LEN * i,
                    DISPLACEMENTMOTOR_SIGN_FLASH_LEN, buffer);
        }
        StepperMotor_SetNumber(&g_displacementMotors[i].stepperMotor, i);//设置步进电机编号为0
        StepperMotor_Init(&g_displacementMotors[i].stepperMotor, param);
        StepperMotor_SetPosLockStatus(&g_displacementMotors[i].stepperMotor, TRUE);

        g_displacementMotors[i].currentSteps = 0;		//当前位置
        g_displacementMotors[i].isRequestStop = FALSE;
        g_displacementMotors[i].isSendEvent = FALSE;
        g_displacementMotors[i].isStatusSwitchStart = FALSE;
        g_displacementMotors[i].limitStepCount = 0;		// 限制步数计数
        g_displacementMotors[i].moveHandler = NULL;
        g_displacementMotors[i].status = STEPERMOTOR_IDLE;	//空闲状态
        g_displacementMotors[i].returnStepCount = 0;	// 回归预测步数
        g_displacementMotors[i].isChangeSteps = FALSE;
    }
    g_displacementMotors[C_DISPLACEMENTMOTOR].maxSteps = 5400;//单次启动的最大运行步数
    			// 位移泵过程控制任务		优先级：6
    xTaskCreate(DisplacementSteperMotor_TaskHandle, "DisplacementSteperMotor", DISPLACEMENTMOTOR_STK_SIZE, (void *)&g_displacementMotors[C_DISPLACEMENTMOTOR],
            DISPLACEMENTMOTOR_TASK_PRIO, &g_displacementMotors[C_DISPLACEMENTMOTOR].xHandle);

}

void DisplacementSteperMotor_Restore()
{
    for (Uint8 i = 0; i < DISPLACEMENTMOTOR_TOTAL_PUMP; i++)
    {
        g_displacementMotors[i].isSendEvent = FALSE;
        DisplacementSteperMotor_RequestStop(i);
    }
}

StepperMotor* DisplacementMotor_GetStepperMotorC(void)
{
    return &g_displacementMotors[C_DISPLACEMENTMOTOR].stepperMotor;
}

StepperMotorParam DisplacementSteperMotor_GetDefaultMotionParam(Uint8 index)
{
    StepperMotorParam param;
    memset(&param, 0, sizeof(StepperMotorParam));

    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        Uint8 readData[DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN] ={ 0 };

        McuFlash_Read(DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN * index,
                DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN, readData);

        memcpy(&param, readData, sizeof(StepperMotorParam));
        return param;
    }
    else
    {
        TRACE_ERROR("\n No. %d  DisplacementMotor.", index);
        return param;
    }
}

StepperMotorParam DisplacementSteperMotor_GetCurrentMotionParam(Uint8 index)
{
    StepperMotorParam param;
    memset(&param, 0, sizeof(StepperMotorParam));

    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        param = StepperMotor_GetCurrentMotionParam(&g_displacementMotors[index].stepperMotor);
        return param;
    }
    else
    {
        TRACE_ERROR("\n No. %d DisplacementMotor.", index);
        return param;
    }
}

Uint16 DisplacementSteperMotor_SetDefaultMotionParam(Uint8 index, StepperMotorParam param)
{
    float lowSpeed = STEPPERMOTOR_MIN_SUBDIVISION_SPEED  / StepperMotor_GetSubdivision(&g_displacementMotors[index].stepperMotor);
    float highSpeed = STEPPERMOTOR_MAX_SUBDIVISION_SPEED / StepperMotor_GetSubdivision(&g_displacementMotors[index].stepperMotor);

    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP && 0 != param.acceleration
            && (param.maxSpeed >= lowSpeed)
            && (param.maxSpeed <= highSpeed))
    {
        TRACE_INFO("\n %s set default param acc :", DisplacementSteperMotor_GetName(index));
        System_PrintfFloat(TRACE_LEVEL_INFO, param.acceleration, 4);
        TRACE_INFO(" step/(s^2),maxSpeed:");
        System_PrintfFloat(TRACE_LEVEL_INFO, param.maxSpeed, 4);
        TRACE_INFO(" step/s");

        if (TRUE == StepperMotor_SetDefaultMotionParam(&g_displacementMotors[index].stepperMotor, param))
        {
            Uint8 writeData[DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN] ={ 0 };
            memcpy(writeData, &param, sizeof(StepperMotorParam));
            McuFlash_Write(
                    DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN * index,
                    DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN, writeData);
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
                "\n The DisplacementMotor %d motion default parameter setting failed because of the parameter error.", index);

        TRACE_ERROR("\n DisplacementMotor %d set default param maxSpeed :", index);
        System_PrintfFloat(TRACE_LEVEL_ERROR, param.maxSpeed, 4);
        TRACE_ERROR(" step/s,acc:");
        System_PrintfFloat(TRACE_LEVEL_ERROR, param.acceleration, 4);
        TRACE_ERROR(" step/(s^2)\n lowSpeed :");
        System_PrintfFloat(TRACE_LEVEL_ERROR, lowSpeed, 4);
        TRACE_ERROR(" step/s\n highSpeed :");
        System_PrintfFloat(TRACE_LEVEL_ERROR, highSpeed, 4);
        TRACE_ERROR(" step/s\n");

        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

Uint16 DisplacementSteperMotor_SetCurrentMotionParam(Uint8 index, StepperMotorParam param)
{
    float lowSpeed = STEPPERMOTOR_MIN_SUBDIVISION_SPEED  / StepperMotor_GetSubdivision(&g_displacementMotors[index].stepperMotor);
       float highSpeed = STEPPERMOTOR_MAX_SUBDIVISION_SPEED / StepperMotor_GetSubdivision(&g_displacementMotors[index].stepperMotor);

       if (index < DISPLACEMENTMOTOR_TOTAL_PUMP && 0 != param.acceleration
               && (param.maxSpeed >= lowSpeed)
               && (param.maxSpeed <= highSpeed))
       {
           TRACE_INFO("\n %s set current param maxSpeed:",  DisplacementSteperMotor_GetName(index));
           System_PrintfFloat(TRACE_LEVEL_INFO, param.maxSpeed, 4);
           TRACE_INFO(" step/s,acc:");
           System_PrintfFloat(TRACE_LEVEL_INFO, param.acceleration, 4);
           TRACE_INFO(" step/(s^2) ");

           StepperMotor_SetCurrentMotionParam(&g_displacementMotors[index].stepperMotor, param);
           return DSCP_OK;
       }
       else
       {
           TRACE_ERROR(
                   "\n The DisplacementMotor %d motion current parameter setting failed because of the parameter error.", index);

           TRACE_ERROR("\n DisplacementMotor %d set current param maxSpeed :", index);
           System_PrintfFloat(TRACE_LEVEL_ERROR, param.maxSpeed, 4);
           TRACE_ERROR(" step/s,acc:");
           System_PrintfFloat(TRACE_LEVEL_ERROR, param.acceleration, 4);
           TRACE_ERROR(" step/(s^2)\n lowSpeed :");
           System_PrintfFloat(TRACE_LEVEL_ERROR, lowSpeed, 4);
           TRACE_ERROR(" step/s\n highSpeed :");
           System_PrintfFloat(TRACE_LEVEL_ERROR, highSpeed, 4);
           TRACE_ERROR(" step/s\n");

           return ((Uint16) DSCP_ERROR_PARAM);
       }
}

DisplacementSteperMotorStatus DisplacementSteperMotor_GetStatus(Uint8 index)
{
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        return g_displacementMotors[index].status;
    }
    else
    {
        TRACE_ERROR("\n No. %d DisplacementMotor.", index);
        return STEPERMOTOR_IDLE;
    }
}

Uint32 DisplacementMotor_GetMaxSteps(Uint8 index)
{
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        return g_displacementMotors[index].maxSteps;
    }
    else
    {
        TRACE_ERROR("\n No. %d DisplacementMotor.", index);
        return 0;
    }
}
//步数转换为角度
Uint32 DisplacementMotor_GetCurrentSteps(Uint8 index)
{
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        return g_displacementMotors[index].currentSteps;
    }
    else
    {
        TRACE_ERROR("\n No. %d DisplacementMotor.", index);
        return 0;
    }
}

Bool DisplacementSteperMotor_IsSensorBlocked(Uint8 index)
{
    Bool ret;
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        if (SENSOR_HIGH_LEVEL == PositionSensor_ReadInputStatus(&g_displacementMotors[index].positionSensor))
        {
            ret = FALSE;
            //ret = TRUE;
        }
        else
        {
            ret = TRUE;
            //ret = FALSE;
        }
    }
    else
    {
        TRACE_ERROR("\n No. %d DisplacementMotor.", index);
        ret = FALSE;
    }
    return ret;
}


Bool DisplacementSteperMotor_SendEventOpen(Uint8 index)
{
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        g_displacementMotors[index].isSendEvent = TRUE;
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n Invalid No. %d DisplacementMotor.", index);
        return FALSE;
    }
}

MoveResult DisplacementMotor_GetMoveResult(Uint8 index)
{
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        return g_displacementMotors[index].moveResult;;
    }
    else
    {
        return RESULT_FAILED;
    }
}

char* DisplacementSteperMotor_GetName(Uint8 index)
{
    static char name[20] = "";
    memset(name, 0, sizeof(name));
    switch(index)
    {
    case C_DISPLACEMENTMOTOR:
        strcpy(name, "CDisplacementSteperMotor");
        break;
    default:
        strcpy(name, "NULL");
        break;
    }
    return name;
}

void DisplacementSteperMotor_TaskHandle(void *pvParameters)
{
    DisplacementSteperMotor *displacementMotor = (DisplacementSteperMotor *)pvParameters;
    vTaskDelay(1000 / portTICK_RATE_MS);
    DisplacementSteperMotor_CheckType(0);
    //vTaskSuspend(NULL);
    while(1)
    {
        vTaskDelay(5 / portTICK_RATE_MS);
        switch (displacementMotor->status)
        {
        case STEPERMOTOR_IDLE:
            vTaskSuspend(NULL);
            break;
        default:
            if (displacementMotor->moveHandler)
            {
                displacementMotor->moveHandler(displacementMotor);
            }
            break;
        }
    }
}

void DisplacementMotor_MoveToSensorOffLimitHandle(void *obj)
{
    StepperMotor *stepperMotor = (StepperMotor *)obj;
    Uint8 number = StepperMotor_GetNumber(stepperMotor);
    //TRACE_INFO("file:%s -> line: %d\n",__FILE__,__LINE__);
    if(FALSE == g_displacementMotors[number].isChangeSteps)
    {
        g_displacementMotors[number].returnStepCount--;
        if(g_displacementMotors[number].returnStepCount < 0)
        {
            TRACE_INFO("\nMotor %d Change Step = %d", number, g_displacementMotors[number].maxSteps + MAXIMUMOFFLIMITSTEP);
            StepperMotor_ChangeStep(stepperMotor, g_displacementMotors[number].maxSteps + MAXIMUMOFFLIMITSTEP);
            g_displacementMotors[number].isChangeSteps = TRUE;
            g_displacementMotors[number].returnStepCount = 0;
        }
    }

    // 如果遮住传感器
    if (TRUE == DisplacementSteperMotor_IsSensorBlocked(number))
    {
        g_displacementMotors[number].limitStepCount++;
        if (g_displacementMotors[number].limitStepCount >= LIMITSTEPMAX)
        {
        //停止运行
            StepperMotor_ImmediatelyStop(stepperMotor);
            TRACE_INFO("\n %s move to sensor.PositionSensor status %d", DisplacementSteperMotor_GetName(number), PositionSensor_ReadInputStatus(&g_displacementMotors[number].positionSensor));
        }
    }
    // 如果遮住传感器
//	if (TRUE == DisplacementSteperMotor_IsSensorBlocked(number))
//	{
//		StepperMotorDriver_SetDirection(&g_displacementMotors[0].stepperMotor.driver,FORWARD);
//		g_displacementMotors[number].limitStepCount++;
//		if (g_displacementMotors[number].limitStepCount >= 100)
//		{
//		//停止运行
//			StepperMotor_ImmediatelyStop(stepperMotor);
//			TRACE_INFO("\n %s move to sensor.PositionSensor status %d", DisplacementSteperMotor_GetName(number), PositionSensor_ReadInputStatus(&g_displacementMotors[number].positionSensor));
//		}
//	}
    else
    {
    	//TRACE_INFO("file:%s -> line: %d\n",__FILE__,__LINE__);
        g_displacementMotors[number].limitStepCount = 0;
    }

    //驱动报警诊断
    StepperMotor_DiagnosticCheck(stepperMotor);
}


void DisplacementMotor_CForWardMoveOffLimitHandle(void *obj)
{
    StepperMotor *stepperMotor = (StepperMotor *)obj;

    //驱动报警诊断
    StepperMotor_DiagnosticCheck(stepperMotor);
}

void DisplacementMotor_CMoveToZerosOffLimitHandle(void *obj)
{
    StepperMotor *stepperMotor = (StepperMotor *)obj;
    // 如果没遮住传感器
    if (FALSE == DisplacementSteperMotor_IsSensorBlocked(C_DISPLACEMENTMOTOR))
    {
        g_displacementMotors[C_DISPLACEMENTMOTOR].limitStepCount++;
        if (g_displacementMotors[C_DISPLACEMENTMOTOR].limitStepCount >= LIMITSTEPMAX_TWO)
        {
            //停止运行
            StepperMotor_ImmediatelyStop(stepperMotor);
            TRACE_INFO("\n %s move to zeros.PositionSensor status %d", DisplacementSteperMotor_GetName(C_DISPLACEMENTMOTOR)
                    , PositionSensor_ReadInputStatus(&g_displacementMotors[C_DISPLACEMENTMOTOR].positionSensor));
        }
    }
    else
    {
        g_displacementMotors[C_DISPLACEMENTMOTOR].limitStepCount = 0;
    }

    //驱动报警诊断
    StepperMotor_DiagnosticCheck(stepperMotor);
}

Bool DisplacementMotor_CMoveCheckPosition(DisplacementSteperMotor *displacementMotor, Direction dir)
{
    Bool ret = TRUE;
    if (BACKWARD == dir && TRUE == DisplacementSteperMotor_IsSensorBlocked(C_DISPLACEMENTMOTOR))//已在零点，不能向后退
    {
        ret = FALSE;
        TRACE_ERROR("\n C displacementMotor is at the starting point and cannot fall back");
    }

    return ret;
}

Uint16 DisplacementSteperMotor_Start(Uint8 index, Int16 step, DisplacementMotorMode mode, Bool isUseDefaultParam)
{
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP
            && abs(step) <=  g_displacementMotors[index].maxSteps//单次启动的最大运行步数
            && mode < MAX_MOTOR_MOVE_MODE)//判断是否在合理的范围内
    {
        if (STEPERMOTOR_IDLE == g_displacementMotors[index].status)//判断步进电机状态是否在空闲状态
        {
        	//根据电机的模式、步数和当前位置的步数计算出电机的运动方向dir和离目标点的步数targetStep
            if (MOTOR_MOVE_ABSOLUTE_MODE == mode)//绝对模式
            {
                 if (step >= 0 && step != g_displacementMotors[index].currentSteps)
                 {
                     //TRACE_INFO("%s : %d\n",__FILE__,__LINE__);
                     //TRACE_INFO("currentSteps :%d\n",g_displacementMotors[index].currentSteps);
                     if (step > g_displacementMotors[index].currentSteps)
                     {
                         g_displacementMotors[index].dir = FORWARD;//向前
                         g_displacementMotors[index].targetStep = step - g_displacementMotors[index].currentSteps;
                     }
                     else
                     {
                         g_displacementMotors[index].dir = BACKWARD;//向后
                         g_displacementMotors[index].targetStep = g_displacementMotors[index].currentSteps - step;
                     }
                 }
                 else
                 {
                     TRACE_ERROR("\n Because of the parameter error(mode = MOTOR_MOVE_ABSOLUTE_MODE , step < 0 or step = currentSteps), the %s starts to fail. ", DisplacementSteperMotor_GetName(index));
                     g_displacementMotors[index].dir = FORWARD;//向前
                     g_displacementMotors[index].targetStep = 10;
                     //return ((Uint16) DSCP_OK);
                 }
            }
            else//相对模式
            {
                if (step != 0)
                {
                    if (step > 0)//步数大于0，方向向前运动
                    {
                        g_displacementMotors[index].dir = FORWARD;
                        g_displacementMotors[index].targetStep = step;
                    }
                    else//步数小于0，方向向后运动
                    {
                        g_displacementMotors[index].dir = BACKWARD;
                        g_displacementMotors[index].targetStep = -1 * step;
                    }

                }
                else
                {
                    TRACE_ERROR("\n Because of the parameter error(mode = MOTOR_MOVE_RELATIVE_MODE or MOTOR_MOVE_SAFE_MODE , step = 0), the %s starts to fail.", DisplacementSteperMotor_GetName(index));
                    return ((Uint16) DSCP_OK);
                }
            }

            DisplacementSteperMotorStatus status = STEPERMOTOR_TO_TARGET_LOCATION;
					//位移电机移动检查位置,如果在零点并且方向向后则返回错误
            if (FALSE == DisplacementMotor_CMoveCheckPosition(&g_displacementMotors[C_DISPLACEMENTMOTOR], g_displacementMotors[C_DISPLACEMENTMOTOR].dir))
            {
                TRACE_ERROR("\n Because C DisplacementMotor position error, the C DisplacementMotor start fail.");
                return ((Uint16) DSCP_ERROR);
            }

            TRACE_INFO("\n %s start mode:%d, step:%d", DisplacementSteperMotor_GetName(index), mode, step);
            TRACE_INFO("\n dir: %d, targetStep:%d, currentSteps: %d", g_displacementMotors[index].dir, g_displacementMotors[index].targetStep, g_displacementMotors[index].currentSteps);

            g_displacementMotors[index].mode = mode;
            g_displacementMotors[index].isUseDefaultParam = isUseDefaultParam;

            g_displacementMotors[index].isRequestStop = FALSE;
            g_displacementMotors[index].limitStepCount = 0;

            g_displacementMotors[index].moveHandler = DisplacementMotor_MoveToTargetLocationHandle;//位移电机运动控制处理函数
            g_displacementMotors[index].status = status;
            g_displacementMotors[index].isStatusSwitchStart = TRUE;
            DncpStack_ClearBufferedEvent();
            vTaskResume(g_displacementMotors[index].xHandle);
            return DSCP_OK;
        }
        else
        {
            TRACE_ERROR("\n Because the DisplacementMotor is running, the %s starts to fail.", DisplacementSteperMotor_GetName(index));
            return DSCP_BUSY;
        }
    }
    else
    {
        TRACE_ERROR("\n Because of the parameter error, the DisplacementMotor %d starts to fail. ", index);
        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

Uint16 DisplacementSteperMotor_Reset(Uint8 index)
{
	//TRACE_INFO("file:%s -> line: %d",__FILE__,__LINE__);
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        if (STEPERMOTOR_IDLE == g_displacementMotors[index].status)
        {
            TRACE_INFO("\n %s start reset", DisplacementSteperMotor_GetName(index));
            g_displacementMotors[index].isRequestStop = FALSE;
            g_displacementMotors[index].limitStepCount = 0;

            g_displacementMotors[index].moveHandler = DisplacementMotor_ResetHandle;//电机复位回调处理函数
            g_displacementMotors[index].status = STEPERMOTOR_TO_SENSOR;//电机到传感器
            g_displacementMotors[index].isStatusSwitchStart = TRUE;
            DncpStack_ClearBufferedEvent();
            vTaskResume(g_displacementMotors[index].xHandle);
            return DSCP_OK;
        }
        else
        {
            TRACE_ERROR("\n Because the DisplacementMotor is running, the %s reset to fail.", DisplacementSteperMotor_GetName(index));
            return DSCP_BUSY;
        }
    }
    else
    {
        TRACE_ERROR("\n Because of the parameter error, the DisplacementMotor %d reset to fail. ", index);
        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

Uint16 DisplacementSteperMotor_RequestStop(Uint8 index)
{
    TRACE_INFO("\n%s:%d\n",__FILE__,__LINE__);
    if (index < DISPLACEMENTMOTOR_TOTAL_PUMP)
    {
        if(STEPERMOTOR_IDLE !=  g_displacementMotors[index].status)
        {
            g_displacementMotors[index].isRequestStop = TRUE;
            StepperMotor_RequestStop(&g_displacementMotors[index].stepperMotor);
            DisplacementSteperMotor_SendEvent(&g_displacementMotors[index], RESULT_FINISHED);
            TRACE_INFO("\n %s RequestStop", DisplacementSteperMotor_GetName(index));
            return DSCP_OK;
        }
        else
        {
            TRACE_ERROR("\n %s does not run, stop failure.", DisplacementSteperMotor_GetName(index));
            return DSCP_ERROR;
        }
    }
    else
    {
        TRACE_ERROR("\n Parameter error, the DisplacementMotor %d stop to fail. ", index);
        return ((Uint16) DSCP_ERROR_PARAM);
    }
}

void DisplacementSteperMotor_SendEvent(DisplacementSteperMotor *displacementMotor, MoveResult moveResult)
{
    if(TRUE == displacementMotor->isSendEvent)
    {
        Uint8 data[10] = {0};
        data[0] =  0;
        memcpy(data + sizeof(Uint8), &moveResult, sizeof(moveResult));
        DncpStack_SendEvent(DSCP_EVENT_MCI_MOTOR_RESULT, data , sizeof(Uint8) + sizeof(moveResult));
        DncpStack_BufferEvent(DSCP_EVENT_MCI_MOTOR_RESULT, data , sizeof(Uint8) + sizeof(moveResult));
    }
    displacementMotor->isSendEvent = FALSE;
}

void DisplacementSteperMotor_Stop(DisplacementSteperMotor *displacementMotor, MoveResult moveResult)
{
    if((RESULT_FINISHED != moveResult) && (RESULT_STOPPED != moveResult))
    {
        StepperMotor_DriverCheck(&displacementMotor->stepperMotor);
    }
    DisplacementSteperMotor_SendEvent(displacementMotor, moveResult);
    displacementMotor->status = STEPERMOTOR_IDLE;
    displacementMotor->isStatusSwitchStart = TRUE;
    displacementMotor->moveResult = moveResult;
    TRACE_INFO("\n%s:%d\n",__FILE__,__LINE__);
    TRACE_INFO("\n DisplacementMotor Stop. result = %d", (Uint8)moveResult);
}

void DisplacementMotor_MoveToTargetLocationHandle(void* obj)
{
    DisplacementSteperMotor *displacementMotor = (DisplacementSteperMotor *)obj;

    if (STEPERMOTOR_TO_TARGET_LOCATION == displacementMotor->status)//移动到目标位置
    {
        if (TRUE == displacementMotor->isStatusSwitchStart)
        {
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);
            TRACE_DEBUG("\n %s status: MOTOR_TO_TARGET_LOCATION", DisplacementSteperMotor_GetName(number));
            displacementMotor->isStatusSwitchStart = FALSE;
            StepperMotor_OtherMoveHandler limitHandle = NULL;//步进电机每走一步（未细分）的附加处理
            if (displacementMotor->dir == BACKWARD)//向零点运动则检查限位传感器
            {
                displacementMotor->returnStepCount = displacementMotor->currentSteps + LIMITSTEPMAX + 1;
                displacementMotor->isChangeSteps = TRUE;
                limitHandle = DisplacementMotor_MoveToSensorOffLimitHandle;//检查是否遮住传感器函数
            }
            //启动电机
            //TRACE_INFO("%s: %d\n",__FILE__,__LINE__);
            //TRACE_INFO("Steper start targetStep = %d\n", displacementMotor->targetStep);
            if  (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, displacementMotor->dir,
            		displacementMotor->targetStep, displacementMotor->isUseDefaultParam, limitHandle))
            {
                TRACE_ERROR("\n Because no idle timer , the %s fail.", DisplacementSteperMotor_GetName(number));
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                return;
            }
        }

        if (StepperMotor_IDLE == StepperMotor_GetStatus(&displacementMotor->stepperMotor))
        {
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);
            MoveResult moveResult;
            if (TRUE == displacementMotor->isRequestStop)
            {
                moveResult = RESULT_STOPPED;
            }
            else if (number == C_DISPLACEMENTMOTOR && FALSE == DisplacementMotor_CMoveCheckPosition(&g_displacementMotors[C_DISPLACEMENTMOTOR], g_displacementMotors[C_DISPLACEMENTMOTOR].dir))
            {
                if(TRUE == StepperMotor_ReadDiagnostic(&displacementMotor->stepperMotor))
                {
                    TRACE_ERROR("\n Stepper motor driver error");
                    moveResult = RESULT_DRIVER_ERROR;
                }
                else
                {
                    TRACE_ERROR("\n Because C DisplacementMotor position error, the X DisplacementMotor fail.");
                    moveResult = RESULT_FAILED;
                }
            }
            else
            {
                moveResult = RESULT_FINISHED;
            }

            Uint32 alreadyStep = StepperMotor_GetAlreadyStep(&displacementMotor->stepperMotor);
            if (displacementMotor->dir == BACKWARD)
            {
                displacementMotor->currentSteps -= alreadyStep;
                if (displacementMotor->currentSteps < 0)//在相对模式中，可能往后退的步数大于当前步数，为能复位回到零点允许此操作
                {
                    displacementMotor->currentSteps = 0;
                }
                TRACE_INFO("\n %s backward Step : %d new currentSteps: %d", DisplacementSteperMotor_GetName(number), alreadyStep, displacementMotor->currentSteps);
            }
            else
            {
                displacementMotor->currentSteps += alreadyStep;
                TRACE_INFO("\n %s forward Step : %d new currentSteps: %d", DisplacementSteperMotor_GetName(number), alreadyStep, displacementMotor->currentSteps);
            }
            DisplacementSteperMotor_Stop(displacementMotor, moveResult);
        }
    }
}

void DisplacementMotor_ResetHandle(void* obj)
{
    DisplacementSteperMotor *displacementMotor = (DisplacementSteperMotor *)obj;

    //使用复位速度
    if(displacementMotor->stepperMotor.number == C_DISPLACEMENTMOTOR)
    {																		 //复位参数
        StepperMotor_SetCurrentMotionParam(&displacementMotor->stepperMotor, s_resetCParam);
    }

    if (STEPERMOTOR_TO_SENSOR == displacementMotor->status)
    {
        if (TRUE == displacementMotor->isStatusSwitchStart)
        {
            displacementMotor->isStatusSwitchStart = FALSE;
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);
            TRACE_DEBUG("\n %s status: MOTOR_TO_SENSOR", DisplacementSteperMotor_GetName(number));
            // 回归预测步数
            displacementMotor->returnStepCount = displacementMotor->currentSteps + LIMITSTEPMAX + 1;
            if(displacementMotor->returnStepCount > displacementMotor->maxSteps + MAXOFFLIMITSTEP)
            {
                displacementMotor->returnStepCount = displacementMotor->maxSteps + MAXOFFLIMITSTEP;
            }
            displacementMotor->isChangeSteps = FALSE;
            TRACE_INFO("\nMotor %d returnStepCount = %d", number, displacementMotor->returnStepCount);
            ///if  (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, BACKWARD, displacementMotor->maxSteps + MAXOFFLIMITSTEP, FALSE, DisplacementMotor_MoveToSensorOffLimitHandle))
            //TRACE_INFO("file:%s -> line: %d\n",__FILE__,__LINE__);
            if  (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, BACKWARD,
            		displacementMotor->returnStepCount + MAXIMUMOFFLIMITSTEP, FALSE, DisplacementMotor_MoveToSensorOffLimitHandle))
            {
                TRACE_ERROR("\n Because no idle timer , the %s fail.", DisplacementSteperMotor_GetName(number));
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                return;
            }
        }
        if (StepperMotor_IDLE == StepperMotor_GetStatus(&displacementMotor->stepperMotor))
        {
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);

            //更新步数是为让复位过程被停止，步数还是相对正确的。
            Uint32 alreadyStep = StepperMotor_GetAlreadyStep(&displacementMotor->stepperMotor);
            displacementMotor->currentSteps -= alreadyStep;
            if (displacementMotor->currentSteps < 0)
            {
                displacementMotor->currentSteps = 0;
            }
            TRACE_INFO("\n %s backward Step : %d new currentSteps: %d", DisplacementSteperMotor_GetName(number), alreadyStep, displacementMotor->currentSteps);

            if (TRUE == displacementMotor->isRequestStop)
            {
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_STOPPED);
            }
            else
            {
                if (FALSE == DisplacementSteperMotor_IsSensorBlocked(number)) //未遮住传感器
                {
                    TRACE_ERROR("\n Because error no find sensor, the %s fail. Position sensor don't Blocked", DisplacementSteperMotor_GetName(number));
                    if(C_DISPLACEMENTMOTOR == number)
                    {
                        DisplacementSteperMotor_Stop(displacementMotor, RESULT_MOVE_IN_SENSOR_FAIL_C); //X轴找不到传感器
                    }
                    else
                    {
                        DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                    }
                }
                else
                {
                    displacementMotor->status = STEPERMOTOR_TO_ZERO;
                    displacementMotor->isStatusSwitchStart = TRUE;
                }
            }
        }
    }
    else if (STEPERMOTOR_TO_ZERO == displacementMotor->status)
    {
        if (TRUE == displacementMotor->isStatusSwitchStart)
        {
            displacementMotor->isStatusSwitchStart = FALSE;
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);
            TRACE_DEBUG("\n %s status: MOTOR_TO_ZERO", DisplacementSteperMotor_GetName(number));
            StepperMotor_OtherMoveHandler limitHandle = NULL;
            limitHandle = DisplacementMotor_CMoveToZerosOffLimitHandle;

            if (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, FORWARD,
            		displacementMotor->maxSteps + MAXIMUMOFFLIMITSTEP, FALSE, limitHandle))
            {
                TRACE_ERROR("\n Because no idle timer , the %s fail.", DisplacementSteperMotor_GetName(number));
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                return;
            }
        }
        if (StepperMotor_IDLE == StepperMotor_GetStatus(&displacementMotor->stepperMotor))
        {
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);

            Uint32 alreadyStep = StepperMotor_GetAlreadyStep(&displacementMotor->stepperMotor);
            displacementMotor->currentSteps += alreadyStep;

            MoveResult moveResult;
            if (TRUE == displacementMotor->isRequestStop)
            {
                moveResult = RESULT_STOPPED;
            }
            else
            {
                if (number == C_DISPLACEMENTMOTOR && TRUE == DisplacementSteperMotor_IsSensorBlocked(C_DISPLACEMENTMOTOR))
                {
                    TRACE_ERROR("\n Because error no find sensor, the X DisplacementMotor reset fail. Position sensor is Blocked");
                    moveResult = RESULT_MOVE_OUT_SENSOR_FAIL_C;

                }
                else if(TRUE == StepperMotor_ReadDiagnostic(&displacementMotor->stepperMotor))
                {
                    TRACE_ERROR("\n Stepper motor driver error");
                    moveResult = RESULT_DRIVER_ERROR;
                }
                else
                {
                    TRACE_INFO("\n %s DisplacementMotor reset finish.", DisplacementSteperMotor_GetName(number));
                    displacementMotor->currentSteps = 0;
                    moveResult = RESULT_FINISHED;
                }
            }
            TRACE_INFO("\n %s forward Step : %d new currentSteps: %d", DisplacementSteperMotor_GetName(number), alreadyStep, displacementMotor->currentSteps);
            DisplacementSteperMotor_Stop(displacementMotor, moveResult);
        }
    }
}


DisplacementMotorType GetDisplacementMotor_type()
{
    return s_displacementMotorType;
}



void DisplacementMotor_Check(void* obj)
{
    DisplacementSteperMotor *displacementMotor = (DisplacementSteperMotor *)obj;

    //使用复位速度
    if(displacementMotor->stepperMotor.number == C_DISPLACEMENTMOTOR)
    {																		 //复位参数
        StepperMotor_SetCurrentMotionParam(&displacementMotor->stepperMotor, s_resetCParam);
    }

    if (STEPERMOTOR_TO_SENSOR == displacementMotor->status)
    {
        if (TRUE == displacementMotor->isStatusSwitchStart)
        {
            displacementMotor->isStatusSwitchStart = FALSE;
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);
            TRACE_DEBUG("\n %s status: MOTOR_TO_SENSOR", DisplacementSteperMotor_GetName(number));
            					// 回归预测步数
            displacementMotor->returnStepCount = displacementMotor->currentSteps + LIMITSTEPMAX + 1;
            if(displacementMotor->returnStepCount > displacementMotor->maxSteps + MAXOFFLIMITSTEP)
            {
                displacementMotor->returnStepCount = displacementMotor->maxSteps + MAXOFFLIMITSTEP;
            }
            displacementMotor->isChangeSteps = FALSE;
            TRACE_INFO("\nMotor %d returnStepCount = %d", number, displacementMotor->returnStepCount);
            ///if  (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, BACKWARD, displacementMotor->maxSteps + MAXOFFLIMITSTEP, FALSE, DisplacementMotor_MoveToSensorOffLimitHandle))
            //TRACE_INFO("file:%s -> line: %d\n",__FILE__,__LINE__);
            if  (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, BACKWARD, displacementMotor->returnStepCount + MAXIMUMOFFLIMITSTEP, FALSE, DisplacementMotor_MoveToSensorOffLimitHandle))
            {
                TRACE_ERROR("\n Because no idle timer , the %s fail.", DisplacementSteperMotor_GetName(number));
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                s_displacementMotorType = DISPLACEMENTMOTOR_DC;
                return;
            }
        }
        if (StepperMotor_IDLE == StepperMotor_GetStatus(&displacementMotor->stepperMotor))
        {
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);

            //更新步数是为让复位过程被停止，步数还是相对正确的。
            Uint32 alreadyStep = StepperMotor_GetAlreadyStep(&displacementMotor->stepperMotor);
            displacementMotor->currentSteps -= alreadyStep;
            if (displacementMotor->currentSteps < 0)
            {
                displacementMotor->currentSteps = 0;
            }
            TRACE_INFO("\n %s backward Step : %d new currentSteps: %d", DisplacementSteperMotor_GetName(number), alreadyStep, displacementMotor->currentSteps);

            if (TRUE == displacementMotor->isRequestStop)
            {
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_STOPPED);
                s_displacementMotorType = DISPLACEMENTMOTOR_DC;
            }
            else
            {
                if (FALSE == DisplacementSteperMotor_IsSensorBlocked(number)) //未遮住传感器
                {
                    TRACE_ERROR("\n Because error no find sensor, the %s fail. Position sensor don't Blocked", DisplacementSteperMotor_GetName(number));
                    if(C_DISPLACEMENTMOTOR == number)
                    {
                        DisplacementSteperMotor_Stop(displacementMotor, RESULT_MOVE_IN_SENSOR_FAIL_C); //X轴找不到传感器
                        s_displacementMotorType = DISPLACEMENTMOTOR_DC;
                    }
                    else
                    {
                        DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                        s_displacementMotorType = DISPLACEMENTMOTOR_DC;
                    }
                }
                else
                {
                    displacementMotor->status = STEPERMOTOR_TO_ZERO;
                    displacementMotor->isStatusSwitchStart = TRUE;
                }
            }
        }
    }
    else if (STEPERMOTOR_TO_ZERO == displacementMotor->status)
    {
        if (TRUE == displacementMotor->isStatusSwitchStart)
        {
            displacementMotor->isStatusSwitchStart = FALSE;
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);
            TRACE_DEBUG("\n %s status: MOTOR_TO_ZERO", DisplacementSteperMotor_GetName(number));
            StepperMotor_OtherMoveHandler limitHandle = NULL;
            limitHandle = DisplacementMotor_CMoveToZerosOffLimitHandle;

            if (FALSE == StepperMotor_Start(&displacementMotor->stepperMotor, FORWARD, displacementMotor->maxSteps + MAXIMUMOFFLIMITSTEP, FALSE, limitHandle))
            {
                TRACE_ERROR("\n Because no idle timer , the %s fail.", DisplacementSteperMotor_GetName(number));
                DisplacementSteperMotor_Stop(displacementMotor, RESULT_FAILED);
                s_displacementMotorType = DISPLACEMENTMOTOR_DC;
                return;
            }
        }
        if (StepperMotor_IDLE == StepperMotor_GetStatus(&displacementMotor->stepperMotor))
        {
            Uint8 number = StepperMotor_GetNumber(&displacementMotor->stepperMotor);

            Uint32 alreadyStep = StepperMotor_GetAlreadyStep(&displacementMotor->stepperMotor);
            displacementMotor->currentSteps += alreadyStep;

            MoveResult moveResult;
            if (TRUE == displacementMotor->isRequestStop)
            {
                moveResult = RESULT_STOPPED;
            }
            else
            {
                if (number == C_DISPLACEMENTMOTOR && TRUE == DisplacementSteperMotor_IsSensorBlocked(C_DISPLACEMENTMOTOR))
                {
                    TRACE_ERROR("\n Because error no find sensor, the X DisplacementMotor reset fail. Position sensor is Blocked");
                    moveResult = RESULT_MOVE_OUT_SENSOR_FAIL_C;
                    s_displacementMotorType = DISPLACEMENTMOTOR_DC;
                }
                else if(TRUE == StepperMotor_ReadDiagnostic(&displacementMotor->stepperMotor))
                {
                    TRACE_ERROR("\n Stepper motor driver error");
                    moveResult = RESULT_DRIVER_ERROR;
                    s_displacementMotorType = DISPLACEMENTMOTOR_DC;
                }
                else
                {
                    TRACE_INFO("\n %s DisplacementMotor reset finish.", DisplacementSteperMotor_GetName(number));
                    displacementMotor->currentSteps = 0;
                    moveResult = RESULT_FINISHED;
                    s_displacementMotorType = DISPLACEMENTMOTOR_STEP;
                }
            }
            TRACE_INFO("\n %s forward Step : %d new currentSteps: %d", DisplacementSteperMotor_GetName(number), alreadyStep, displacementMotor->currentSteps);
            DisplacementSteperMotor_Stop(displacementMotor, moveResult);
            
        }
    }
}

Uint16 DisplacementSteperMotor_CheckType(Uint8 index)
{
	if (STEPERMOTOR_IDLE == g_displacementMotors[index].status)
	{
		TRACE_INFO("\n %s start reset", DisplacementSteperMotor_GetName(index));
		g_displacementMotors[index].isRequestStop = FALSE;
		g_displacementMotors[index].limitStepCount = 0;

		g_displacementMotors[index].moveHandler = DisplacementMotor_Check;//电机复位回调处理函数
		g_displacementMotors[index].status = STEPERMOTOR_TO_SENSOR;//电机到传感器
		g_displacementMotors[index].isStatusSwitchStart = TRUE;
		DncpStack_ClearBufferedEvent();
		vTaskResume(g_displacementMotors[index].xHandle);
		return DSCP_OK;
	}
	else
	{
		TRACE_ERROR("\n Because the DisplacementMotor is running, the %s CheckType to fail.", DisplacementSteperMotor_GetName(index));
		return DSCP_BUSY;
	}
}



