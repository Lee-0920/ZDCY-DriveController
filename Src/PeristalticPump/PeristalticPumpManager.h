/*
 * PeristalticPumpManager.h
 *
 *  Created on: 2016年5月31日
 *      Author: Administrator
 */

#ifndef SRC_PERISTALTICPUMP_PERISTALTICPUMPMANAGER_H_
#define SRC_PERISTALTICPUMP_PERISTALTICPUMPMANAGER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "StepperMotor.h"
#include "Common/Types.h"
#include "string.h"
#include "tracer/trace.h"
#include "LuipApi/PeristalticPumpInterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PERISTALTICPUMPMANAGER_TOTAL_PUMP 2

#define PUMP_STEP_MOTOR_OFFSET   3  //蠕动泵编号 + 3  = 步进电机编号

/**
 * @brief 定标状态。
 */
typedef enum
{
	CHECKPOINT_IDLE,                //定量空闲
	CHECKPOINT_START,               //定量开始
	HIGHLEVEL_STATUS,				//高电平状态
	LOWLEVEL_STATUS,				//低电平状态
	CHECKPOINT_FINISH,				//定量完成
} CheckPoint;

typedef struct
{
    StepperMotor stepperMotor;
    float factor;
    Bool isSendEvent;
    CheckPointStatus targetStatus;		//目标检测点状态
    CheckPointStatus currentCheckPointStatus;//当前检测点状态
    float maxVolume;					//定量过程最大体积
    CheckPoint checkPoint;
    xTaskHandle xHandle;
    Bool waitPumpStop;
}PeristalticPump;

void PeristalticPumpManager_Init(void);
void PeristalticPumpManager_Restore(void);
Uint16 PeristalticPumpManager_GetTotalPumps(void);
StepperMotorParam PeristalticPumpManager_GetDefaultMotionParam(Uint8 index);
StepperMotorParam PeristalticPumpManager_GetCurrentMotionParam(Uint8 index);
Uint16 PeristalticPumpManager_SetDefaultMotionParam(Uint8 index, StepperMotorParam param);
Uint16 PeristalticPumpManager_SetCurrentMotionParam(Uint8 index, StepperMotorParam param);
float PeristalticPumpManager_GetFactor(Uint8 index);
Uint16 PeristalticPumpManager_SetFactor(Uint8 index, float factor);
StepperMotorStatus PeristalticPumpManager_GetStatus(Uint8 index);
float PeristalticPumpManager_GetVolume(Uint8 index);
Uint16 PeristalticPumpManager_Start(Uint8 index, Direction dir, float volume, Bool isUseDefaultParam);
Uint16 PeristalticPumpManager_StartToPoint(Uint8 index, Direction dir, CheckPointStatus TargetStatus, float volume, Bool isUseDefaultParam);
Uint16 PeristalticPumpManager_Stop(Uint8 index);
void PeristalticPumpManager_ChangeVolume(Uint8 index, float volume);
Bool PeristalticPumpManager_SendEventOpen(Uint8 index);
StepperMotor* PeristalticPumpManager_GetStepperMotor(Uint8 index);
Bool WaterCheckSensorBlocked(Uint8 index);


#ifdef __cplusplus
}
#endif

#endif /* SRC_PERISTALTICPUMP_PERISTALTICPUMPMANAGER_H_ */
