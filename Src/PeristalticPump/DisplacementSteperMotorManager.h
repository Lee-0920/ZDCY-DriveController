/*
 * DisplacementMotorManager.h
 *
 *  Created on: 2018年3月7日
 *      Author: LIANG
 */

#ifndef SRC_PERISTALTICPUMP_DISPLACEMENTSTEPERMOTORMANAGER_H_
#define SRC_PERISTALTICPUMP_DISPLACEMENTSTEPERMOTORMANAGER_H_

#include "Common/Types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "StepperMotor.h"
#include "Driver/LiquidDriver/PositionSensor.h"

#define DISPLACEMENTMOTOR_TOTAL_PUMP 1
#define C_DISPLACEMENTMOTOR    0

#define STEP_OF_CIRCLE 200*26.85     //步进电机200步每圈，减速比27:1
#define DEGREES_TO_STEP(degrees) (Uint32)(((degrees)*(STEP_OF_CIRCLE))/360.0)
#define STEP_TO_DEGREES(step) (float)(((step)*360.0)/(STEP_OF_CIRCLE))

typedef enum
{
    DISPLACEMENTMOTOR_DC,   //直流电机
    DISPLACEMENTMOTOR_STEP, //步进电机
}DisplacementMotorType;


typedef enum
{
    MOTOR_MOVE_ABSOLUTE_MODE,//绝对模式
    MOTOR_MOVE_RELATIVE_MODE,//相对模式
    MOTOR_MOVE_SAFE_MODE,	 //水质采样器无安全模式
    MAX_MOTOR_MOVE_MODE,
}DisplacementMotorMode;

typedef enum
{
    STEPERMOTOR_IDLE,					//空闲
    STEPERMOTOR_TO_WAIT_Z_RESET,		//等待z轴电机复位
    STEPERMOTOR_TO_TARGET_LOCATION,
    STEPERMOTOR_TO_SENSOR,
    STEPERMOTOR_TO_ZERO,
}DisplacementSteperMotorStatus;

typedef void (*DisplacementMotor_MoveHandler)(void* obj);

typedef struct
{
    StepperMotor stepperMotor;
    PositionSensor positionSensor;
    //PositionSensor limitSensor;
    Int32 currentSteps;//当前位置
    Uint32 maxSteps;//单次启动的最大运行步数
    __IO Bool isRequestStop;
    Uint32 limitStepCount;      // 限制步数计数
    Int32 returnStepCount;     // 回归预测步数
    Bool isChangeSteps;
    Bool isSendEvent;
    Bool isStatusSwitchStart;
    DisplacementSteperMotorStatus status;
    DisplacementMotorMode mode;
    Direction dir;
    Int32 targetStep;
    Bool isUseDefaultParam;
    xTaskHandle xHandle;
    DisplacementMotor_MoveHandler moveHandler;
    MoveResult moveResult;
}DisplacementSteperMotor;

void DisplacementSteperMotor_Init();
void DisplacementSteperMotor_Restore();
StepperMotorParam DisplacementSteperMotor_GetDefaultMotionParam(Uint8 index);
StepperMotorParam DisplacementSteperMotor_GetCurrentMotionParam(Uint8 index);
Uint16 DisplacementSteperMotor_SetDefaultMotionParam(Uint8 index, StepperMotorParam param);
Uint16 DisplacementSteperMotor_SetCurrentMotionParam(Uint8 index, StepperMotorParam param);
DisplacementSteperMotorStatus DisplacementSteperMotor_GetStatus(Uint8 index);
Uint32 DisplacementMotor_GetMaxSteps(Uint8 index);
Uint32 DisplacementMotor_GetCurrentSteps(Uint8 index);
//Bool DisplacementMotor_IsSensorBlocked(Uint8 index);
Bool DisplacementSteperMotor_SendEventOpen(Uint8 index);
MoveResult DisplacementSteperMotor_GetMoveResult(Uint8 index);
char* DisplacementSteperMotor_GetName(Uint8 index);
Uint16 DisplacementSteperMotor_Start(Uint8 index, Int16 step, DisplacementMotorMode mode, Bool isUseDefaultParam);
Uint16 DisplacementSteperMotor_Reset(Uint8 index);
Uint16 DisplacementSteperMotor_RequestStop(Uint8 index);
StepperMotor* DisplacementMotor_GetStepperMotorC(void);
DisplacementMotorType GetDisplacementMotor_type();
Uint16 DisplacementSteperMotor_CheckType(Uint8 index);


#endif /* SRC_PERISTALTICPUMP_DISPLACEMENTMOTORMANAGER_H_ */
