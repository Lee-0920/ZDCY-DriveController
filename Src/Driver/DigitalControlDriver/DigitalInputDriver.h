/*
 * DigitalInputDriver.h
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_DIGITALCONTROLDRIVER_DIGITALINPUTDRIVER_H_
#define SRC_DRIVER_DIGITALCONTROLDRIVER_DIGITALINPUTDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

#define DIGITAL_IN_NUM  4

void DigitalInput_Init(void);
Bool DigitalInput_Read(Uint8 index);
Uint8 DigitalInput_TotalNumber(void);
Uint32 DigitalInput_GetMap(void);

#endif /* SRC_DRIVER_DIGITALCONTROLDRIVER_DIGITALINPUTDRIVER_H_ */
