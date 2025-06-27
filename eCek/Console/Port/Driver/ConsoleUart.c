/**
 * @addtogroup module_Driver
 * @{
 */

/**
 * @file
 * @brief 命令行 Uart 驱动。
 * @details 仅供串口命令行程序使用，见 @ref Console.c
 *  主要的 RS232 通信实现移至 Console 模块。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-10-31
 */

#include "console/port/driver/ConsoleUart.h"
#include "SystemConfig.h"
/**
 * @brief ConsoleUart 驱动初始化。
 */
void ConsoleUart_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    //打开引脚时钟,串口时钟
    USARTx_RX_CONFIG;
    USARTx_TX_CONFIG;
    USARTx_CLK_CONFIG;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
    GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
    GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_PinSource, USARTx_GPIO_AF);
    GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_PinSource, USARTx_GPIO_AF);

    //配置串口工作模式
    USART_InitStructure.USART_BaudRate = USARTx_UART_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
            USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);
    {
        uint16_t mantissa;
        uint16_t fraction;

        float temp = (float) (90 * 1000000) / (USARTx_UART_BAUD * 16);
        mantissa = (uint16_t) temp;
        fraction = (uint16_t) ((temp - mantissa) * 16);
        mantissa <<= 4;
        mantissa += fraction;
        USARTx->BRR = mantissa;
    }
    USART_Cmd(USARTx, ENABLE);
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

    // 配置中断向量管理器的USARTx_IRQn的中断
    NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
            CONSOLEUART_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief USART6 中断服务程序。
 */
void USART6_IRQHandler(void)
{
    ConsoleUart_InterruptHandleBegin();
    UartConsoleDriver_IntHandle();
    ConsoleUart_InterruptHandleEnd();
}
/** @} */
