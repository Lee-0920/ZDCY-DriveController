/**
 * @file LaiRS485Adapter.h
 * @brief Lai层RS485使用的串口相关宏定义以及串口的实现功能
 * @details
 *
 * @version 1.0.0
 * @author xingfan
 * @date 2016-5-9
 */
#ifndef _ECEK_DNCP_LAI_DRIVER_LAIRS485ADAPTER_H_
#define _ECEK_DNCP_LAI_DRIVER_LAIRS485ADAPTER_H_
//******************************************************************
// 资源定义，在这里定义本适配器使用到相关资源
// 根据需要定义
//

// 头文件包含
#include "stm32f4xx.h"

//******************************************************************
// 串口相关定义，在这里定义使用的串口，中断通道，时钟，引脚，波特率和RS485
//收发切换引脚
#define PERIPHERAL_SYSCLK                         90            //串口使用的时钟频率，APB1 为45M，APB2为90M
#define LAIRS485ADAPTER_IRQn                      USART1_IRQn
#define LAIRS485ADAPTER_CLK_CONFIG                RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)
#define LAIRS485ADAPTER_RX_GPIO_CLK_CONFIG        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define LAIRS485ADAPTER_TX_GPIO_CLK_CONFIG        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define LAIRS485ADAPTERx                          USART1
#define LAIRS485ADAPTER_GPIO_AF                   GPIO_AF_USART1
#define LAIRS485ADAPTER_TX_PIN                    GPIO_Pin_9
#define LAIRS485ADAPTER_TX_PinSource              GPIO_PinSource9
#define LAIRS485ADAPTER_TX_GPIO_PORT              GPIOA
#define LAIRS485ADAPTER_RX_PIN                    GPIO_Pin_10
#define LAIRS485ADAPTER_RX_PinSource              GPIO_PinSource10
#define LAIRS485ADAPTER_RX_GPIO_PORT              GPIOA
#define LAIRS485ADAPTER_UART_BAUD                 115200
//#define LAIRS485ADAPTER_UART_BAUD                 230400

#define LAIRS485ADAPTER_DE_RE_SWITCH_PORT          GPIOA
#define LAIRS485ADAPTER_DE_RE_SWITCH_PIN           GPIO_Pin_11
#define LAIRS485ADAPTER_DE_RE_GPIO_CLK_CONFIG      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)

extern void LaiRS485Adapter_Init(void);
//******************************************************************
// 功能定义，在这里实现适配器的所有功能
// 如果具体硬件没有相应的功能，请保持宏定义为空，而不能直接删除
//
/**
 * @brief 设置RS485驱动芯片为发送状态
 */
static void LaiRS485Adapter_RS485_SwitchToTx(void)
{
    LAIRS485ADAPTER_DE_RE_SWITCH_PORT->BSRRL |= LAIRS485ADAPTER_DE_RE_SWITCH_PIN;
}
/**
 * @brief 设置RS485驱动芯片为接收状态
 */
static void LaiRS485Adapter_RS485_SwitchToRx(void)
{
    LAIRS485ADAPTER_DE_RE_SWITCH_PORT->BSRRH |= LAIRS485ADAPTER_DE_RE_SWITCH_PIN;
}
/**
 * @brief 中断处理初始化。
 * @details 有些CPU需要手动清除中断状态，可以把清中代码放在此处。
 * @note 调用时应放在中断处理函数的最前面（变量定义后面）。
 */
static void LaiRS485Adapter_INT_HANDLE_INIT(void)
{

}
/**
 * @brief 查询 中断状态是否为发送中断。发送中断为非0
 */
static inline bool LaiRS485Adapter_INT_STATUS_IS_TX(void)
{
    return (((LAIRS485ADAPTERx->SR & USART_SR_TXE) && (LAIRS485ADAPTERx->CR1 & USART_CR1_TXEIE)) || ((LAIRS485ADAPTERx->CR1 & USART_CR1_TCIE) && (LAIRS485ADAPTERx->SR & USART_SR_TC)));
}
/**
 * @brief 查询 中断状态是否为接收中断。接收中断为非0
 */
static inline bool LaiRS485Adapter_INT_STATUS_IS_RX(void)
{
    return ((LAIRS485ADAPTERx->CR1 & USART_CR1_RXNEIE) && (LAIRS485ADAPTERx->SR & USART_SR_RXNE));
}
/**
 * @brief 开启发送寄存器空中断。
 */
static inline void LaiRS485Adapter_INT_TXE_ENABLE(void)
{
    LAIRS485ADAPTERx->CR1 |= USART_CR1_TXEIE;
}
/**
 * @brief 关闭发送寄存器空中断。
 */
static inline void LaiRS485Adapter_INT_TXE_DISABLE()
{
    LAIRS485ADAPTERx->CR1 &= 0xff7f;
}
/**
 * @brief 开启发送完成中断。
 */
static inline void LaiRS485Adapter_INT_TC_ENABLE(void)
{
    LAIRS485ADAPTERx->CR1 |= USART_CR1_TCIE;
}
/**
 * @brief 关闭发送完成中断。
 */
static inline void LaiRS485Adapter_INT_TC_DISABLE()
{
    LAIRS485ADAPTERx->CR1 &= 0xffbf;
}
/**
 * @brief 开启接收中断。
 */
static inline void LaiRS485Adapter_INT_RX_ENABLE(void)
{
    USART_ITConfig(LAIRS485ADAPTERx, USART_IT_RXNE, ENABLE);
}
/**
 * @brief 关闭接收中断。
 */
static inline void LaiRS485Adapter_INT_RX_DISABLE(void)
{
    USART_ITConfig(LAIRS485ADAPTERx, USART_IT_RXNE, DISABLE);
}
/**
 * @brief 查询 接收模块中是否仍然有数据可读。
 * @details 本接口主要用于有硬件FIFO 的情形，
 *  在读取一个数据后，试图再读前的判断。
 *  如果没有 FIFO，请把本宏直接定义为 0。
 */
#define LaiRS485Adapter_DATA_STILL_AVAIL()                             \
    0

/**
 * @brief 查询 UART 接收模块中是否有数据可读。
 */
#define LaiRS485Adapter_DATA_AVAIL()                                   \
    0

/**
 * @brief 查询 UART 发送模块中是否有空间可写。
 */
#define LaiRS485Adapter_SPACE_AVAIL()                                  \
    0

/**
 * @brief 查询发送模块中是否仍然有空间可写。
 * @details 本接口主要用于有硬件FIFO 的情形，
 *  在写入一个数据后，试图再写前的判断。
 *  如果没有 FIFO，请把本宏直接定义为 0。
 */
#define LaiRS485Adapter_SPACE_STILL_AVAIL()                            \
    0

/**
 * @brief 从接收模块中读取一个数据。
 */
static inline unsigned char LaiRS485Adapter_DATA_READ(void)
{
    return ((uint8_t) (LAIRS485ADAPTERx->DR & (uint8_t) 0x00FF));
}
/**
 * @brief 写入一个数据到发送模块中。
 */
static inline void LaiRS485Adapter_DATA_WRITE(unsigned char data)
{
    LAIRS485ADAPTERx->DR = (uint8_t) (data & (uint8_t) 0x00FF);
}

static inline bool LaiRS485Adapter_IsWritable()
{
    return (LAIRS485ADAPTERx->SR & USART_SR_TXE);
}

static inline bool LaiRS485Adapter_IsReadable()
{
    return (LAIRS485ADAPTERx->SR & USART_SR_RXNE);
}
#endif

