/*
 * DisplacementSteperMotorMap.h
 *
 *  Created on: 2023年03月02日
 *      Author: hgq
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_DISPLACEMENTSTEPERMOTORMAP_H_
#define SRC_DRIVER_LIQUIDDRIVER_DISPLACEMENTSTEPERMOTORMAP_H_
#include "stm32f4xx.h"
#include "Common/Types.h"
#include "PeristalticPump/DisplacementMotorManager.h"
#include "PeristalticPump/DisplacementSteperMotorManager.h"
#include "ConstantDCMotorDriver.h"


void StepperMotorMap_DisplacementMotorInit(DisplacementSteperMotor *displacementMotor);


#endif /* SRC_DRIVER_LIQUIDDRIVER_DISPLACEMENTMOTORMAP_H_ */
