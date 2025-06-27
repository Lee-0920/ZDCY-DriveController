/*
 * ValveDriver.h
 *
 *  Created on: 2016年6月7日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_VALVEDRIVER_H_
#define SRC_DRIVER_VALVEDRIVER_H_

#include "Common/Types.h"
#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    GPIO_TypeDef *port;
    Uint16 pin;
    Uint32 rcc;
}Valve;

typedef enum
{
    VAlVE_CLOSE,
    VAlVE_OPEN
}SolenoidValueStatus;

void ValveDriver_Init(Valve *valve);
void ValveDriver_Control(Valve *valve, SolenoidValueStatus status);
SolenoidValueStatus ValveDriver_ReadStatus(Valve *valve);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DRIVER_VALVEDRIVER_H_ */
