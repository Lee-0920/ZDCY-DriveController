/*
 * StepperMotorMap.h
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORMAP_H_
#define SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORMAP_H_

#include "PeristalticPump/PeristalticPumpManager.h"
#include "PeristalticPump/DisplacementMotorManager.h"

#ifdef __cplusplus
extern "C"
{
#endif

void StepperMotorMap_PeristalticPumpInit(PeristalticPump *peristalticPump);
//void StepperMotorMap_DisplacementMotorInit(DisplacementMotor *displacementMotor);

#ifdef __cplusplus
}
#endif
#endif /* SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORMAP_H_ */
