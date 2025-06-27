/*
 * TMC2160.h
 *
 *  Created on: 2020年5月14日
 *      Author: Administrator
 */

#ifndef SRC_PERISTALTICPUMP_TMC2160_H_
#define SRC_PERISTALTICPUMP_TMC2160_H_

#include "stm32f4xx.h"
#include "TMCConfig.h"
#include "Common/Types.h"
#include "PeristalticPump/StepperMotor.h"

Uint8 TMC2160_DriverCheck(StepperMotor* motor);
DriverError TMC2160_ReadDriveError(Uint8 slaveAddr);
Bool TMC2160_WriteData(Uint8 slaveAddr,Uint8 regAddr,Uint32 data);
Bool TMC2160_ReadData(Uint8 slaveAddr,Uint8 regAddr,Uint32 *data);
Uint32 TMC2160_ReadSubdivision(Uint8 slaveAddr);
Bool TMC2160_WriteSubdivision(Uint8 slaveAddr, Uint32 subdivision);
Uint32 TMC2160_CurrentSet(Uint8 slaveAddr, Uint8 ihold, Uint8 irun, Uint8 delay);
Bool TMC2160Config_MotorDriverInit(StepperMotor* motor);
void TMC2160Config_RegList(Uint8 slaveAddr);

#endif /* SRC_PERISTALTICPUMP_TMC2160_H_ */
