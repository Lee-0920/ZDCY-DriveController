/*
 * DeviceIndicatorLED.h
 *
 *  Created on: 2016年7月23日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_DEVICEINDICATORLED_H_
#define SRC_DRIVER_DEVICEINDICATORLED_H_

void DeviceIndicatorLED_Init(void);
void DeviceIndicatorLED_SetBlink(Uint32 duration, Uint16 onTime, Uint16 offTime);

#endif /* SRC_DRIVER_DEVICEINDICATORLED_H_ */
