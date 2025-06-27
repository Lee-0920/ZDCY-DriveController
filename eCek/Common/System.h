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

#ifndef COMMON_SYSTEM_H_
#define COMMON_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 挂起当前线程，使系统等待一段时间再往下执行。
 * @param milliseconds 要挂起的毫秒数。
 */
void System_Sleep(unsigned int milliseconds);


#ifdef __cplusplus
}
#endif


#endif  // COMMON_SYSTEM_H_

/** @} */
