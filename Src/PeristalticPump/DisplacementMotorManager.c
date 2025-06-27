/*
 * DisplacementMotorManager.c
 *
 *  Created on: 2018年3月7日
 *      Author: LIANG
 */
#include "stm32f4xx.h"
#include "DNCP/App/DscpSysDefine.h"
#include "System.h"
#include "Driver/LiquidDriver/PositionSensorMap.h"
#include "Driver/LiquidDriver/StepperMotorMap.h"
#include "Driver/LiquidDriver/DisplacementMotorMap.h"
#include "StepperMotor.h"
#include "SystemConfig.h"
#include "Driver/McuFlash.h"
#include "DncpStack/DncpStack.h"
#include "DisplacementMotorManager.h"
#include <stdlib.h>
#include "LuipApi/MotorControlInterface.h"


static void DisplacementMotor_TaskHandle(void *pvParameters);
static void DisplacementMotor_SendEvent();
//位移电机
DisplacementMotor s_displacementMotor;
DisplacementMotorStatus s_motorStatus = MOTOR_IDLE;
static xTaskHandle s_displacementMotorHandle;
MoveResult s_moveResult;

//走0.1°需要的事件，精确到ms
static float MotorSpeed_MS  = 0;
//目标位置的度数
static float s_targetLocationDegrees  = 0;

static Uint32 MotoRunCount = 0;
static Uint32 SetMotoRunCount = 0;

#define TIM_MAX_TIMEOUT 70000   //电机单次最大运行时长

void Init_Timer2()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); ///使能 TIM2 时钟
	NVIC_InitTypeDef    NVIC_InitStructure;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 99; 					//1ms
	TIM_TimeBaseStructure.TIM_Prescaler =719; 				//10us=0.01ms
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);     //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	
	TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2, DISABLE);  //使能TIMx外设

}

void TIM2_IRQHandler(void)   //TIM2中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) //检查指定的TIM中断发生与否:TIM 中断源
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源
		MotoRunCount++;
		if(SetMotoRunCount && (MotoRunCount>=SetMotoRunCount))
		{
			DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
			SetMotoRunCount = 0;
		}
	}
    //TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //清除中断标志位
}

void Start_Count()
{
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设
}

void Stop_Count()
{
	TIM_Cmd(TIM2, DISABLE);  //使能TIMx外设
}

Uint32 Get_Count()
{
	return MotoRunCount;
}

void clear_Count()
{
	MotoRunCount = 0;
}

void SetMotoRunTime(Uint32 count)
{
    clear_Count();
	SetMotoRunCount = count;
    Start_Count();
    
}

void Set_Count(Uint32 count)
{
    MotoRunCount = count;
}

void DisplacementMotor_Init()
{
	memset(&s_displacementMotor,0,sizeof(s_displacementMotor));
	DisplacementMotorMap_Init(&s_displacementMotor);
	PositionSensorMap_DisplacementMotorInit(&s_displacementMotor);
	Init_Timer2();
    xTaskCreate(DisplacementMotor_TaskHandle, "DisplacementMotor", DISPLACEMENTMOTOR_STK_SIZE, NULL,
    		DISPLACEMENTMOTOR_TASK_PRIO, &s_displacementMotorHandle);
}

void DisplacementMotor_Restore(Uint8 index)
{
	DisplacementMotor_RequestStop(index);
}

Uint16 DisplacementMotor_Start(Uint8 index, float degree)
{
	Uint16 ret = DSCP_OK;
	if (s_motorStatus != MOTOR_IDLE)
	{
		ret = DSCP_BUSY;
	}
	else if (degree>=0 && degree<360)
	{
		s_targetLocationDegrees = degree;
		TRACE_INFO("DisplacementMotor Start ");
		TRACE_INFO("degree =  %f",s_targetLocationDegrees);
		s_motorStatus = MOTOR_WAIT_CALCULATE_SPEED;
        s_moveResult = RESULT_FINISHED;
		vTaskResume(s_displacementMotorHandle);
	}
	else
	{
		ret = DSCP_ERROR_PARAM;
		TRACE_INFO("\n param degree error ,[0,360)");
	}
	return ret;
}

Uint16 DisplacementMotor_Reset(Uint8 index)
{
	Uint16 ret = DSCP_OK;
	if (s_motorStatus != MOTOR_IDLE)
	{
		ret = DSCP_BUSY;
	}
	else
	{
		s_targetLocationDegrees = 0;
		TRACE_INFO("\n DisplacementMotor Reset Start ");
		TRACE_INFO("degree =  %f",s_targetLocationDegrees);
		s_motorStatus = MOTOR_WAIT_CALCULATE_SPEED;
    	s_moveResult = RESULT_FINISHED;
		vTaskResume(s_displacementMotorHandle);
	}
	return ret;
}

Uint16 DisplacementMotor_RequestStop(Uint8 index)
{
	Printf("\nDC Stop");
	Printf("\n");
	DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
    Stop_Count();
	s_moveResult = RESULT_STOPPED;
	s_motorStatus = MOTOR_IDLE;
	DisplacementMotor_SendEvent();
	return DSCP_OK;
}



StepperMotorParam DisplacementMotor_GetCurrentMotionParam(Uint8 index)
{
	StepperMotorParam ret;
	memset(&ret,0,sizeof(StepperMotorParam));
    return ret;
}

Uint16 DisplacementMotor_SetDefaultMotionParam(Uint8 index, StepperMotorParam param)
{
	return 0;
}

Uint16 DisplacementMotor_SetCurrentMotionParam(Uint8 index, StepperMotorParam param)
{
	return 0;
}

DisplacementMotorStatus DisplacementMotor_GetStatus(Uint8 index)
{
    return s_motorStatus;
}

float DisplacementMotor_GetMaxDegrees(Uint8 index)
{
    return 360;
}

float DisplacementMotor_GetCurrentDegrees(Uint8 index)
{
    return 0;
}

Bool DisplacementMotor_IsSensorBlocked(Uint8 index , Uint8 num)
{
	Bool ret = FALSE;
	if (num < DISPLACEMENTMOTOR_SENSOR)
	{
		ret = PositionSensor_ReadInputStatus(&s_displacementMotor.positionSensor[num]);
	}
	if (ret)
	{
		ret = FALSE;
	}
	else
	{
		ret = TRUE;
	}
    return ret;
}

Bool DisplacementMotor_SendEventOpen(Uint8 index)
{
	s_displacementMotor.isSendEvent = TRUE;
	return TRUE;
}


char* DisplacementMotor_GetName(Uint8 index)
{
    static char name[20] = "";
    memset(name, 0, sizeof(name));
    strcpy(name, "CDisplacementMotor");
    return name;
}

/*
 * Lift为1->2->3瓶的方向，Right为3->2->1瓶的方向
 * */
void DisplacementMotor_TaskHandle(void *pvParameters)
{
	static CalculateStatus calculateStatus = CALCULATE_IDLE;
	//static Uint32 timeCount;
	static float targetLocationTime  = 0;
    vTaskSuspend(NULL);
    while(1)
    {
        vTaskDelay(1 / portTICK_RATE_MS);
        switch(s_motorStatus)
        {
        case MOTOR_IDLE:
        	calculateStatus = CALCULATE_IDLE;
            s_moveResult = RESULT_FINISHED;
        	vTaskSuspend(NULL);
            break;
        case MOTOR_WAIT_CALCULATE_SPEED:
            switch(calculateStatus)
            {
            case CALCULATE_IDLE:
            	calculateStatus = CALCULATE_CURRENT_STATUS;
            	//先往正方向转一秒，防止已经越过第一个传感器
            	{
            		TRACE_INFO("\nCalculate start");
            		DisplacementMotorDriver_Lift(&s_displacementMotor.Motor);
            		vTaskDelay(800 / portTICK_RATE_MS);
            		DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
            	}
                break;
            case CALCULATE_CURRENT_STATUS:
            	//当前位置为0°
            	if (DisplacementMotor_IsSensorBlocked(0,0))
            	{
            		TRACE_INFO("\nCurrent is 0");
            		if (0 == s_targetLocationDegrees)
            		{
            			Printf("\n Reset Finish");
            			s_moveResult = RESULT_FINISHED;
            			s_motorStatus = MOTOR_FINISH;
            		}
            		else
            		{
                		calculateStatus = CALCULATE_MOVE_TO_180;
                		DisplacementMotorDriver_Lift(&s_displacementMotor.Motor);
            		}
            	}
            	//否则均为往零点走
            	else
            	{
            		Printf("\nCurrent is Unknowable");
            		calculateStatus = CALCULATE_MOVE_TO_0;
            		DisplacementMotorDriver_Right(&s_displacementMotor.Motor);
            	}
                clear_Count();
                break;
            case CALCULATE_MOVE_TO_0:
            	Start_Count();
            	//到达0°
            	while((!DisplacementMotor_IsSensorBlocked(0,0))&&(s_moveResult != RESULT_STOPPED))
                {
                    vTaskDelay(1 / portTICK_RATE_MS);
                    if(Get_Count() > TIM_MAX_TIMEOUT)
                	{
                		//60秒没有找到传感器则停止
                		Debug_Log("Move to 0 ERROR");
                		TRACE_ERROR("\n Move to 0 ERROR,No Sensor Found. Start Reset.");
                        s_moveResult = RESULT_FAILED;
                		s_motorStatus = MOTOR_FINISH;
                        DisplacementMotorDriver_Reset(&s_displacementMotor.Motor);
                        break;
                	}
                }
            	if (DisplacementMotor_IsSensorBlocked(0,0))
            	{
            	    DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
            		TRACE_INFO("\n Arrive 0");
                    Debug_Log("Arrive 0");
            		if (0 == s_targetLocationDegrees)
            		{
            			Printf("\n Reset Finish");
            			s_motorStatus = MOTOR_FINISH;
            		}
            		else
            		{
                		calculateStatus = CALCULATE_MOVE_TO_180;
            		}
            	}
                //DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
                Stop_Count();
                clear_Count();
                break;
            case CALCULATE_MOVE_TO_180:
                Start_Count();
                DisplacementMotorDriver_Lift(&s_displacementMotor.Motor);
            	//到达180°
            	while((!DisplacementMotor_IsSensorBlocked(0,1))&&(s_moveResult != RESULT_STOPPED))
                {   
                    vTaskDelay(1 / portTICK_RATE_MS);
                    if (Get_Count() > TIM_MAX_TIMEOUT/2)
                	{
                		//DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
                		//30秒没有找到传感器则停止  进行复位
                		Debug_Log("Move to 180 ERROR");
                		TRACE_ERROR("\n Move to 180 ERROR,No Sensor Found. Start Reset.");
                		clear_Count();
                		s_moveResult = RESULT_FAILED;
                        s_motorStatus = MOTOR_FINISH;
                		s_targetLocationDegrees = 0;
                		calculateStatus = CALCULATE_MOVE_TO_0;
                        DisplacementMotorDriver_Reset(&s_displacementMotor.Motor);
                        break;
                	}
                }
            	if (DisplacementMotor_IsSensorBlocked(0,1))
            	{
            		Stop_Count();
            		TRACE_INFO("\n Arrive 180");
                    Debug_Log("Arrive 180");
            		DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
            		calculateStatus = CALCULATE_SPEED;
            	}
                Stop_Count();
                break;
            case CALCULATE_SPEED:
                //计算速度
            	MotorSpeed_MS = Get_Count()/178.5;
                TRACE_INFO("\n count = %d ,speed = %f",Get_Count(),MotorSpeed_MS);
                if (s_targetLocationDegrees == 180)
                {
                    //目标为180度
                    s_motorStatus = MOTOR_FINISH;
                }
                else
                {
                    if (s_targetLocationDegrees > 180)
                    {
                        targetLocationTime = MotorSpeed_MS*(s_targetLocationDegrees+1 - 180);
                        TRACE_INFO("\n need Run time = %f Ms",targetLocationTime);
                        Debug_Log("need Run time = %f Ms",targetLocationTime);
                        s_motorStatus = MOTOR_TO_TARGET_LOCATION;
                    }
                    else
                    {
                        targetLocationTime = MotorSpeed_MS*s_targetLocationDegrees;
                        TRACE_INFO("\n need Run time = %f Ms",targetLocationTime);
                        Debug_Log("need Run time = %f Ms",targetLocationTime);
                        s_motorStatus = MOTOR_TO_START_POINT;
                        //向0°方向
                        DisplacementMotorDriver_Right(&s_displacementMotor.Motor);
                    }
                    //计数器清零
                    clear_Count();
                }
                break;
            }
            break;
        case MOTOR_TO_START_POINT:
            Start_Count();
        	//到达0°
        	while((!DisplacementMotor_IsSensorBlocked(0,0))&&(s_moveResult != RESULT_STOPPED))
            {
                vTaskDelay(1 / portTICK_RATE_MS);
                if(Get_Count() > TIM_MAX_TIMEOUT/2)
                {
                    //DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
                    Stop_Count();
                    s_moveResult = RESULT_FAILED;
                    s_motorStatus = MOTOR_FINISH;
                    s_targetLocationDegrees = 0;
                    Debug_Log("Back to 0 ERROR");
                    DisplacementMotorDriver_Reset(&s_displacementMotor.Motor);
                    break;
                }
            }   
        	if (DisplacementMotor_IsSensorBlocked(0,0))
        	{
        		s_motorStatus = MOTOR_TO_TARGET_LOCATION;
                DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
                Debug_Log("Start Point");
                Stop_Count();
        	}
            Stop_Count();
            break;
        case MOTOR_TO_TARGET_LOCATION:
        	if (s_targetLocationDegrees < 180)
        	{
        		TRACE_INFO("\n 0-180");
        	}
        	else
        	{
        		TRACE_INFO("\n 180-360");
        	}
        	s_motorStatus = MOTOR_WAIT_FINISH;
            break;
        case MOTOR_WAIT_FINISH:
        	DisplacementMotorDriver_Lift(&s_displacementMotor.Motor);
            SetMotoRunTime((Uint32)(targetLocationTime+0.5)); //四舍五入
            while((Get_Count()< targetLocationTime)&&(s_moveResult != RESULT_STOPPED))
                vTaskDelay(1 / portTICK_RATE_MS);
        	if (Get_Count() >= targetLocationTime)
        	{
        		
        		TRACE_INFO("\n timeCount %d targetLocationTime %f",Get_Count(),targetLocationTime);
        		s_moveResult = RESULT_FINISHED;
            	s_motorStatus = MOTOR_FINISH;
        	}
            break;
        case MOTOR_FINISH:
            Stop_Count();
            clear_Count();
    		DisplacementMotorDriver_Stop(&s_displacementMotor.Motor);
    		TRACE_INFO("\n dc motor finish stop");
    		Debug_Log("dc motor finish stop");
        	DisplacementMotor_SendEvent();
        	s_motorStatus = MOTOR_IDLE;
            break;
        }
    }
}

void DisplacementMotor_SendEvent()
{
    if(TRUE == s_displacementMotor.isSendEvent)
    {
        Uint8 data[10] = {0};
        data[0] =  0;
        memcpy(data + sizeof(Uint8), &s_moveResult, sizeof(s_moveResult));
        DncpStack_SendEvent(DSCP_EVENT_MCI_MOTOR_RESULT, data , sizeof(Uint8) + sizeof(s_moveResult));
        DncpStack_BufferEvent(DSCP_EVENT_MCI_MOTOR_RESULT, data , sizeof(Uint8) + sizeof(s_moveResult));
    }
    s_displacementMotor.isSendEvent = FALSE;
}

