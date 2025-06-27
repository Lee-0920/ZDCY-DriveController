/*
 * PositionSensorMap.h
 *
 *  Created on: 2018年3月6日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_POSITIONSENSORMAP_H_
#define SRC_DRIVER_LIQUIDDRIVER_POSITIONSENSORMAP_H_

#include "PeristalticPump/DisplacementMotorManager.h"
#include "PeristalticPump/DisplacementSteperMotorManager.h"

void PositionSensorMap_DisplacementMotorInit(DisplacementMotor *displacementMotor);
void PositionSensorMap_DisplacementSteperMotorInit(DisplacementSteperMotor *displacementMotor);
void PositionSensorMap_WaterCheckSensorInit(PositionSensor *positionSensor);

#endif /* SRC_DRIVER_LIQUIDDRIVER_POSITIONSENSORMAP_H_ */
