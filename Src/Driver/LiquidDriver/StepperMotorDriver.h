/*
 * StepperMotorDriver.h
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORDRIVER_H_
#define SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORDRIVER_H_
#include "stm32f4xx.h"
#include "Common/Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    GPIO_TypeDef *portEnable;
    Uint16 pinEnable;
    uint32_t rccEnable;
    GPIO_TypeDef *portDir;
    Uint16 pinDir;
    uint32_t rccDir;
    GPIO_TypeDef *portClock;
    Uint16 pinClock;
    uint32_t rccClock;
    BitAction forwardLevel;
    GPIO_TypeDef *portDiag;
    Uint16 pinDiag;
    uint32_t rccDiag;
    GPIO_TypeDef *portReset;
    Uint16 pinReset;
    uint32_t rccReset;
} StepperMotorDriver;

typedef enum
{
    FORWARD, //向前
    BACKWARD, //向后
    MAX_DIRECTION
} Direction;

void StepperMotorDriver_Init(StepperMotorDriver *stepperMotorDriver);
void DisplaceStepperMotorDriver_Init(StepperMotorDriver *stepperMotorDriver);
void StepperMotorDriver_Enable(StepperMotorDriver *stepperMotorDriver);
void StepperMotorDriver_Disable(StepperMotorDriver *stepperMotorDriver);
void StepperMotorDriver_SetDirection(StepperMotorDriver *stepperMotorDriver, Direction dir);
void StepperMotorDriver_PullHigh(StepperMotorDriver *stepperMotorDriver);
void StepperMotorDriver_PullLow(StepperMotorDriver *stepperMotorDriver);
Bool StepperMotorDriver_ReadDiagnostic(StepperMotorDriver *stepperMotorDriver);
void StepperMotorDriver_SetForwardLevel(StepperMotorDriver *stepperMotorDriver, BitAction bitVal);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DRIVER_LIQUIDDRIVER_STEPPERMOTORDRIVER_H_ */
