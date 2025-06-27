#include <ConsoleConfig.h>
/**
 * @addtogroup module_EmConsole
 * @{
 */

/**
 * @file
 * @brief 串口控制台驱动程序实现。
 * @details 实现串口控制台驱动程序，通过串口（UART）与上位机控制台程序通信。
 *  具体的UART配置（波特率、停止位、校验位等）由另外的模块实现，本模块只实现通信逻辑。
 * @version 1.1.1
 * @author kim.xiejinqiang
 * @date 2016-4-28
 */


#include "../Container/ByteFifo.h"
#include "ConsoleScheduler.h"
#include "ConsoleConfig.h"
#include "IConsoleDriver.h"
#include "Port/Driver/ConsoleUart.h"
#include "UartConsoleDriver.h"

// *******************************************************************
// 内部静态变量、宏定义

//--------------------------------------------
// 发送缓冲（环形缓冲）和读写索引
static unsigned char s_bufTx[CONSOLE_TX_BUF_SIZE];
static ByteFifo s_fifoTx;
static volatile unsigned char s_mode;


/**
 * @brief UartConsoleDriver 初始化。
 * @note 请确保调用此函数前，使用的 UART 硬件模块已经正确配置并初始化好了。
 */
void ConsoleDriver_Init()
{
    s_mode = CONSOLE_MODE_NONBLOCK;
    ByteFifo_Init(&s_fifoTx, s_bufTx, CONSOLE_TX_BUF_SIZE);
}

/**
 * @brief 切换数据发送模式。
 */
void ConsoleDriver_SwitchMode(unsigned char mode)
{
    if (mode == CONSOLE_MODE_BLOCK)
    {
        ConsoleUart_DisableTxInterrupt();
    }
    s_mode = mode;
}

/**
 * @brief UART 中断处理函数。
 */
void UartConsoleDriver_IntHandle(void)
{
    char needTrigger = 0;

    // 发送中断处理
    if (ConsoleUart_IsTxInterruptTriggered())
    {

        while (ConsoleUart_IsWritable() && (!ByteFifo_IsEmpty(&s_fifoTx)))
        {
            // 缓冲中还有数据待发送
            ConsoleUart_Write(ByteFifo_Pop(&s_fifoTx));
        }

        // 发送缓冲中无数据时，关闭发送中断
        if (ByteFifo_IsEmpty(&s_fifoTx))
        {
            ConsoleUart_DisableTxInterrupt();
        }
    }

    // 接收中断处理
    if (ConsoleUart_IsRxInterruptTriggered())
    {
        // 如果有事件触发，则激活其后台任务，交由后台处理
        while (ConsoleUart_IsReadable())
        {
            needTrigger |= g_kIfNewCharHandle(ConsoleUart_Read());
        }

        if (needTrigger)
        {
            ConsoleScheduler_Active();
        }
    }
}

int ConsoleDriver_WriteData(const char* data, int len)
{
    int i;

    if (s_mode == CONSOLE_MODE_NONBLOCK)
    {
        // 缓冲中剩余空间有多少，就最多写多少数据，超出部分部丢弃
        i = ByteFifo_GetRemain(&s_fifoTx);
        if (len > i)
            len = i;

        // 写数据到发送缓冲中
        for (i = 0; i < len; i++)
        {
            // 在 \n 前插入 \r
            if (data[i] == '\n')
            {
                ByteFifo_Push(&s_fifoTx, '\r');
            }
            ByteFifo_Push(&s_fifoTx, data[i]);
        }

        // 有数据则立即开始发送
        if (!ByteFifo_IsEmpty(&s_fifoTx))
        {
            // 关闭发送中断，防止因为竞发条件造成发送缓冲的索引混乱
            ConsoleUart_DisableTxInterrupt();

            while (ConsoleUart_IsWritable() && (!ByteFifo_IsEmpty(&s_fifoTx)))
            {
                // 缓冲中还有数据待发送
                ConsoleUart_Write(ByteFifo_Pop(&s_fifoTx));
            }

            // 还有数据等发送，应该打开发送中断
            if (!ByteFifo_IsEmpty(&s_fifoTx))
            {
                ConsoleUart_EnableTxInterrupt();
            }
        }
    }
    else    // CONSOLE_MODE_BLOCK
    {
        for (i = 0; i < len; i++)
        {
            // 在 \n 前插入 \r
            if (data[i] == '\n')
            {
                while (!ConsoleUart_IsWritable());
                ConsoleUart_Write('\r');
            }
            while (!ConsoleUart_IsWritable());
            ConsoleUart_Write(data[i]);
        }
    }

    return i;
}

/** @} */
