/*
 * AnalogDriver.h
 *
 *  Created on: 2020年5月22日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_ANALOGDRIVER_ANALOGDRIVER_H_
#define SRC_DRIVER_ANALOGDRIVER_ANALOGDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

void AnalogSignal_ADC1_Init(void);
Uint16 Get_ADC1(Uint8 ch);

#endif /* SRC_DRIVER_ANALOGDRIVER_ANALOGDRIVER_H_ */
