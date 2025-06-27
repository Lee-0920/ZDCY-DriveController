/**
 * @addtogroup module_Common
 * @{
 */

/**
* @file
* @brief 安全堆管理。
* @details 提供安全堆内存管理（如分配和释放）接口。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-29
 */

#include <stdlib.h>

#include "Common/SafeHeap.h"

#include "Tracer/Trace.h"

#include "FreeRTOS.h"
#include "task.h"

void * SafeHeap_Alloc(unsigned int bytes)
{
    void * data;



    // 关中断保护（嵌入式C库内存分配函数不可重入）
    // data = malloc(bytes);

    data = pvPortMalloc(bytes);

    return data;
}


void SafeHeap_Free(void * data)
{
    // 关中断保护（嵌入式C库内存分配函数不可重入）
    // free(data);

    vPortFree(data);
}


/** @} */
