/*
 * DrainWaterPump.h
 *
 *  Created on: 2022年6月10日
 *      Author: hyz
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_DRAINWATERPUMP_H_
#define SRC_DRIVER_LIQUIDDRIVER_DRAINWATERPUMP_H_

#include "Common/Types.h"
#include "stm32f4xx.h"
#include "WaterPump.h"

void DrainWaterPump_Init(void);
void DrainWaterPumpDriver_Control(WaterPumpValueStatus status);		//控制采水泵的开关
WaterPumpValueStatus DrainWaterPumpDriver_ReadStatus(void);			//读取采水泵的状态


#endif /* SRC_DRIVER_LIQUIDDRIVER_DRAINWATERPUMP_H_ */
