/**
 * @file ConsoleUart.h
 * @brief 命令行 Uart 驱动。
 * @details 定义 Console 模块使用到的 UART 通信模块的一些功能接口。
 *  <p>本驱动主要用于不同UART硬件模块的抽象，以保证上层模块驱动的相对独立性。
 *  <br>本驱动使用宏/内联函数实现，以提高上层模块底层执行的效率。
 *  <p>不同的UART硬件有不同的实现方式，请根据具体的硬件修改本驱动的实现。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2016-4-28
 */

#ifndef DRIVER_CONSOLE_UART_H_
#define DRIVER_CONSOLE_UART_H_

//******************************************************************
// 资源定义，在这里定义本驱动使用到相关资源
// 根据需要定义

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "Console/UartConsoleDriver.h"

//******************************************************************
// 串口相关定义，在这里定义使用的串口，中断通道，时钟，引脚，波特率
#define USARTx_IRQn                    USART6_IRQn
#define USARTx_CLK_CONFIG              RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE)
#define USARTx_RX_CONFIG               RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE)
#define USARTx_TX_CONFIG               RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE)
#define USARTx                         USART6
#define USARTx_GPIO_AF                 GPIO_AF_USART6
#define USARTx_TX_PIN                  GPIO_Pin_6
#define USARTx_TX_PinSource            GPIO_PinSource6
#define USARTx_TX_GPIO_PORT            GPIOC
#define USARTx_RX_PIN                  GPIO_Pin_7
#define USARTx_RX_PinSource            GPIO_PinSource7
#define USARTx_RX_GPIO_PORT            GPIOC
#define USARTx_UART_BAUD               115200
//#define USARTx_UART_BAUD               230400
//******************************************************************
// 功能定义，在这里实现驱动的所有功能
// 如果具体硬件没有相应的功能，请保持内联定义为空实现，而不能直接删除
//

/**
 * @brief 驱动初始化。
 * @details 初始化后，要保证：
 *  -# 开启了UART模块，正确配置了相关参数
 *  -# 开启了UART中断，且发送中断关闭，接收中断打开。
 */
void ConsoleUart_Init(void);



//------------------------------------------------------------
// 数据收发

/**
 * @brief 查询 UART 接收模块中是否有数据可读。
 * @note 上层可通过本操作实现阻塞接收功能；或者对于拥有接收 FIFO 的 CPU，
 *  为上层提供一种循环读取多个数据的判定方法（检测是否还有数据可读）。1为非空
 */
static inline bool ConsoleUart_IsReadable()
{
    return (USARTx->SR & USART_SR_RXNE);
}

/**
 * @brief 查询 UART 发送模块中是否有空间可写。
 * @note 上层可通过本操作实现阻塞发送功能；或者对于拥有发送 FIFO 的 CPU，
 *  为上层提供一种循环写入多个数据的判定方法（检测是否还有空间可写）。1为空
 */
static inline bool ConsoleUart_IsWritable()
{
    return (USARTx->SR & USART_SR_TXE);
}

/**
 * @brief 从接收模块中读取一个数据。
 * @note 对于无接收 FIFO 的 MCU，本操作直接从接收寄存器中读出数据并返回；
 *  对于拥有接收 FIFO 的 CPU，本操作仅从队头取出一个数据，如果需要循环读出所有数据，
 *  还需要结合 IsReadable() 操作，只要可读，就继续调用本操作读出数据。
 */
static inline unsigned char ConsoleUart_Read()
{
    return ((uint8_t) (USARTx->DR & (uint8_t) 0x00FF));
}

/**
 * @brief 写入一个数据到发送模块中。
 * @note 对于无发送 FIFO 的 MCU，本操作直接把数据存入发送寄存器中等待发送；
 *  对于拥有发送 FIFO 的 CPU，本操作仅把一个数据插入队尾，如果需要一次发送多个数据，
 *  还需要结合 IsWritable() 操作，只要可写，就继续调用本操作往队尾插入数据。
 */
static inline void ConsoleUart_Write(unsigned char data)
{
    USARTx->DR = (uint8_t) (data & (uint8_t) 0x00FF);
}

//------------------------------------------------------------
// 中断事务处理，仅在使用中断实现通信时，需要实现这些操作

/**
 * @brief 中断处理开始代码。
 * @details 本操作将在中断服务程序入口处被调用。
 * @note 有些 CPU 需要在中断服务程序开始处执行一些特定的操作（如清中断），
 *  此时可以把相应代码放在此处。如果不需要特定的操作，请保持为空。
 */
static inline void ConsoleUart_InterruptHandleBegin()
{
}

/**
 * @brief 中断处理结束代码。
 * @details 本操作将在中断服务程序的出口处被调用。
 * @note 有些 CPU 需要在中断服务程序的最后执行一些特定的操作（如清中断），
 *  此时可以把相应代码放在此处。如果不需要特定的操作，请保持为空。
 */
static inline void ConsoleUart_InterruptHandleEnd()
{

}

/**
 * @brief 查询 UART 中断状态是否为发送中断。发送中断为非0
 */
static inline bool ConsoleUart_IsTxInterruptTriggered()
{
    return (USART_GetITStatus(USARTx, USART_IT_TXE));
}

/**
 * @brief 查询 UART 中断状态是否为接收中断。接收中断为非0
 */
static inline bool ConsoleUart_IsRxInterruptTriggered()
{
    return (USART_GetITStatus(USARTx, USART_IT_RXNE));
}

/**
 * @brief 查询 UART 发送中断是否打开。1为打开
 */
static inline bool ConsoleUart_IsTxInterruptEnabled()
{
    return ((USARTx->CR1) & USART_CR1_RXNEIE);
}

/**
 * @brief 开启发送中断。
 */
static inline void ConsoleUart_EnableTxInterrupt()
{
    USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

/**
 * @brief 关闭发送中断（屏蔽）。
 */
static inline void ConsoleUart_DisableTxInterrupt()
{
    USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
}

#ifdef __cplusplus
}
#endif

#endif // DRIVER_CONSOLE_UART_H_
