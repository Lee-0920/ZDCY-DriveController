/*
 * DisplacementMotorManager.h
 *
 *  Created on: 2018年3月7日
 *      Author: LIANG
 */

#ifndef SRC_PERISTALTICPUMP_DISPLACEMENTMOTORMANAGER_H_
#define SRC_PERISTALTICPUMP_DISPLACEMENTMOTORMANAGER_H_

#include "Common/Types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "StepperMotor.h"
#include "Driver/LiquidDriver/PositionSensor.h"

#define DISPLACEMENTMOTOR_TOTAL_PUMP 	1
#define DISPLACEMENTMOTOR_SENSOR 		2
/*
typedef enum
{
    DISPLACEMENTMOTOR_DC,   //直流电机
    DISPLACEMENTMOTOR_STEP, //步进电机
}DisplacementMotorType;
*/


typedef enum
{
    MOTOR_IDLE,						//空闲
    MOTOR_WAIT_CALCULATE_SPEED,		//等待速度计算完成   即走1°需要多久
	MOTOR_TO_START_POINT,			//前往起点(0~179度需要回到起点)
    MOTOR_TO_TARGET_LOCATION,		//前往目标位置
    MOTOR_WAIT_FINISH,				//等待运动完成
	MOTOR_FINISH,					//结束，处理结果
	MOTOR_SENSOR_ERR,               //错误,未找到传感器
}DisplacementMotorStatus;

typedef enum
{
	CALCULATE_IDLE,						//空闲
	CALCULATE_CURRENT_STATUS,			//当前位置
	CALCULATE_MOVE_TO_0,				//移动至0度
	CALCULATE_MOVE_TO_180,				//移动至180度
	CALCULATE_SPEED,					//计算速度
}CalculateStatus;

typedef struct
{
    GPIO_TypeDef *portL;
    Uint16 pinL;
    uint32_t rccL;
    GPIO_TypeDef *portR;
    Uint16 pinR;
    uint32_t rccR;
}ConstantDCMotor;

typedef struct
{
	ConstantDCMotor Motor;
    PositionSensor positionSensor[2];
    Bool isSendEvent;
}DisplacementMotor;


void DisplacementMotor_Init();
void DisplacementMotor_Restore(Uint8 index);
DisplacementMotorStatus DisplacementMotor_GetStatus(Uint8 index);
float DisplacementMotor_GetMaxDegrees(Uint8 index);
float DisplacementMotor_GetCurrentDegrees(Uint8 index);
Bool DisplacementMotor_IsSensorBlocked(Uint8 index,Uint8 num);
Bool DisplacementMotor_SendEventOpen(Uint8 index);
char* DisplacementMotor_GetName(Uint8 index);
Uint16 DisplacementMotor_Start(Uint8 index, float degrees);
Uint16 DisplacementMotor_Reset(Uint8 index);
Uint16 DisplacementMotor_RequestStop(Uint8 index);
//DisplacementMotorType GetDisplacementMotor_type();
//StepperMotor* DisplacementMotor_GetStepperMotorC(void);

#endif /* SRC_PERISTALTICPUMP_DISPLACEMENTMOTORMANAGER_H_ */
