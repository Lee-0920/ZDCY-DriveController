
#ifndef SRC_DRIVER_LIQUIDDRIVER_POSITIONSENSOR_H_
#define SRC_DRIVER_LIQUIDDRIVER_POSITIONSENSOR_H_

#include "Common/Types.h"
#include "stm32f4xx.h"

/**
 * 位置传感器驱动
 */
typedef struct
{
    GPIO_TypeDef *port;
    Uint16 pin;
    uint32_t rcc;
}PositionSensor;

/**
 * 位置传感器电平
 */
typedef enum
{
    SENSOR_LOW_LEVEL,
    SENSOR_HIGH_LEVEL,
}SensorStatus;

void PositionSensor_Init(PositionSensor *positionSensor);
SensorStatus PositionSensor_ReadInputStatus(PositionSensor *positionSensor);

#endif /* SRC_DRIVER_LIQUIDDRIVER_POSITIONSENSOR_H_ */
