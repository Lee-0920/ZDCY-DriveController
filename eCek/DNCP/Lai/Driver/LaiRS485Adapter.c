/**
 * @file LaiRS485Adapter.c
 * @brief Lai层RS485使用的串口初始化
 * @details
 *
 * @version 1.0.0
 * @author xingfan
 * @date 2016-5-9
 */
#include "systemConfig.h"
#include "LaiRS485Adapter.h"

void LaiRS485Adapter_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 配置中断向量管理器的USARTx_IRQn的中断
    NVIC_InitStructure.NVIC_IRQChannel = LAIRS485ADAPTER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
            LAIRS485ADAPTER_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    //打开引脚时钟,串口时钟
    LAIRS485ADAPTER_CLK_CONFIG;
    LAIRS485ADAPTER_RX_GPIO_CLK_CONFIG;
    LAIRS485ADAPTER_TX_GPIO_CLK_CONFIG;

    //配置引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = LAIRS485ADAPTER_TX_PIN;
    GPIO_Init(LAIRS485ADAPTER_TX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LAIRS485ADAPTER_RX_PIN;
    GPIO_Init(LAIRS485ADAPTER_RX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(LAIRS485ADAPTER_TX_GPIO_PORT, LAIRS485ADAPTER_TX_PinSource,
            LAIRS485ADAPTER_GPIO_AF);
    GPIO_PinAFConfig(LAIRS485ADAPTER_RX_GPIO_PORT, LAIRS485ADAPTER_RX_PinSource,
            LAIRS485ADAPTER_GPIO_AF);

    //配置串口工作模式
    USART_InitStructure.USART_BaudRate = LAIRS485ADAPTER_UART_BAUD;
    USART_InitStructure.USART_HardwareFlowControl =
    USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(LAIRS485ADAPTERx, &USART_InitStructure);
    {
        uint16_t mantissa;
        uint16_t fraction;

        float temp = (float) (PERIPHERAL_SYSCLK * 1000000) / (LAIRS485ADAPTER_UART_BAUD * 16);
        mantissa = (uint16_t) temp;
        fraction = (uint16_t) ((temp - mantissa) * 16);
        mantissa <<= 4;
        mantissa += fraction;
        LAIRS485ADAPTERx->BRR = mantissa;
    }
    USART_ITConfig(LAIRS485ADAPTERx, USART_IT_RXNE, ENABLE);
    USART_Cmd(LAIRS485ADAPTERx, ENABLE);
    USART_ClearFlag(LAIRS485ADAPTERx, USART_FLAG_TC);

    //配置RS485收发切换引脚
    LAIRS485ADAPTER_DE_RE_GPIO_CLK_CONFIG;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = LAIRS485ADAPTER_DE_RE_SWITCH_PIN;
    GPIO_Init(LAIRS485ADAPTER_DE_RE_SWITCH_PORT, &GPIO_InitStructure);

    LaiRS485Adapter_RS485_SwitchToRx();
}
