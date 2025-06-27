/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 系统相关调用。
 * @details 本文件中定义了操作系统的一些常用的操作，
 *  这些操作的具体实现代码应放在平台架构目录Arch下。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-3-4
 */
#include "FreeRTOS.h"
#include "task.h"
#include "Common/System.h"

void System_Sleep(unsigned int milliseconds)
{
    vTaskDelay(milliseconds/portTICK_RATE_MS);
}


/** @} */
