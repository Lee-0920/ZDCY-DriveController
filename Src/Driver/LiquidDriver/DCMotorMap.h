/*
 * DCMotorMap.h
 *
 *  Created on: 2018年2月28日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_DCMOTORMAP_H_
#define SRC_DRIVER_LIQUIDDRIVER_DCMOTORMAP_H_

#include "PeristalticPump/DCMotorManager.h"
#include "PeristalticPump/DisplacementMotorManager.h"
#include "ConstantDCMotorDriver.h"

void DCMotorMap_Init(ConstantDCMotor *motor);

#endif /* SRC_DRIVER_LIQUIDDRIVER_DCMOTORMAP_H_ */
