/*
 * DigitalOutputDriver.h
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_DIGITALCONTROLDRIVER_DIGITALOUTPUTDRIVER_H_
#define SRC_DRIVER_DIGITALCONTROLDRIVER_DIGITALOUTPUTDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

typedef struct
{
    GPIO_TypeDef *port;
    Uint16 pin;
    uint32_t rcc;
} DigitalIO;

#define DIGITAL_OUT_NUM  1

void DigitalOutput_Init(void);
void DigitalOutput_TurnOnAll(void);
void DigitalOutput_TurnOffAll(void);
void DigitalOutput_TurnOn(int index);
void DigitalOutput_TurnOff(int index);
Bool DigitalOutput_IsOpen(int index);
Uint8 DigitalOutput_TotalNumber(void);
void DigitalOutput_SetMap(Uint32 map);
Uint32 DigitalOutput_GetMap(void);

#endif /* SRC_DRIVER_DIGITALCONTROLDRIVER_DIGITALOUTPUTDRIVER_H_ */
