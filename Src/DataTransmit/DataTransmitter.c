/*
 * DataTransmitter.c
 *
 *  Created on: 2020年3月17日
 *      Author: Administrator
 */

#include "DncpStack/DncpStack.h"
#include "DataTransmit/DataTransmitter.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"
#include "SystemConfig.h"
#include "LuipApi/DataTransmitInterface.h"
#include <string.h>

#define MODBUS_FRAME_RECV_TIMEOUT_MS  30
#define MODBUS_FRAME_SEND_TIMEOUT_MS  30
#define MODBUS_TASK__INTERVAL_TIMEOUT_MS  30

#define MAX_EVENT_PACK_DATA_LENGTH   240
#define UART_DATA_NUM   8
Uint16 Uart_Date_Buff[UART_DATA_NUM] = {0}; 

/******Recv Task Handle******/
static Bool DataTransmitter_RecvDataFSM(void* transmitter);
static void DataTransmitter_RecvTaskHandle(void* argument);

/******Send Task Handle******/
static Bool DataTransmitter_SendDataFSM(void* transmitter);
static void DataTransmitter_SendTaskHandle(void* argument);

/******SerialPort Control******/
static void DataTransmitter_SerialEnable(DataTransmitter* transmitter, Bool rxEnable, Bool txEnable);
static void DataTransmitter_SerialRxEnable(DataTransmitter* transmitter, Bool rxEnable);
static void DataTransmitter_SerialTxEnable(DataTransmitter* transmitter, Bool txEnable);
static void DataTransmitter_SwitchToRx(DataTransmitter* transmitter);
static void DataTransmitter_SwitchToTx(DataTransmitter* transmitter);
static Bool DataTransmitter_SerialGetByte(DataTransmitter* transmitter, char* ch);
static Bool DataTransmitter_SerialPutByte(DataTransmitter* transmitter, char ch);

/******Event Handle******/
static Bool DataTransmitter_EventPost(QueueHandle_t queue, TransmitterFrameEvent eEvent);
static Bool DataTransmitter_EventPostFromISR(QueueHandle_t queue, TransmitterFrameEvent eEvent);
static Bool DataTransmitter_EventGet(QueueHandle_t queue, TransmitterFrameEvent* peEvent, Uint32 timeout );

/******DNCP Handle******/
static void DataTransmitter_UploadData(DataTransmitter* transmitter);
/******Local Function******/
//static void DataTransmitter_PrintData(Uint8* buffer, Uint8 len);

/**
 * @brief 初始化
 * @details 创建任务定时器队列，注册函数等
 */
void DataTransmitter_Init(DataTransmitter* transmitter)
{
    xTaskCreate(DataTransmitter_SendTaskHandle, (const char*)transmitter->txTaskName,
            UART_DATATRANSMITTER_SEND_STK_SIZE, (void *)transmitter,
            UART_DATATRANSMITTER_SEND_TASK_PRIO, &transmitter->sendTaskHandle);

    xTaskCreate(DataTransmitter_RecvTaskHandle, (const char*)transmitter->rxTaskName,
            UART_DATATRANSMITTER_RECV_STK_SIZE, (void *)transmitter,
            UART_DATATRANSMITTER_RECV_TASK_PRIO, &transmitter->recvTaskHandle);

    transmitter->sendFunc = DataTransmitter_SendDataFSM;
    transmitter->recvFunc = DataTransmitter_RecvDataFSM;

    transmitter->sendFrameEvent = Transmitter_Frame_NoEvent;
    transmitter->sendEventQueue = xQueueCreate(1, sizeof(TransmitterFrameEvent));

    transmitter->recvFrameEvent = Transmitter_Frame_NoEvent;
    transmitter->recvEventQueue = xQueueCreate(1, sizeof(TransmitterFrameEvent));

    transmitter->sendStatus =Transmitter_Init;
    transmitter->isSendable = TRUE;

    transmitter->recvStatus =Transmitter_Init;

    transmitter->sendBufferLen = 0;
    transmitter->sendBufferPos = 0;
    transmitter->sendState = Transmitter_Send_Idle;
    memset(transmitter->sendBuffer, 0, MAX_TRANSMITTER_SEND_BUFFER_LEN*sizeof(Uint8));

    transmitter->recvBufferLen = 0;
    transmitter->recvBufferPos = 0;
    transmitter->recvState = Transmitter_Recv_Idle;
    memset(transmitter->recvBuffer, 0, MAX_TRANSMITTER_RECV_BUFFER_LEN*sizeof(Uint8));

    UartDriver_Init(&transmitter->uartConfig, transmitter->type);

    ///TRACE_INFO("\nDataTransmitter %d Init Over", transmitter->index);
}

/**
 * @brief 重新初始化
 * @details 数据状态清空
 */
void DataTransmitter_Reset(DataTransmitter* transmitter)
{
    transmitter->sendFrameEvent = Transmitter_Frame_NoEvent;
    xQueueReset(transmitter->sendEventQueue);

    transmitter->sendStatus =Transmitter_Init;
    transmitter->isSendable = TRUE;

    transmitter->recvStatus =Transmitter_Init;

    transmitter->sendBufferLen = 0;
    transmitter->sendBufferPos = 0;
    transmitter->sendState = Transmitter_Send_Idle;
    memset(transmitter->sendBuffer, 0, MAX_TRANSMITTER_SEND_BUFFER_LEN*sizeof(Uint8));

    transmitter->recvBufferLen = 0;
    transmitter->recvBufferPos = 0;
    transmitter->recvState = Transmitter_Recv_Idle;
    memset(transmitter->recvBuffer, 0, MAX_TRANSMITTER_RECV_BUFFER_LEN*sizeof(Uint8));

    UartDriver_Init(&transmitter->uartConfig, transmitter->type);

    TRACE_INFO("\nDataTransmitter %d Reset Over", transmitter->index);
}

/**
 * @brief DNCP发送一串数据请求
 * @details 上层调用
 */
Bool DataTransmitter_SendData(DataTransmitter* transmitter, Uint8* data, Uint16 len)
{
    Bool ret = FALSE;
    if(transmitter->sendStatus == Transmitter_Idle && transmitter->isSendable)
    {
        ///taskENTER_CRITICAL();

        memcpy(transmitter->sendBuffer, data, len);
        transmitter->sendBufferLen = len;
        transmitter->sendBufferPos = 0;

        ///taskEXIT_CRITICAL();

        DataTransmitter_EventPost(transmitter->sendEventQueue, Transmitter_Frame_SendRequestEvent);
        ret = TRUE;
    }
    else
    {
        TRACE_ERROR("\nTrasmitter %d send data fail, its status is %d", transmitter->index, transmitter->sendStatus);
    }

    return ret;
}

/**
 * @brief 向DNCP上层报告收到的数据包
 * @details 本地任务调用
 */
void DataTransmitter_UploadData(DataTransmitter* transmitter)
{
    Uint8 data[MAX_EVENT_PACK_DATA_LENGTH] = {0};
    Uint16 len = 0;
    Uint16 index = transmitter->index;

    ///taskENTER_CRITICAL();

    memcpy(&data[0], &index, sizeof(Uint16));
    memcpy(&data[0]+sizeof(Uint16), &transmitter->recvBufferLen, sizeof(Uint16));
    memcpy(&data[0]+2*sizeof(Uint16), &transmitter->recvBuffer, transmitter->recvBufferLen);
    len = 2 + 2 + transmitter->recvBufferLen;

    transmitter->recvBufferLen = 0;
    transmitter->recvBufferPos = 0;
    memset(transmitter->recvBuffer, 0, sizeof(transmitter->recvBuffer));

    ///taskEXIT_CRITICAL();

    DncpStack_SendEvent(DSCP_EVENT_DTI_UPLOAD_DATA, data, len);
}

/**
 * @brief 检查是否空闲
 * @details 上层调用
 */
Bool DataTransmitter_IsSendable(DataTransmitter* transmitter)
{
    return transmitter->isSendable;
}

/**
 * @brief 从串口缓冲中读取一个字节
 * @details 串口中断调用
 */
Bool DataTransmitter_SerialGetByte(DataTransmitter* transmitter, char* ch)
{
    *ch = USART_ReceiveData(transmitter->uartConfig.uart);

    return TRUE;
}

/**
 * @brief 将字节写入到串口缓冲中
 * @details 串口中断调用
 */
Bool DataTransmitter_SerialPutByte(DataTransmitter* transmitter, char ch)
{
    //TRACE_INFO("%02X ",ch);
    USART_SendData(transmitter->uartConfig.uart, ch);

    return TRUE;
}

/**
 * @brief 接收字节状态机
 * @details 串口中断回调
 */
Bool DataTransmitter_RecvDataFSM(void* argument)
{
    DataTransmitter* transmitter = (DataTransmitter*)argument;
    Uint8 index = transmitter->index;
    static char ch;

    DataTransmitter_SerialGetByte(transmitter, &ch);
    TRACE_CODE("<%02x ", ch);
    transmitter->recvTime = xTaskGetTickCount();

    switch(transmitter->recvState)
    {
        case Transmitter_Recv_Idle:    //开始接收
            TRACE_DEBUG("\nDT %d RxFSM => Recv_Idle", index);
            transmitter->recvBufferPos = 0;
            transmitter->recvBufferLen = 0;
            transmitter->recvBuffer[transmitter->recvBufferPos++] = ch;
            transmitter->recvBufferLen++;
            transmitter->recvState = Transmitter_Recv_Busy;
            TRACE_DEBUG("\nDT %d RxFSM => Recv_Busy", index);

            DataTransmitter_EventPostFromISR(transmitter->recvEventQueue, Transmitter_Frame_RecvingEvent);
            break;
        case Transmitter_Recv_Busy:  //正在接收
            if( transmitter->recvBufferPos < MAX_EVENT_PACK_DATA_LENGTH )
            {
                transmitter->recvBuffer[transmitter->recvBufferPos++]  = ch;
                transmitter->recvBufferLen++;
                DataTransmitter_EventPostFromISR(transmitter->recvEventQueue, Transmitter_Frame_RecvingEvent);
            }
            else
            {
                transmitter->recvState = Transmitter_Recv_Error;
            }
            break;
        case Transmitter_Recv_Error:
            TRACE_WARN("\nDT %d RxFSM => Recv_Error", index);
            DataTransmitter_EventPostFromISR(transmitter->recvEventQueue, Transmitter_Frame_RecvedFullEvent);
            break;
        default:
            break;
    }

    return FALSE;
}

/**
 * @brief 发送字节状态机
 * @details 串口中断回调
 */
Bool DataTransmitter_SendDataFSM(void* argument)
{
    DataTransmitter* transmitter = (DataTransmitter*)argument;
    Uint8 index = transmitter->index;
    Bool bTaskWoken = FALSE;

    switch(transmitter->sendState)
    {
        case Transmitter_Send_Idle:
            DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
            break;
        case Transmitter_Send_Busy:  //正在发送
            if(transmitter->sendBufferLen != 0)
            {
                DataTransmitter_SerialPutByte(transmitter, transmitter->sendBuffer[transmitter->sendBufferPos]);
                transmitter->sendBufferPos++;
                transmitter->sendBufferLen--;
            }
            else
            {
                transmitter->sendState = Transmitter_Send_Over;
                TRACE_DEBUG("\nDT %d TxFSM => Send_Over", index);
            }
            break;
        case Transmitter_Send_Over:   //发送完毕   //切换到接收状态
            ///DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
            transmitter->isSendable = TRUE;
            transmitter->sendState = Transmitter_Send_Idle;
            TRACE_DEBUG("\nDT %d TxFSM => Send_Idle", index);
            memset(transmitter->sendBuffer, 0, sizeof(transmitter->sendBuffer));
            transmitter->sendBufferLen = 0;
            transmitter->sendBufferPos = 0;
            bTaskWoken = DataTransmitter_EventPostFromISR( transmitter->sendEventQueue, Transmitter_Frame_SendOverEvent);
            break;
        default:
            break;
    }

    return bTaskWoken;
}


/**
 * @brief 串口发送任务
 */
void DataTransmitter_SendTaskHandle(void* argument)
{
    DataTransmitter* transmitter = (DataTransmitter *)argument;
    Uint8 index = transmitter->index;
    ///vTaskSuspend(NULL);
    while(1)
    {
        switch(transmitter->sendStatus)
        {
            case Transmitter_Init:
                TRACE_DEBUG("\nDT %d TxState => Init", index);
                transmitter->sendStatus = Transmitter_Idle;
                ///DataTransmitter_SwitchToRx(transmitter);
                ///DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
                break;
            case Transmitter_Idle:
                if(TRUE == DataTransmitter_EventGet(transmitter->sendEventQueue, &transmitter->sendFrameEvent, MODBUS_TASK__INTERVAL_TIMEOUT_MS))
                {
                    switch(transmitter->sendFrameEvent)
                    {
                        case Transmitter_Frame_NoEvent:
                            break;
                        case Transmitter_Frame_SendRequestEvent:
                            TRACE_DEBUG("\nDT %d <= SendRequestEvent", index);
                            transmitter->sendStatus = Transmitter_Send;
                            TRACE_DEBUG("\nDT %d TxState => Send", index);
                            break;
                        default:
                            break;
                    }
                }
                break;
            case Transmitter_Send:
                //TRACE_INFO("\nDT %d Send >>", index);
                //DataTransmitter_PrintData(transmitter->sendBuffer, transmitter->sendBufferLen);
                transmitter->sendState = Transmitter_Send_Busy;
                TRACE_DEBUG("\nDT %d TxFSM => Send_Busy", index);
                transmitter->isSendable = FALSE;
                DataTransmitter_SerialTxEnable(transmitter, TRUE);  ///DataTransmitter_SerialEnable(transmitter, FALSE, TRUE);
                if(TRUE == DataTransmitter_EventGet(transmitter->sendEventQueue, &transmitter->sendFrameEvent, MODBUS_FRAME_SEND_TIMEOUT_MS))
                {
                    if(transmitter->sendFrameEvent == Transmitter_Frame_SendOverEvent)
                    {
                        TRACE_DEBUG("\nDT %d <= SendOverEvent", index);
                        System_Delay(3);
                        DataTransmitter_SwitchToRx(transmitter);
                        DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
                        transmitter->sendStatus = Transmitter_Idle;
                        TRACE_DEBUG("\nDT %d TxState => Idle", index);
                    }
                    else
                    {
                        TRACE_WARN("\nInvalid Event = %d", transmitter->sendFrameEvent);
                    }
                }
                break;
            default:
                vTaskSuspend(NULL);
                break;
        }
        System_Delay(1);
    }
}


//modbus计算16位crc
Uint16 calcrc16(Uint8 *p, Uint8 len)
{
    Uint8 i,j,temp;
    Uint16 wcrc=0xffff;
    for (i = 0; i < len; i++)       //循环计算每个数据
    {
        temp = *p & 0X00FF;         //将八位数据与crc寄存器亦或
        p++;                        //指针地址增加，指向下个数据
        wcrc ^= temp;               //将数据存入crc寄存器
        for (j = 0; j < 8; j++)     //循环计算数据的
        {
            if (wcrc & 0X0001)      //判断右移出的是不是1，如果是1则与多项式进行异或。
            {
                wcrc >>= 1;         //先将数据右移一位
                wcrc ^= 0XA001;     //与上面的多项式进行异或
            }
            else                    //如果不是1，则直接移出
            {
                wcrc >>= 1;         //直接移出
            }
        }
    }
    //高低字节交换
    return (wcrc<<8)+(wcrc>>8);
}

//数据解析
void DataTransmitManager_RevData(DataTransmitter* transmitter)
{
    int i = 0;
    Uint16 Temp = 0, crc = 0xffff;

    if(Transmitter_Idle == transmitter->recvStatus)  
    {
        if(transmitter->recvBufferLen == 42)
        {
            crc = calcrc16(transmitter->recvBuffer, transmitter->recvBufferLen-2);
            Temp = transmitter->recvBuffer[transmitter->recvBufferLen-2];
            Temp <<=8;
            Temp += transmitter->recvBuffer[transmitter->recvBufferLen-1];
            if(crc == Temp)
            {
                for(i=0;i<UART_DATA_NUM;i++)
                {
                    Temp = transmitter->recvBuffer[i*2+3];
                    Temp <<=8;
                    Temp += transmitter->recvBuffer[i*2+1+3];
                    Temp -= 30000;
                    Uart_Date_Buff[i] = Temp;
                }
                DncpStack_SendEvent(DSCP_EVENT_DTI_UPLOAD_DATA, Uart_Date_Buff, 8*sizeof(Uint16));
            } 
        }
    }
    return ;
}

/**
 * @brief 串口接收任务
 */
void DataTransmitter_RecvTaskHandle(void* argument)
{
    DataTransmitter* transmitter = (DataTransmitter *)argument;
    Uint8 index = transmitter->index;
    ///vTaskSuspend(NULL);
    while(1)
    {
        switch(transmitter->recvStatus)
        {
            case Transmitter_Init:
                TRACE_DEBUG("\nDT %d RxState => Init", index);
                transmitter->recvStatus = Transmitter_Idle;
                DataTransmitter_SwitchToRx(transmitter);
                DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
                break;
            case Transmitter_Idle:
                if(TRUE == DataTransmitter_EventGet(transmitter->recvEventQueue, &transmitter->recvFrameEvent, MODBUS_TASK__INTERVAL_TIMEOUT_MS))
                {
                    switch(transmitter->recvFrameEvent)
                    {
                        case Transmitter_Frame_NoEvent:
                            break;
                            break;
                        case Transmitter_Frame_RecvingEvent:
                            TRACE_DEBUG("\nDT %d <= RecvingEvent", index);
                            transmitter->recvStatus = Transmitter_Recving;
                            TRACE_DEBUG("\nDT %d RxState => Recving", index);
                            break;
                        default:
                            break;
                    }
                }
                break;
            case Transmitter_Recving:
                //超时无数据
                if(FALSE == DataTransmitter_EventGet(transmitter->recvEventQueue, &transmitter->recvFrameEvent, MODBUS_FRAME_RECV_TIMEOUT_MS))
                {
                    DataTransmitter_SerialRxEnable(transmitter, FALSE);
                    transmitter->recvState = Transmitter_Recv_Idle;
                    transmitter->recvStatus = Transmitter_Recved;
                    TRACE_DEBUG("\nDT %d RxState => Recved !", index);
                }
                else
                {
                    if(transmitter->recvFrameEvent == Transmitter_Frame_RecvingEvent)
                    {
                        //接收中
                    }
                    else if(transmitter->recvFrameEvent == Transmitter_Frame_RecvedFullEvent)
                    {
                        DataTransmitter_SerialRxEnable(transmitter, FALSE);
                        transmitter->recvState = Transmitter_Recv_Idle;
                        transmitter->recvStatus = Transmitter_Recved;
                        TRACE_DEBUG("\nDT %d RxState => Recved Full", index);
                        //DataTransmitManager_RevData(transmitter);
                    }
                    else
                    {
                        DataTransmitter_SerialRxEnable(transmitter, FALSE);
                        transmitter->recvState = Transmitter_Recv_Idle;
                        transmitter->recvStatus = Transmitter_Idle;
                        TRACE_DEBUG("\nDT %d RxState => Idle", index);
                        DataTransmitter_SwitchToRx(transmitter);
                        DataTransmitter_SerialRxEnable(transmitter, TRUE);  ///DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
                    }
                }
                break;
            case Transmitter_Recved:
                //TRACE_INFO("\nDT %d Recv <<", index);
                //DataTransmitter_PrintData(transmitter->recvBuffer, transmitter->recvBufferLen);
            
                DataTransmitter_UploadData(transmitter);
                transmitter->recvStatus = Transmitter_Idle;
                TRACE_DEBUG("\nDT %d RxState => Idle", index);
                DataTransmitter_SwitchToRx(transmitter);
                DataTransmitter_SerialRxEnable(transmitter, TRUE);  ///DataTransmitter_SerialEnable(transmitter, TRUE, FALSE);
                break;
            default:
                vTaskSuspend(NULL);
                break;
        }
        System_Delay(1);
    }
}

/**
 * @brief 串口收/发功能使能管理
 * @param transmitter DataTransmitter*，传输器指针
 * @param rxEnable Bool，接收使能
 * @param txEnable Bool，发送使能
 */
void DataTransmitter_SerialEnable(DataTransmitter* transmitter, Bool rxEnable, Bool txEnable)
{
    if(rxEnable)
    {
        ///DataTransmitter_SwitchToRx(transmitter);

        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_RXNE, ENABLE); // 使能接收中断
    }
    else
    {
        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_RXNE, DISABLE);  // 除能接收中断
    }

    if(txEnable)
    {
        DataTransmitter_SwitchToTx(transmitter);

        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TXE, ENABLE);    // 使能发送缓冲区空中断
        ///USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TC, DISABLE);    // 除能发送完成中断
    }
    else
    {
        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TXE, DISABLE);   // 除能发送缓冲区空中断
        ///USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TC, ENABLE);     // 使能发送完成中断
    }
}

/**
 * @brief 串口接收使能控制
 */
void DataTransmitter_SerialRxEnable(DataTransmitter* transmitter, Bool rxEnable)
{
    if(rxEnable)
    {
        ///DataTransmitter_SwitchToRx(transmitter);

        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_RXNE, ENABLE); // 使能接收中断
    }
    else
    {
        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_RXNE, DISABLE);  // 除能接收中断
    }
}

/**
 * @brief 串口发送使能控制
 */
void DataTransmitter_SerialTxEnable(DataTransmitter* transmitter, Bool txEnable)
{
    if(txEnable)
    {
        DataTransmitter_SwitchToTx(transmitter);

        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TXE, ENABLE);    // 使能发送缓冲区空中断
        ///USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TC, DISABLE);    // 除能发送完成中断
    }
    else
    {
        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TXE, DISABLE);   // 除能发送缓冲区空中断
        ///USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TC, ENABLE);     // 使能发送完成中断
    }
}

/**
 * @brief RS485切换到接收状态
 */
void DataTransmitter_SwitchToRx(DataTransmitter* transmitter)
{
    if(transmitter->type == RS485)
    {
        ///Printf("\nRS485->RX");
        UartDriver_RS485_SwitchToRx(&transmitter->uartConfig);
    }
}

/**
 * @brief RS485切换到发送状态
 */
void DataTransmitter_SwitchToTx(DataTransmitter* transmitter)
{
    if(transmitter->type == RS485)
    {
        ///Printf("\nRS485->TX");
        UartDriver_RS485_SwitchToTx(&transmitter->uartConfig);
    }
}

/**
 * @brief 帧队列事件接口
 */
Bool DataTransmitter_EventPost(QueueHandle_t queue, TransmitterFrameEvent eEvent)
{
    Bool  bStatus = TRUE;

    xQueueSend(queue, ( const void * )&eEvent, pdFALSE );

    return bStatus;
}

/**
 * @brief 帧队列事件接口
 */
Bool DataTransmitter_EventPostFromISR(QueueHandle_t queue, TransmitterFrameEvent eEvent)
{
    Bool  bStatus = TRUE;

    xQueueSendFromISR( queue, ( const void * )&eEvent, pdFALSE);

    return bStatus;
}


/**
 * @brief 获取帧队列事件
 * @param queue QueueHandle_t，指定消息队列
 * @param peEvent TransmitterFrameEvent*，事件缓存指针
 * @param timeout Uint32，超时时间，单位ms
 */
Bool DataTransmitter_EventGet(QueueHandle_t queue, TransmitterFrameEvent* peEvent, Uint32 timeout )
{
    Bool   eventHappened = FALSE;

    if( pdTRUE == xQueueReceive(queue, peEvent, portTICK_RATE_MS * timeout ) )
    {
        eventHappened = TRUE;
    }
    return eventHappened;
}

/**
 * @brief 打印原始字符
 */
void DataTransmitter_PrintData(Uint8* buffer, Uint8 len)
{
    for(Uint8 i = 0; i < len; i++)
    {
        TRACE_INFO(" %02X", buffer[i]);
    }
    TRACE_INFO("\n");
}

static short HexChar2Dec(char c)
{
    if ( '0'<=c && c<='9' )
    {
        return (c-'0');
    }
    else if ( 'a'<=c && c<='f' )
    {
        return (c-'a') + 10;
    }
    else if ( 'A'<=c && c<='F' )
    {
        return (c-'A') + 10 ;
    }
    else
    {
        return -1;
    }
}

static int Str2Num16(const char* str)
{
  return (str[1] == '\0') ? HexChar2Dec(str[0]) : HexChar2Dec(str[0])*16+HexChar2Dec(str[1]);
}

int DataTransmitter_HexStringToCharArray(char* hexStr, char* data)
{
    int len = (strlen(hexStr)%2 == 0) ? strlen(hexStr)/2 : (strlen(hexStr)/2+1);
    for (int i = 0; i < len; ++i)
    {
        int idx = i*2;
        *(data++) = Str2Num16(hexStr+idx);
    }

    return len;
}
