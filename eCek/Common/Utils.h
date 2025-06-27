/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 实用工具。
 * @details 本模块实现了项目中常用的一些工具函数。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-5-24
 */

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include "Types.h"
#include "Driver/System.h"
#include "Console/Console.h"


/**
 * @brief 软件延时（毫秒）。
 * @see System_Delay
 */
#define Delay       System_Delay

/**
 * @brief 软件延时（微秒）。
 * @see System_DelayUs
 */
#define DelayUs     System_DelayUs

/**
 * @brief 软件延时（秒）。
 * @see System_DelaySec
 */
#define DelaySec        System_DelaySec

#endif // COMMON_UTILS_H_

/** @} */
