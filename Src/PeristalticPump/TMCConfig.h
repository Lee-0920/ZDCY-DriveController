/*
 * TMCConfig.h
 *
 *  Created on: 2020年1月7日
 *      Author: Administrator
 */

#ifndef SRC_PERISTALTICPUMP_TMCCONFIG_H_
#define SRC_PERISTALTICPUMP_TMCCONFIG_H_

#include "stm32f4xx.h"
#include "Common/Types.h"
#include "PeristalticPump/StepperMotor.h"

typedef enum
{
    NoError = 0,
    ShortCircuit = 1,                 //短路
    OverTemperature = 2,        //过热
    UnderVoltage = 3,               //欠压
    StallDetection = 4,              //堵转
}DriverError;

typedef struct
{
    Uint8 destAddr;
    Uint8 regAddr;
    Uint32 data;
}TMC2160_Data;

typedef enum
{
	TMC2209,
	TMC2160
}TMCDriverType;

typedef struct
{
	TMCDriverType type;

}TMCConfiger;

void TMCConfig_Init(void);
void TMCConfig_Reinit(void);
Uint8 TMCConfig_DriverCheck(StepperMotor* motor);
DriverError TMCConfig_ReadDriveError(Uint8 slaveAddr);
Bool TMCConfig_EnableMicrostepReg(Uint8 slaveAddr);
Uint32 TMCConfig_ReadSubdivision(Uint8 slaveAddr);
Bool TMCConfig_WriteSubdivision(Uint8 slaveAddr, Uint32 subdivision);
Uint32 TMCConfig_CurrentSet(Uint8 slaveAddr, Uint8 ihold, Uint8 irun, Uint8 delay);
Bool TMCConfig_ReadData(Uint8 slaveAddr, Uint8 regAddr, Uint32* data);
Bool TMCConfig_WriteData(Uint8 slaveAddr, Uint8 regAddr, Uint32 data);
void TMCConfig_RegList(Uint8 slaveAddr);
char* TMCConfig_DriverName(Uint8 index);

#endif /* SRC_PERISTALTICPUMP_TMCCONFIG_H_ */
