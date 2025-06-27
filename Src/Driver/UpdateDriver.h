/*
 * UpdateDriver.h
 *
 *  Created on: 2016年7月19日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_UPDATEDRIVER_H_
#define SRC_DRIVER_UPDATEDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

Bool UpdateDriver_Erase(void);
Bool UpdateDriver_Write(u32 offsetAddr, u32 numToWrite, u8 *buffer);
void UpdateDriver_Read(u32 offsetAddr, u32 numToRead, u8 *buffer);
#ifdef _CS_APP
void UpdateDriver_JumpToUpdater(void);
#else
void UpdateDriver_JumpToApplication(void);
void UpdateDriver_Boot(void);
#endif
#endif /* SRC_DRIVER_UPDATEDRIVER_H_ */
