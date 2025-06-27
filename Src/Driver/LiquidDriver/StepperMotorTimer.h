/*
 * StepperMotorTimer.h
 *
 *  Created on: 2016年5月31日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORTIMER_H_
#define SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORTIMER_H_
#include "stm32f4xx.h"
#include "Common/Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*TimerHandle)(void *obj);

void StepperMotorTimer_Init(void);
void StepperMotorTimer_Start(void* obj);
void StepperMotorTimer_Stop(void* obj);
Bool StepperMotorTimer_CancelRegisterHandle(void* obj);
Bool StepperMotorTimer_RegisterHandle(TimerHandle Handle, void* obj);
void StepperMotorTimer_SpeedSetting(void* obj, float speed);

#ifdef __cplusplus
}
#endif
#endif /* SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORTIMER_H_ */
