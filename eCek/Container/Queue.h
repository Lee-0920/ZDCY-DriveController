/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 通用对象队列，用于容纳对象指针。
 * @details 提供循环队列的功能，作为基本的对象容器。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-24
 */

#ifndef COMMON_CONTAINER_QUEUE_H_
#define COMMON_CONTAINER_QUEUE_H_

#include "Common/Types.h"

//******************************************************************
// 队列数据结构
// 

/**
 * @brief 抽象对象，同时也是队列的一个结点
 */
typedef void* Object;

/**
 * @brief 对象队列。
 * @details 用户不必关心该对象内的具体成员，调用相关操作即可。
 */
typedef struct QueueStruct
{
	void** data;                        ///< 队列的数据缓冲
    unsigned int mask;                  ///< 队列长度的掩码
    unsigned int idxRead;               ///< 读指针
    unsigned int idxWrite;              ///< 写指针
}
Queue;


//******************************************************************
// 队列操作宏

/**
 * @brief 队列初始化。
 * @param queue 要初始化的队列。注意传入的是对象，而不是指针。
 * @param buffer 队列要使用的缓冲区，缓冲区长度应满足 @p capacity 的要求。
 * @param capacity 队列的容量，即缓冲区能存放的对象个数。
 *  缓冲长度只能是2的指数值，并且不能大于无符号整数的最大值。
 * @note 受限于环形队列的实现机制，真正能用的容量比设置的 @p capacity 要少
 *  1 个，在调用 @ref QUEUE_GET_REMAIN 时可以体现出来。
 * @see Queue_Init
 */
#define QUEUE_INIT(queue, buffer, capacity)                 \
{                                                           \
    (queue).data = (void**)(buffer);                        \
    (queue).mask = capacity - 1;                            \
    (queue).idxRead = 0;                                    \
    (queue).idxWrite = 0;                                   \
}

/**
 * @brief 查询队列的容量。
 * @param queue 要查询的队列。注意传入的是对象，而不是指针。
 * @return 队列容量，即能容纳最大结点的数目。
 * @see Queue_GetCapacity
 */
#define QUEUE_GET_CAPACITY(queue)                           \
		((queue).mask + 1)

/**
 * @brief 查询队列是否为空。
 * @param queue 要查询的队列。注意传入的是对象，而不是指针。
 * @see Queue_IsEmpty
 */
#define QUEUE_IS_EMPTY(queue)                               \
    ((queue).idxRead == (queue).idxWrite)


/**
 * @brief 查询队列是否为满。
 * @param queue 要查询的队列。注意传入的是对象，而不是指针。
 * @see Queue_IsFull
 */
#define QUEUE_IS_FULL(queue)                               \
    ((queue).idxRead == (((queue).idxWrite + 1) & (queue).mask))

/**
 * @brief 查询队列中的元素个数。
 * @param queue 要查询的队列。注意传入的是对象，而不是指针。
 * @return 队列中的元素个数。
 * @see Queue_GetCount
 */
#define QUEUE_GET_COUNT(queue)                              \
    (((queue).idxWrite + (queue).mask + 1 - (queue).idxRead) & (queue).mask)

/**
 * @brief 查询队列的剩余空间。
 * @param queue 要查询的队列。注意传入的是对象，而不是指针。
 * @return 队列还能容纳的元素个数。
 * @see Queue_GetRemain
 */
#define QUEUE_GET_REMAIN(queue)                             \
    (((queue).idxRead + (queue).mask - (queue).idxWrite) & (queue).mask)

/**
 * @brief 把结点压入队列，置于队列的最后。
 * @details 请先调用 @ref QUEUE_IS_FULL 查询队列状态， 确保队列不满。
 *  在队列满的情况调用本函数无效。
 * @param queue 要操作的队列。注意传入的是对象，而不是指针。
 * @param node 要压入结点的指针，结点内存由用户在上层分配。
 * @see Queue_Push
 */
#define QUEUE_PUSH(queue, node)                             \
{                                                           \
    if (!QUEUE_IS_FULL(queue))                              \
    {                                                       \
        (queue).data[(queue).idxWrite] = (void*)(node);     \
        (queue).idxWrite = ((queue).idxWrite + 1) & (queue).mask; \
    }                                                       \
}

/**
 * @brief 从队列最前端取出一个结点，但不弹出。
 * @param queue 要操作的队列。注意传入的是对象，而不是指针。
 * @see Queue_Peek
 */
#define QUEUE_PEEK(queue)                                   \
    ((void*)((queue).data[(queue).idxRead]))

/**
 * @brief 从队列最前端安静弹出一个结点，但不返回。
 * @details 该宏不返回弹出的结点。如果希望返回弹出的结点，应先调用
 * 	 @ref QUEUE_PEEK 取得队列的头结点。
 * @param queue 要操作的队列。注意传入的是对象，而不是指针。
 * @see Queue_Pop
 */
#define QUEUE_POP(queue)                                    \
{                                                           \
	if (!QUEUE_IS_EMPTY(queue))                             \
    {                                                       \
        (queue).idxRead = ((queue).idxRead + 1) & (queue).mask; \
    }                                                       \
}


//******************************************************************
// 队列操作函数


#ifdef __cplusplus
extern "C" {
#endif

extern void Queue_Init(Queue* queue, void** buffer, unsigned int capacity);

extern Bool Queue_IsEmpty(Queue* queue);
extern Bool Queue_IsFull(Queue* queue);
extern unsigned int Queue_GetCapacity(Queue* queue);
extern unsigned int Queue_GetCount(Queue* queue);
extern unsigned int Queue_GetRemain(Queue* queue);

extern void Queue_Push(Queue* queue, Object node);
extern Object Queue_Peek(Queue* queue);
extern Object Queue_Pop(Queue* queue);


#ifdef __cplusplus
}
#endif


#endif  // COMMON_CONTAINER_QUEUE_H_

/** @} */
