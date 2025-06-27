/*
 * DoorLock.h
 *
 *  Created on: 2020年5月18日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_DOORLOCK_H_
#define SRC_DRIVER_LIQUIDDRIVER_DOORLOCK_H_

#include "Common/Types.h"
#include "stm32f4xx.h"

typedef enum
{
    DOOR_CLOSE,
    DOOR_OPEN
}DoorStatus;

void ElectronicLockControl_Init(void);
void DoorDetection_Init(void);
Bool ElectronicLock_Control(Bool status);
Bool ElectronicLock_ReadStatus(void);
DoorStatus DoorDetection_ReadStatus(void);

#endif /* SRC_DRIVER_LIQUIDDRIVER_DOORLOCK_H_ */
