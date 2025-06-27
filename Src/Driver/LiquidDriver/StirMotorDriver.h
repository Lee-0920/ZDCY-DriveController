/********** Copyright (c) 2022-2022 SZLABSUN.Co.Ltd. All rights reserved.**********
* File Name          : StirMotorDriver.h
* Author             : hyz
* Date               : 06/28/2022
* Description        : This file provides all the StirMotorDriver functions.
*******************************************************************************/

#ifndef SRC_DRIVER_LIQUIDDRIVER_STIRMOTORDRIVER_H_
#define SRC_DRIVER_LIQUIDDRIVER_STIRMOTORDRIVER_H_

#include "tracer/trace.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdint.h"
#include "Common/Types.h"
#include "stm32f4xx.h"

#define STIRMOTOR_MAX_NUM		2

typedef struct
{
    GPIO_TypeDef *port;
    Uint16 pin;
    uint32_t rcc;
    float workLevel;
} StirMotorDriver;

void StirMotorDriver_Init();
Bool StirMotorDriver_SetLevel(Uint8 index ,float level);

#endif /* SRC_DRIVER_LIQUIDDRIVER_STIRMOTORDRIVER_H_ */
