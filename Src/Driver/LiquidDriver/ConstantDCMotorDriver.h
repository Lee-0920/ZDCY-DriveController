/*
 * ConstantDCMotorDriver.h
 *
 *  Created on: 2022年06月08日
 *      Author: hyz
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_CONSTANTDCMOTORDRIVER_H_
#define SRC_DRIVER_LIQUIDDRIVER_CONSTANTDCMOTORDRIVER_H_
#include "stm32f4xx.h"
#include "Common/Types.h"
#include "PeristalticPump/DisplacementMotorManager.h"

void DisplacementMotorDriver_Init(ConstantDCMotor* Motor);
void DisplacementMotorDriver_Lift(ConstantDCMotor* Motor);
void DisplacementMotorDriver_Right(ConstantDCMotor* Motor);
void DisplacementMotorDriver_Stop(ConstantDCMotor* Motor);
void DisplacementMotorDriver_Reset(ConstantDCMotor* Motor);


#endif /* SRC_DRIVER_LIQUIDDRIVER_CONSTANTDCMOTORDRIVER_H_ */
