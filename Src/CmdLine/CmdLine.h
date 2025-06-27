/**
 * @file
 * @brief 命令行。
 * @details
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-5-21
 */

#ifndef CMD_LINE_H_
#define CMD_LINE_H_

#include "Common/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初始化
void CmdLine_Init(void);
Bool IsEqual(const char* str1, const char* str2);

#ifdef __cplusplus
}
#endif

#endif // CMD_LINE_H_
