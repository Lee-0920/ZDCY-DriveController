/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 字节FIFO，用于容纳字节。
 * @details 提供循环队列的功能，作为基本的字节容器。
 *  本容器同时提供了宏接口和函数接口，使用时选择其中一个即可。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-8-23
 */

#ifndef COMMON_CONTAINER_BYTEFIFO_H_
#define COMMON_CONTAINER_BYTEFIFO_H_

#include "Common/Types.h"

//******************************************************************
// FIFO数据结构
// 

/**
 * @brief 字节FIFO。
 * @details 用户不必关心该ByteFifo内的具体成员，调用相关操作即可。
 */
typedef struct ByteFifoStruct
{
	unsigned char* buffer;              ///< FIFO的数据缓冲
    unsigned int mask;                  ///< FIFO长度的掩码
    volatile unsigned int idxRead;      ///< 读指针
    volatile unsigned int idxWrite;     ///< 写指针
}
ByteFifo;


//******************************************************************
// FIFO操作宏

/**
 * @brief FIFO初始化。
 * @param fifo 要初始化的FIFO。注意传入的是变量，而不是指针。
 * @param byteBuffer FIFO要使用的缓冲区，缓冲区长度应满足 @p capacity 的要求。
 * @param capacity FIFO的容量，即缓冲区能存放的字节数。
 *  缓冲长度只能是2的指数值，并且不能大于无符号整数的最大值。
 * @note 受限于环形FIFO的实现机制，真正能用的容量比设置的 @p capacity 要少
 *  1 个，在调用 @ref BYTEFIFO_GET_REMAIN 时可以体现出来。
 * @see ByteFifo_Init
 */
#define BYTEFIFO_INIT(fifo, byteBuffer, capacity)               \
{                                                               \
    (fifo).buffer = (unsigned char*)(byteBuffer);               \
    (fifo).mask = capacity - 1;                                 \
    (fifo).idxRead = 0;                                         \
    (fifo).idxWrite = 0;                                        \
}

/**
 * @brief 查询FIFO的容量。
 * @param fifo 要查询的FIFO。注意传入的是变量，而不是指针。
 * @return FIFO容量，即能容纳最大字节数。
 * @see ByteFifo_GetCapacity
 */
#define BYTEFIFO_GET_CAPACITY(fifo)                             \
		((fifo).mask + 1)

/**
 * @brief 查询FIFO是否为空。
 * @param fifo 要查询的FIFO。注意传入的是变量，而不是指针。
 * @see ByteFifo_IsEmpty
 */
#define BYTEFIFO_IS_EMPTY(fifo)                                 \
    ((fifo).idxRead == (fifo).idxWrite)


/**
 * @brief 查询FIFO是否为满。
 * @param fifo 要查询的FIFO。注意传入的是变量，而不是指针。
 * @see ByteFifo_IsFull
 */
#define BYTEFIFO_IS_FULL(fifo)                                  \
    ((fifo).idxRead == (((fifo).idxWrite + 1) & (fifo).mask))

/**
 * @brief 清空 FIFO。
 * @param fifo 要清空的FIFO。
 */
#define BYTEFIFO_CLEAR(fifo)                                    \
    ((fifo).idxRead = (fifo).idxWrite = 0)

/**
 * @brief 查询FIFO中已存入的字节数。
 * @param fifo 要查询的FIFO。注意传入的是变量，而不是指针。
 * @return FIFO中的字节数。
 * @see ByteFifo_GetCount
 */
#define BYTEFIFO_GET_COUNT(fifo)                                \
    (((fifo).idxWrite + (fifo).mask + 1 - (fifo).idxRead) & (fifo).mask)

/**
 * @brief 查询FIFO的剩余空间。
 * @param fifo 要查询的FIFO。注意传入的是变量，而不是指针。
 * @return FIFO还能容纳的字节数。
 * @see ByteFifo_GetRemain
 */
#define BYTEFIFO_GET_REMAIN(fifo)                               \
    (((fifo).idxRead + (fifo).mask - (fifo).idxWrite) & (fifo).mask)

/**
 * @brief 把数据压入FIFO，置于FIFO的最后。
 * @details 请先调用 @ref BYTEFIFO_IS_FULL 查询FIFO状态， 确保FIFO不满。
 *  在FIFO满的情况调用本宏无效。
 * @param fifo 要操作的FIFO。注意传入的是变量，而不是指针。
 * @param data 要压入的字节。
 * @see ByteFifo_Push
 */
#define BYTEFIFO_PUSH(fifo, data)                               \
{                                                               \
    if (!BYTEFIFO_IS_FULL(fifo))                                \
    {                                                           \
        (fifo).buffer[(fifo).idxWrite] = (data);                \
        (fifo).idxWrite = ((fifo).idxWrite + 1) & (fifo).mask;  \
    }                                                           \
}

/**
 * @brief 从FIFO最前端取出一个数据，但不弹出。
 * @param fifo 要操作的FIFO。注意传入的是变量，而不是指针。
 * @see ByteFifo_Peek
 */
#define BYTEFIFO_PEEK(fifo)                                     \
    ((fifo).buffer[(fifo).idxRead])

/**
 * @brief 从FIFO最前端安静弹出一个数据，但不返回。
 * @details 该宏不返回弹出的数据。如果希望返回弹出的数据，应先调用
 * 	 @ref BYTEFIFO_PEEK 取得FIFO的头字节。
 * @param fifo 要操作的FIFO。注意传入的是变量，而不是指针。
 * @see ByteFifo_Pop
 */
#define BYTEFIFO_POP(fifo)                                      \
{                                                               \
	if (!BYTEFIFO_IS_EMPTY(fifo))                               \
    {                                                           \
        (fifo).idxRead = ((fifo).idxRead + 1) & (fifo).mask;    \
    }                                                           \
}


//******************************************************************
// FIFO操作函数


#ifdef __cplusplus
extern "C" {
#endif


 /**
  * @brief FIFO初始化。
  * @param fifo 要操作的FIFO。
  * @param buffer FIFO要使用的缓冲区，缓冲区长度应满足 @p capacity 的要求。
  * @param capacity FIFO的容量，即缓冲区能存放的字节数。
  *  缓冲长度只能是2的指数值，并且不能大于无符号整数的最大值。
  * @note 受限于环形FIFO的实现机制，真正能用的容量比设置的 @p capacity 要少
  *  1 个，在调用 ByteFifo_GetRemain() 时可以体现出来。
  */
static inline void ByteFifo_Init(ByteFifo* fifo, unsigned char* buffer, unsigned int capacity)
{
    fifo->buffer = buffer;
    fifo->mask = capacity - 1;
    fifo->idxRead = 0;
    fifo->idxWrite = 0;
}


/**
 * @brief 查询FIFO是否为空。
 * @param fifo 要查询的FIFO。
 * @return FIFO是否为空。
 */
static inline Bool ByteFifo_IsEmpty(ByteFifo* fifo)
{
    return (fifo->idxRead == fifo->idxWrite);
}


/**
 * @brief 查询FIFO是否已满。
 * @param fifo 要查询的FIFO。
 * @return FIFO是否为满。
 */
static inline Bool ByteFifo_IsFull(ByteFifo* fifo)
{
    return (fifo->idxRead == ((fifo->idxWrite + 1) & fifo->mask));
}


/**
 * @brief 清空 FIFO。
 * @param fifo 要清空的FIFO。
 */
static inline void ByteFifo_Clear(ByteFifo* fifo)
{
    fifo->idxRead = fifo->idxWrite;
}


/**
 * @brief 查询FIFO的容量。
 * @param fifo 要查询的FIFO。
 * @return FIFO容量，即能容纳最大字节数。
 */
static inline unsigned int ByteFifo_GetCapacity(ByteFifo* fifo)
{
    return (fifo->mask + 1);
}


/**
 * @brief 查询FIFO中已存入的字节数。
 * @param fifo 要查询的FIFO。
 * @return FIFO中的字节数。
 */
static inline unsigned int ByteFifo_GetCount(ByteFifo* fifo)
{
    return ((fifo->idxWrite + fifo->mask + 1 - fifo->idxRead) & fifo->mask);
}


/**
 * @brief 查询FIFO的剩余空间。
 * @param fifo 要查询的FIFO。
 * @return FIFO还能容纳的字节数。
 */
static inline unsigned int ByteFifo_GetRemain(ByteFifo* fifo)
{
    return ((fifo->idxRead + fifo->mask - fifo->idxWrite) & fifo->mask);
}


/**
 * @brief 把数据压入FIFO，置于FIFO的最后。
 * @details 请先调用 ByteFifo_IsFull() 查询FIFO状态，确保FIFO不满。
 * @param fifo 要操作的FIFO。
 * @param data 要压入的字节。
 */
static inline void ByteFifo_Push(ByteFifo* fifo, unsigned char data)
{
    if (fifo->idxRead != ((fifo->idxWrite + 1) & fifo->mask))
    {
        fifo->buffer[fifo->idxWrite] = data;
        fifo->idxWrite = (fifo->idxWrite + 1) & fifo->mask;
    }
}


/**
 * @brief 从FIFO前面取出一个数据，但不弹出。
 * @details 本操作不会更改FIFO的状态。
 * @param fifo 要操作的FIFO。
 * @return 返回的最前端的字节。
 */
static inline unsigned char ByteFifo_Peek(ByteFifo* fifo)
{
    return (unsigned char)(fifo->buffer[fifo->idxRead]);
}


/**
 * @brief 从FIFO最前端弹出一个数据并返回。
 * @param fifo 要操作的FIFO。
 * @return 弹出的字节。FIFO为空时返回的值不确定。
 */
static inline unsigned char ByteFifo_Pop(ByteFifo* fifo)
{
    unsigned char data = fifo->buffer[fifo->idxRead];
    fifo->idxRead = (fifo->idxRead + 1) & fifo->mask;
    return data;
}


#ifdef __cplusplus
}
#endif


#endif  // COMMON_CONTAINER_BYTEFIFO_H_

/** @} */
