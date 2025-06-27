/*
 * PumpEventScheduler.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#ifndef SRC_PERISTALTICPUMP_PUMPEVENTSCHEDULER_H_
#define SRC_PERISTALTICPUMP_PUMPEVENTSCHEDULER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "LuipApi/PeristalticPumpInterface.h"
#include "Common/Types.h"

void PumpEventScheduler_Init(void);
void PumpEventScheduler_SendEvent(Uint8 pumpNumber, enum PumpResult pumpResult, Bool isSendEvent);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PERISTALTICPUMP_PUMPEVENTSCHEDULER_H_ */
