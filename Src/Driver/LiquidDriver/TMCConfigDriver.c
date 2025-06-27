/*
 * TMCConfigDriver.c
 *
 *  Created on: 2020年1月7日
 *      Author: Administrator
 */
#include "TMCConfigDriver.h"
#include "SystemConfig.h"
#include "Tracer/Trace.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/********串口配置********/
#define TMC_UART                                               USART3
#define TMC_UART_CLK                                       45                                //串口使用的时钟频率，APB1 为45M，APB2为90M
#define TMC_UART_BAUD                                   38400
#define TMC_UART_IRQn                                      USART3_IRQn
#define TMC_UART_IRQHANDLER                      USART3_IRQHandler
#define TMC_UART_RCC                                       RCC_APB1Periph_USART3

#define TMC_UART_GPIO_AF                               GPIO_AF_USART3
#define TMC_UART_TX_PIN                                   GPIO_Pin_10
#define TMC_UART_TX_PinSource                       GPIO_PinSource10
#define TMC_UART_TX_PORT                               GPIOB
#define TMC_UART_TX_RCC                                RCC_AHB1Periph_GPIOB

#define TMC_UART_RX_PIN                                  GPIO_Pin_11
#define TMC_UART_RX_PinSource                       GPIO_PinSource11
#define TMC_UART_RX_PORT                               GPIOB
#define TMC_UART_RX_RCC                                 RCC_AHB1Periph_GPIOB

/********接收状态机********/
#define TMC_CONFIG_RX_STATE_IDLE               0x00
#define TMC_CONFIG_RX_STATE_GOT_FRAME_HEAD     0x01        // 已经检测到帧头
#define TMC_CONFIG_RX_STATE_GOT_MASTER_ADDR     0x02     // 检测到mster地址
#define TMC_CONFIG_RX_STATE_WAIT_CHECK     0x03                //等待校验
#define TMC_CONFIG_RX_STATE_GOT_FRAME_ERROR    0x04        // 出现错误

#define TMC_CONFIG_FRAME_HEAD         0x05     // 帧头
#define TMC_CONFIG_MASTER_ADDR       0xFF    //Master地址

#define TMC_RECV_FRAME_LEN         8            //接收帧长度(带CRC)
#define TMC_CONFIG_TX_BUFF_SIZE  8             //发送缓存大小
#define TMC_CONFIG_RX_BUFF_SIZE  8             //接收缓存大小

/********缓存********/
static Uint8 s_RxBuffer[TMC_CONFIG_RX_BUFF_SIZE] = {0};
static Uint8 s_TxBuffer[TMC_CONFIG_TX_BUFF_SIZE] = {0};
static Uint8 s_TxBufferHead = 0;     // 发送缓冲区头指针,读指针
static Uint8 s_TxBufferTail = 0;     // 发送缓冲区尾指针,写指针
static Uint8 s_TxBufferLen = 0;      // 发送缓冲区未发送数据个数

/********消息队列********/
xQueueHandle g_TMCRespQueue = NULL;

/********Function********/
//static void TMCConfigDriver_PrintData(Uint8* buffer, Uint8 len);

/**
 * @brief UART初始化配置
 */
void TMCConfigDriver_UARTInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 配置中断向量管理器的USARTx_IRQn的中断
    NVIC_InitStructure.NVIC_IRQChannel = TMC_UART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TMC_UART_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    //打开引脚时钟,串口时钟
    RCC_APB1PeriphClockCmd(TMC_UART_RCC, ENABLE);
    RCC_AHB1PeriphClockCmd(TMC_UART_TX_RCC, ENABLE);
    RCC_AHB1PeriphClockCmd(TMC_UART_RX_RCC, ENABLE);

    //配置收发引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = TMC_UART_TX_PIN;
    GPIO_Init(TMC_UART_TX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = TMC_UART_RX_PIN;
    GPIO_Init(TMC_UART_RX_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(TMC_UART_TX_PORT, TMC_UART_TX_PinSource, TMC_UART_GPIO_AF);
    GPIO_PinAFConfig(TMC_UART_RX_PORT, TMC_UART_RX_PinSource, TMC_UART_GPIO_AF);

    USART_InitStructure.USART_BaudRate              = TMC_UART_BAUD;
    USART_InitStructure.USART_WordLength         = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits              = USART_StopBits_1;
    USART_InitStructure.USART_Parity                = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(TMC_UART, &USART_InitStructure);
    {
        uint16_t mantissa;
        uint16_t fraction;

        float temp = (float) (TMC_UART_CLK * 1000000) / (TMC_UART_BAUD * 16);
        mantissa = (uint16_t) temp;
        fraction = (uint16_t) ((temp - mantissa) * 16);
        mantissa <<= 4;
        mantissa += fraction;
        TMC_UART->BRR = mantissa;
    }
    USART_Cmd(TMC_UART, ENABLE);

    g_TMCRespQueue = xQueueCreate(1,  TMC_RECV_FRAME_LEN);

    USART_ClearFlag(TMC_UART, USART_FLAG_TC);
    USART_ITConfig(TMC_UART, USART_IT_RXNE, ENABLE);
}

/**
 * @brief 重置回应包消息
 */
void TMCConfigDriver_ClearResp(void)
{
        xQueueReset(g_TMCRespQueue);
}

/**
 * @brief 接收解析回应包消息
 */
Bool TMCConfigDriver_WaitRespData(TMC_Data* resp, Uint16 timeoutMS)
{
    Uint8 respBuffer[TMC_RECV_FRAME_LEN] = {0};
    Bool   waitResp = FALSE;

    if( pdTRUE == xQueueReceive(g_TMCRespQueue, respBuffer, portTICK_RATE_MS * timeoutMS ) )
    {
        resp->destAddr = respBuffer[1];
        resp->regAddr = respBuffer[2];

        resp->data = (respBuffer[3] << 24) | (respBuffer[4] << 16) | (respBuffer[5] << 8) | respBuffer[6];

        waitResp = TRUE;
    }
    ///Printf("\nRecv Data: ");
    ///TMCConfigDriver_PrintData(respBuffer, TMC_RECV_FRAME_LEN);

    return waitResp;
}

/**
 * @brief 发送数据到消息邮箱
 */
static void TMCConfigDriver_PostRespData(Uint8* respData)
{
    xQueueSendFromISR(g_TMCRespQueue, ( const Uint8* )respData, pdFALSE);
}

/**
 * @brief 接收数据状态机处理
 * @detail 中断调用
 */
static void TMCConfigDriver_ReceiveDataHandle(Uint8 data)
{
    static Uint8 s_RxState = TMC_CONFIG_RX_STATE_IDLE;
    static Uint8 s_RxBufferLen = 0;
    Uint8 check = 0;   //接收到的CRC码

    switch(s_RxState)
    {
        ///******** 空闲状态 ********
        case TMC_CONFIG_RX_STATE_IDLE:
            if((data & 0x0F) == TMC_CONFIG_FRAME_HEAD) // 如果是帧头
            {
                //改变状态机状态
                s_RxState = TMC_CONFIG_RX_STATE_GOT_FRAME_HEAD;

                //保存数据
                s_RxBuffer[s_RxBufferLen++] = data;
            }
            //其他数据，直接丢弃
            break;

        ///******** 已经找到帧头 ********
        case TMC_CONFIG_RX_STATE_GOT_FRAME_HEAD:
            if((data & 0xFF) == TMC_CONFIG_MASTER_ADDR) // 如果是Master地址
            {
                //改变状态机状态
                s_RxState = TMC_CONFIG_RX_STATE_GOT_MASTER_ADDR;

                //保存数据
                s_RxBuffer[s_RxBufferLen++] = data;
            }
            //其他数据，直接丢弃
            break;

        ///******** 已经找到Master地址 ********
        case TMC_CONFIG_RX_STATE_GOT_MASTER_ADDR:
            //保存数据
            s_RxBuffer[s_RxBufferLen++] = data;

            if(s_RxBufferLen >= TMC_RECV_FRAME_LEN - 1)  //等待校验码
            {
                s_RxState = TMC_CONFIG_RX_STATE_WAIT_CHECK;
            }
            break;

        /// ******** 接收校验和高字节 ********
        case TMC_CONFIG_RX_STATE_WAIT_CHECK:
        {
            check = data;
            TMCConfigDriver_CRC8(s_RxBuffer, TMC_RECV_FRAME_LEN);
            if(check == s_RxBuffer[s_RxBufferLen])
            {
                //通知处理
                TMCConfigDriver_PostRespData(s_RxBuffer);
            }
            else
            {
                TRACE_ERROR("\nError CRC");
                //CRC错误
            }

            // 改变状态机状态，恢复初始状态
            s_RxState = TMC_CONFIG_RX_STATE_IDLE;
            s_RxBufferLen = 0;
            memset((uint8_t*)s_RxBuffer, 0, sizeof(s_RxBuffer));
        }
            break;

        default :
            break;
    }
}

/**
 * @brief 发送数据
 * @detail 外部调用
 */
Uint32 TMCConfigDriver_WriteData(Uint8 *pTx_Buff, Uint8 len)
{
    Uint32 i = 0;
    Uint32 send_count = 0;

    // 参数检查
    if ((NULL == pTx_Buff) || (0 == len))
    {
        return 0;
    }

    ///Printf("\nSend Data: ");
    ///TMCConfigDriver_PrintData(pTx_Buff, len);

    while (send_count < len)
    {
        // 进入临界区
        taskENTER_CRITICAL();
        for (i = send_count; i < len; i++)
        {
            // 如果缓冲区已满
            if (s_TxBufferLen >= TMC_CONFIG_TX_BUFF_SIZE)
            {
                break;
            }

            // 写入缓冲区,修改缓冲区尾指针
            s_TxBuffer[s_TxBufferTail] = pTx_Buff[i];
            s_TxBufferLen++;
            s_TxBufferTail++;
            if (s_TxBufferTail >= TMC_CONFIG_TX_BUFF_SIZE)
            {
                s_TxBufferTail = 0;
            }
            send_count++;
        }
        // 退出临界区
        taskEXIT_CRITICAL();

        // 使能发送中断
        USART_ITConfig(TMC_UART, USART_IT_TXE, ENABLE);
    }

    return send_count;
}

/**
 * @brief 从本地发送缓存读出写入到串口发送缓冲区(中断调用)
 * @detail 发送单个字节
 */
static Bool TMCConfigDriver_ReadTxBuffer(Uint8 *pByte)
{
    // 参数检查
    if (NULL == pByte)
    {
        return FALSE;
    }

    // 如果可发送数据长度为0
    if (0 == s_TxBufferLen)
    {
        return FALSE;
    }

    // 读取数据,修改缓冲区头指针
    *pByte = s_TxBuffer[s_TxBufferHead];
    s_TxBufferLen--;
    s_TxBufferHead++;
    if (s_TxBufferHead >= TMC_CONFIG_TX_BUFF_SIZE)
    {
        s_TxBufferHead = 0;
    }

    return TRUE;
}

/**
 * @brief UART中断
 */
void TMC_UART_IRQHANDLER(void)
{
    static Uint8 s_byte_data = 0;

    // 接收处理
    if (USART_GetITStatus(TMC_UART, USART_IT_RXNE) != RESET)
    {
        // 读取数据
        s_byte_data = USART_ReceiveData(TMC_UART);

        // 对数据进行判断，对状态进行处理
        TMCConfigDriver_ReceiveDataHandle(s_byte_data);

        /* Clear the USART Receive interrupt */
        USART_ClearITPendingBit(TMC_UART, USART_IT_RXNE);
    }

    // 发送处理
    if (USART_GetITStatus(TMC_UART, USART_IT_TXE) != RESET)
    {
        /* Clear the USART transmit interrupt */
        // USART_ClearITPendingBit(RS232DRIVER_USARTx, USART_IT_TXE);   // 写操作会自动清除TXE中断标识

        // 从缓冲区中读取数据
        if(TMCConfigDriver_ReadTxBuffer(&s_byte_data))
        {
            /* Write one byte to the transmit data register */
            USART_SendData(TMC_UART, s_byte_data);
        }
        else //数据发送完毕
        {
            /* Disable the USART Transmit interrupt */
            USART_ITConfig(TMC_UART, USART_IT_TXE, DISABLE);
        }
    }

    // 若溢出错误
    if (USART_GetFlagStatus(TMC_UART, USART_FLAG_ORE) != RESET)
    {
        // 读取数据可清除溢出错误标志
        USART_ReceiveData(TMC_UART);

        /* Clear the USART Receive interrupt */
        USART_ClearITPendingBit(TMC_UART, USART_IT_RXNE);
    }
}

/**
 * @brief CRC-8
 * @detail CRC = x^8 + x^2 + x^1 + x^0
 */
void TMCConfigDriver_CRC8(unsigned char* datagram, unsigned char len)
{
    int i,j;
    unsigned char* crc = datagram + (len - 1);
    unsigned char curByte;

    *crc = 0;
    for(i = 0; i < len - 1; i++)
    {
        curByte = datagram[i];
        for(j = 0; j < 8; j++)
        {
            if((*crc >> 7) ^ (curByte&0x01))
            {
                *crc = (*crc << 1) ^ 0x07;
            }
            else
            {
                *crc = (*crc << 1);
            }
            curByte = curByte >> 1;
        }
    }
}

//void TMCConfigDriver_PrintData(Uint8* buffer, Uint8 len)
//{
//    for(Uint8 i = 0; i < len; i++)
//    {
//        Printf(" %02X", buffer[i]);
//    }
//    Printf("\n");
//}

