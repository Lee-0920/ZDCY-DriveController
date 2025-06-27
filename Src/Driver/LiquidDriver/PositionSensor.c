
#include "Driver/LiquidDriver/PositionSensor.h"
#include "stm32f4xx.h"

void PositionSensor_Init(PositionSensor *positionSensor)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(positionSensor->rcc , ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Pin = positionSensor->pin;
    GPIO_Init(positionSensor->port, &GPIO_InitStructure);
}

SensorStatus PositionSensor_ReadInputStatus(PositionSensor *positionSensor)
{
    if (GPIO_ReadInputDataBit(positionSensor->port, positionSensor->pin))
    {
        return SENSOR_HIGH_LEVEL;
    }
    else
    {
        return SENSOR_LOW_LEVEL;
    }
}
