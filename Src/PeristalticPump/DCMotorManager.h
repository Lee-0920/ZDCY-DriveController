/*
 * DCMotorManager.h
 *
 *  Created on: 2018年3月9日
 *      Author: LIANG
 */

#ifndef SRC_PERISTALTICPUMP_DCMOTORMANAGER_H_
#define SRC_PERISTALTICPUMP_DCMOTORMANAGER_H_

#include "LiquidDriver/DCMotorDriver.h"
#include "FreeRTOS.h"
#include "timers.h"
#include <LiquidDriver/ConstantDCMotorDriver.h>

#define DCMOTOR_INDEX 1

typedef enum
{
    DCMOTORDEVICE_IDLE   = 0,
	DCMOTORDEVICE_BUSY   = 1,
}DCMotorDeviceStatus;

typedef enum
{
	DCMOTORHANDLE_IDLE,
	DCMOTORHANDLE_START,
	DCMOTORHANDLE_BUSY,
	DCMOTORHANDLE_FINISH,
}DCMotorHandleStatus;


void DCMotorManager_Init();
void DCMotorManager_Restore();
DCMotorDeviceStatus DCMotorManager_GetState(Uint8 index);
Uint16 DCMotorManager_Start(Uint8 index, Uint8 dir, float time);
Bool DCMotorManager_SendEventOpen(Uint8 index);
Uint16 DCMotorManager_Stop(Uint8 index);

#endif /* SRC_PERISTALTICPUMP_DCMOTORMANAGER_H_ */
