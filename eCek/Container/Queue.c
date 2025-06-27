/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 通用对象队列，用于容纳对象指针。
 * @details 提供循环队列的功能，作为基本的对象容器。
 * 	<p> 本模块提供了两套队列操作接口：
 * 	- 宏接口：适用于对性能要求较高的场合。宏接口只实现了基本的队列操作。
 * 	- 函数接口：适用于大部分场合，程序的可读性较高。
 * 	<p> 这两套操作接口的功能是完全一致，但调用形式略有不同。
 * @note 本模块不管理对象的内存分配和回收，调用者必须管理内存的分配工作。
 * 	<p>本队列设计时使用固定大小的结点缓冲，该缓冲内存由客户程序提供，
 * 	   在队列初始化时传入即可。因为缓冲是预留的，会较占用较大内存。
 * 	<p>队列使用的缓冲大小是有规定的，结点数只能是2的指数倍数。
 * @attention 非线程安全，如用在多线程环境中，应考虑资源竞争问题。当只
 *  有两个线程，一个是生产者一个是消费者时，则不用考虑竞争混乱问题。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-24
 */

#include "Queue.h"

//******************************************************************
// 队列操作函数


/**
 * @brief 队列初始化。
 * @param queue 要操作的队列。
 * @param buffer 队列要使用的缓冲区，缓冲区长度应满足 @p capacity 的要求。
 * @param capacity 队列的容量，即缓冲区能存放的对象个数。
 *  缓冲长度只能是2的指数值，并且不能大于无符号整数的最大值。
 * @note 受限于环形队列的实现机制，真正能用的容量比设置的 @p capacity 要少
 *  1 个，在调用 Queue_GetRemain() 时可以体现出来。
 */
void Queue_Init(Queue* queue, void** buffer, unsigned int capacity)
{
    queue->data = buffer;
    queue->mask = capacity - 1;
    queue->idxRead = 0;
    queue->idxWrite = 0;
}


/**
 * @brief 查询队列的容量。
 * @param queue 要查询的队列。
 * @return 队列容量，即能容纳最大结点的数目。
 */
unsigned int Queue_GetCapacity(Queue* queue)
{
    return (queue->mask + 1);
}


/**
 * @brief 查询队列是否为空。
 * @param queue 要查询的队列。
 * @return 队列是否为空。
 */
Bool Queue_IsEmpty(Queue* queue)
{
    return (queue->idxRead == queue->idxWrite);
}


/**
 * @brief 查询队列是否已满。
 * @param queue 要查询的队列。
 * @return 队列是否为满。
 */
Bool Queue_IsFull(Queue* queue)
{
    return (queue->idxRead == ((queue->idxWrite + 1) & queue->mask));
}


/**
 * @brief 查询队列中的元素个数。
 * @param queue 要查询的队列。
 * @return 队列中的元素个数。
 */
unsigned int Queue_GetCount(Queue* queue)
{
    return ((queue->idxWrite + queue->mask + 1 - queue->idxRead) & queue->mask);
}


/**
 * @brief 查询队列的剩余空间。
 * @param queue 要查询的队列。
 * @return 队列还能容纳的元素个数。
 */
unsigned int Queue_GetRemain(Queue* queue)
{
    return ((queue->idxRead + queue->mask - queue->idxWrite) & queue->mask);
}


/**
 * @brief 从队列前面取出一个结点，但不弹出。
 * @details 本操作不会更改队列的状态。
 * @param queue 要操作的队列。
 * @return 返回的最前端的结点对象。
 */
Object Queue_Peek(Queue* queue)
{
    return (Object)(queue->data[queue->idxRead]);
}


/**
 * @brief 把结点压入队列，置于队列的最后。
 * @details 请先调用 Queue_IsFull() 查询队列状态，确保队列不满。
 * @param queue 要操作的队列。
 * @param node 要压入的结点对象，结点内存由用户在上层分配。
 */
void Queue_Push(Queue* queue, Object node)
{
    if (queue->idxRead != ((queue->idxWrite + 1) & queue->mask))
    {
        queue->data[queue->idxWrite] = (void*) node;
        queue->idxWrite = (queue->idxWrite + 1) & queue->mask;
    }
}


/**
 * @brief 从队列最前端弹出一个结点并返回。
 * @param queue 要操作的队列。
 * @return 弹出的结点对象，上层使用后应手动释放内存。队列为空时将返回 0。
 */
Object Queue_Pop(Queue* queue)
{
    Object node = queue->data[queue->idxRead];
    queue->idxRead = (queue->idxRead + 1) & queue->mask;
    return node;
}


/** @} */
