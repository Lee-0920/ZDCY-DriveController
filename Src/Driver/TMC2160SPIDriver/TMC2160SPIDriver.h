/*
 * TMC2160SPIDriver.h
 *
 *  Created on: 2020年6月5日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_TMC2160SPIDRIVER_TMC2160SPIDRIVER_H_
#define SRC_DRIVER_TMC2160SPIDRIVER_TMC2160SPIDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

void TMC2160_SPIInit(void);
void TMC2160_1_sendData(unsigned long address, unsigned long datagram);
void TMC2160_2_sendData(unsigned long address, unsigned long datagram);
unsigned long TMC2160_1_ReadData(unsigned long address);
unsigned long TMC2160_2_ReadData(unsigned long address);



#endif /* SRC_DRIVER_TMC2160SPIDRIVER_TMC2160SPIDRIVER_H_ */
