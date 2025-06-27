/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 命令行跟踪调试信息输出。
 * @details
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-4-8
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "./Trace.h"
#include "DncpStack/DncpStack.h"
#include "LuipApi/DataTransmitInterface.h"


unsigned char g_traceLevel = TRACE_LEVEL_INFO;

/**
 * @brief 设置调试信息的动态输出级别。
 * @param level 级别代码，数值越大，输出的调试信息将越多。
 */
void Trace_SetLevel(unsigned char level)
{
    g_traceLevel = level;
}

/**
 * @brief 获取调试信息的输出级别。
 * @return 级别代码。
 */
unsigned char Trace_GetLevel()
{
    return g_traceLevel;
}

void Debug_Log(char *format, ...)
{
    char buffer[80];
    va_list aptr;
    int len=0;
    
    va_start(aptr, format);
    vsprintf(buffer, format, aptr);
    va_end(aptr);
    len = strlen(buffer);
    DncpStack_SendEvent(DSCP_EVENT_DTI_UPLOAD_LOG, buffer, len+1);
}


/** @} */
