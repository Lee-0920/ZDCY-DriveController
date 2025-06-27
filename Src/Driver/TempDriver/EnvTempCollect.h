/**
 * @file
 * @brief 环境温度采集驱动头文件。
 * @details 提供采集环境温度功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2015-06-04
 */

#ifndef ENVTEMPERATUREADC_H_
#define ENVTEMPERATUREADC_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

void EnvTempCollect_Init(void);
float EnvironmentTemp_Get();

#ifdef __cplusplus
}
#endif


#endif /* ENVTEMPERATUREADC_H_ */



/******************* (C) COPYRIGHT 2009 LABSUN *****************END OF FILE****/

