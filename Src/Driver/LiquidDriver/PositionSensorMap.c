/*
 * PositionSensorMap.c
 *
 *  Created on: 2018年3月6日
 *      Author: LIANG
 */
#include "PositionSensor.h"
#include "PositionSensorMap.h"



void PositionSensorMap_DisplacementMotorInit(DisplacementMotor *displacementMotor)
{
    displacementMotor->positionSensor[0].pin = GPIO_Pin_5;
    displacementMotor->positionSensor[0].port = GPIOA;
    displacementMotor->positionSensor[0].rcc = RCC_AHB1Periph_GPIOA;
    PositionSensor_Init(&displacementMotor->positionSensor[0]);

    displacementMotor->positionSensor[1].pin = GPIO_Pin_6;
    displacementMotor->positionSensor[1].port = GPIOA;
    displacementMotor->positionSensor[1].rcc = RCC_AHB1Periph_GPIOA;
    PositionSensor_Init(&displacementMotor->positionSensor[1]);
}

void PositionSensorMap_DisplacementSteperMotorInit(DisplacementSteperMotor *displacementMotor)
{
    displacementMotor->positionSensor.pin = GPIO_Pin_5;
    displacementMotor->positionSensor.port = GPIOA;
    displacementMotor->positionSensor.rcc = RCC_AHB1Periph_GPIOA;
    PositionSensor_Init(&displacementMotor->positionSensor);
}

void PositionSensorMap_WaterCheckSensorInit(PositionSensor *positionSensor)
{
    positionSensor[0].pin = GPIO_Pin_11;
    positionSensor[0].port = GPIOE;
    positionSensor[0].rcc = RCC_AHB1Periph_GPIOE;
    PositionSensor_Init(&positionSensor[0]);

    positionSensor[1].pin = GPIO_Pin_12;
    positionSensor[1].port = GPIOE;
    positionSensor[1].rcc = RCC_AHB1Periph_GPIOE;
    PositionSensor_Init(&positionSensor[1]);
}

