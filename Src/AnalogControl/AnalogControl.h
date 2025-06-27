/*
 * AnalogControl.h
 *
 *  Created on: 2020年5月22日
 *      Author: Administrator
 */

#ifndef SRC_ANALOGCONTROL_ANALOGCONTROL_H_
#define SRC_ANALOGCONTROL_ANALOGCONTROL_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

void AnalogController_Init(void);
Uint16 AnalogController_GetAINumber(void);
void AnalogController_StartFlow(void);
void AnalogController_StopFlow(void);
Uint16 AnalogController_GetAIData(Uint16 index);
Bool AnalogController_GetAllAIData(Uint16* num, Uint32* data);
void AnalogController_SetAIUploadPeriod(Uint32 time);

#endif /* SRC_ANALOGCONTROL_ANALOGCONTROL_H_ */
