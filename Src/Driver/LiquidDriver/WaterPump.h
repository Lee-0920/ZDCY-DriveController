/*
 * WaterPump.h
 *
 *  Created on: 2020年5月18日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_WATERPUMP_H_
#define SRC_DRIVER_LIQUIDDRIVER_WATERPUMP_H_

#include "Common/Types.h"
#include "stm32f4xx.h"

typedef enum
{
    PUMP_CLOSE,
    PUMP_OPEN
}WaterPumpValueStatus;

void WaterPump_Init(void);
void WaterPumpDriver_Control(WaterPumpValueStatus status);		//控制采水泵的开关
WaterPumpValueStatus WaterPumpDriver_ReadStatus(void);			//读取采水泵的状态


#endif /* SRC_DRIVER_LIQUIDDRIVER_WATERPUMP_H_ */
