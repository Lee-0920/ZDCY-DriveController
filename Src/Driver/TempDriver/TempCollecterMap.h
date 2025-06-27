/*
 * TempCollecterMap.h
 *
 *  Created on: 2017年11月20日
 *      Author: LIANG
 */

#ifndef SRC_DRIVER_TEMPDRIVER_TEMPCOLLECTERMAP_H_
#define SRC_DRIVER_TEMPDRIVER_TEMPCOLLECTERMAP_H_

#include "Driver/TempDriver/TempCollecter.h"

#define MEAROOM_TEMP           		0
#define BACTROOM_TEMP         		1
#define MEAROOM_OUT_TEMP           	2
#define BACTROOM_OUT_TEMP         	3

#define TOTAL_TEMP               	1

void TempCollecterMap_Init(TempCollecter *tempCollecter);
char* TempCollecterMap_GetName(Uint8 index);

#endif /* SRC_DRIVER_TEMPDRIVER_TEMPCOLLECTERMAP_H_ */
