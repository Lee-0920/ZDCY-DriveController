/*
 * UartDriver.c
 *
 *  Created on: 2020年3月18日
 *      Author: Administrator
 */

#include "UartDriver.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"
#include "SystemConfig.h"
//#include "DataTransmit/DataTransmitter.h"

void UartDriver_Init(UartConfigDef* config, SerialPortType type)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 配置中断向量管理器的USARTx_IRQn的中断
    NVIC_InitStructure.NVIC_IRQChannel = config->irq;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TRANSMITUART_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    //打开引脚时钟,串口时钟
    config->uartRccCmd(config->uartRcc, ENABLE);
    config->txRccCmd(config->txRcc, ENABLE);
    config->rxRccCmd(config->rxRcc, ENABLE);

    //配置引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = config->txPin;
    GPIO_Init(config->txPort, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = config->rxPin;
    GPIO_Init(config->rxPort, &GPIO_InitStructure);

    GPIO_PinAFConfig(config->txPort, config->txPinSource, config->afMap);
    GPIO_PinAFConfig(config->rxPort, config->rxPinSource, config->afMap);

    //配置串口工作模式
    USART_InitStructure.USART_BaudRate = config->portParam.baud;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    switch(config->portParam.parity)
    {
        case Parity_No:
            USART_InitStructure.USART_Parity = USART_Parity_No;
            break;
        case Parity_Even:
            USART_InitStructure.USART_Parity = USART_Parity_Even;
            break;
        case Parity_Odd:
            USART_InitStructure.USART_Parity = USART_Parity_Odd;
            break;
        default:
            USART_InitStructure.USART_Parity = USART_Parity_No;
            break;
    }
    switch(config->portParam.stopBits)
    {
        case StopBits_1:
            USART_InitStructure.USART_StopBits = USART_StopBits_1;
            break;
        case StopBits_1_5:
            USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
            break;
        case StopBits_2:
            USART_InitStructure.USART_StopBits = USART_StopBits_2;
            break;
        default:
            USART_InitStructure.USART_StopBits = USART_StopBits_1;
            break;
    }
    switch(config->portParam.wordLen)
    {
        case 8:
            USART_InitStructure.USART_WordLength = USART_WordLength_8b;
            break;
        case 9:
            USART_InitStructure.USART_WordLength = USART_WordLength_9b;
            break;
        default:
            USART_InitStructure.USART_WordLength = USART_WordLength_8b;
            break;
    }
    USART_Init(config->uart, &USART_InitStructure);
    {
        uint16_t mantissa;
        uint16_t fraction;

        float temp = (float) (config->clk) / (config->portParam.baud * 16);
        mantissa = (uint16_t) temp;
        fraction = (uint16_t) ((temp - mantissa) * 16);
        mantissa <<= 4;
        mantissa += fraction;
        config->uart->BRR = mantissa;
    }
    USART_ITConfig(config->uart, USART_IT_RXNE, ENABLE);
    USART_Cmd(config->uart, ENABLE);
    USART_ClearFlag(config->uart, USART_FLAG_TC);

    //配置RS485收发切换引脚
    if(type == RS485)
    {
        config->swRccCmd(config->swRcc, ENABLE);

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Pin = config->swPin;
        GPIO_Init(config->swPort, &GPIO_InitStructure);

        UartDriver_RS485_SwitchToRx(config);
    }
}

/**
 * @brief 设置RS485驱动芯片为发送状态
 */
void UartDriver_RS485_SwitchToTx(UartConfigDef* config)
{
   ///config->swPort->BSRRL |= config->swPin;
   GPIO_SetBits(config->swPort, config->swPin);
}

/**
 * @brief 设置485驱动芯片为接收状态
 */
void UartDriver_RS485_SwitchToRx(UartConfigDef* config)
{
   ///config->swPort->BSRRH |= config->swPin;
   GPIO_ResetBits(config->swPort, config->swPin);
}
