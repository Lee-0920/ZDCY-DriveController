/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 安全堆管理。
 * @details 提供安全堆内存管理（如分配和释放）接口。
 *  不同的平台可能有不同的实现。
 *  <p>实现时请保证以下安全项：
 *     - 线程安全：多任务之间、嵌套中断之间、中断与后台之间的访问安全。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-29
 */

#ifndef COMMON_SAFE_HEAP_H_
#define COMMON_SAFE_HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 在堆中安全分配内存块。
 * @param bytes 要分配的内存大小，单位为字节。
 * @return 分配好的内存地址。
 */
void * SafeHeap_Alloc(unsigned int bytes);

/**
 * @brief 在堆中安全释放内存块。
 * @param data 要释放的内存块地址。
 */
void SafeHeap_Free(void * data);


#ifdef __cplusplus
}
#endif


#endif  // COMMON_SAFE_HEAP_H_

/** @} */
