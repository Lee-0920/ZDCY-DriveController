/*
#include <LiquidDriver/ConstantDCMotorDriver.h>
 * DisplacementMotorMap.c
 *
 *  Created on: 2022年06月08日
 *      Author: hyz
 */

#include "DisplacementMotorMap.h"

void DisplacementMotorMap_Init(DisplacementMotor *motor)
{
    motor->Motor.pinL 	= GPIO_Pin_11;
    motor->Motor.portL 	= GPIOD;
    motor->Motor.rccL 	= RCC_AHB1Periph_GPIOD;
    motor->Motor.pinR 	= GPIO_Pin_12;
    motor->Motor.portR 	= GPIOD;
    motor->Motor.rccR 	= RCC_AHB1Periph_GPIOD;
    DisplacementMotorDriver_Init(&motor->Motor);
}
