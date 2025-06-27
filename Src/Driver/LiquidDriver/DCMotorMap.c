/*
 * DCMotorMap.c
 *
 *  Created on: 2018年2月28日
 *      Author: LIANG
 */

#include "DCMotorMap.h"
#include "Tracer/Trace.h"

void DCMotorMap_Init(ConstantDCMotor *motor)
{
    motor->pinL 	= GPIO_Pin_9;
    motor->portL 	= GPIOD;
    motor->rccL 	= RCC_AHB1Periph_GPIOD;
    motor->pinR 	= GPIO_Pin_10;
    motor->portR 	= GPIOD;
    motor->rccR 	= RCC_AHB1Periph_GPIOD;
    DisplacementMotorDriver_Init(motor);
}


