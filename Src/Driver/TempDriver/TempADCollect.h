/**
 * @file
 * @brief 消解温度采集驱动头文件。
 * @details 提供采集消解温度功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2015-06-04
 */

#ifndef DRIVER_TEMPADCOLLECT_H_
#define DRIVER_TEMPADCOLLECT_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

void TempADCollect_Init(void);
uint16_t TempADCollect_GetAD(uint8_t index);
#ifdef __cplusplus
}
#endif

#endif /* DRIVER_TEMPADCOLLECT_H_ */
