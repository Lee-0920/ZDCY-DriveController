/*
 * DisplacementMotorMap.h
 *
 *  Created on: 2022年06月08日
 *      Author: hyz
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_DISPLACEMENTMOTORMAP_H_
#define SRC_DRIVER_LIQUIDDRIVER_DISPLACEMENTMOTORMAP_H_
#include "stm32f4xx.h"
#include "Common/Types.h"
#include "PeristalticPump/DisplacementMotorManager.h"
#include "ConstantDCMotorDriver.h"

void DisplacementMotorMap_Init(DisplacementMotor *motor);

#endif /* SRC_DRIVER_LIQUIDDRIVER_DISPLACEMENTMOTORMAP_H_ */
