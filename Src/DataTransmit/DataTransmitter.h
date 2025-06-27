/*
 * DataTransmitter.h
 *
 *  Created on: 2020年3月17日
 *      Author: Administrator
 */

#ifndef SRC_DATATRANSMIT_DATATRANSMITTER_H_
#define SRC_DATATRANSMIT_DATATRANSMITTER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"
#include "LuipApi/DataTransmitInterface.h"
#include "Driver/UartDriver/UartDriver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TRANSMITTER_SEND_BUFFER_LEN     512
#define MAX_TRANSMITTER_RECV_BUFFER_LEN     512

typedef enum
{
    Transmitter_Frame_NoEvent = 0,
    Transmitter_Frame_SendRequestEvent = 1,
    Transmitter_Frame_SendOverEvent = 2,
    Transmitter_Frame_RecvingEvent = 3,
    Transmitter_Frame_RecvedFullEvent = 4,
}TransmitterFrameEvent;

typedef enum
{
    Transmitter_Init = 0,
    Transmitter_Idle = 1,
    Transmitter_Send = 2,
    Transmitter_Recving = 3,
    Transmitter_Recved = 4,
}TransmitterStatus;

typedef enum
{
    Transmitter_Send_Idle = 0,
    Transmitter_Send_Busy = 1,
    Transmitter_Send_Over = 2,
}TransmitterSendState;

typedef enum
{
    Transmitter_Recv_Idle = 0,
    Transmitter_Recv_Busy = 1,
    Transmitter_Recv_Error = 2,
}TransmitterRecvState;

typedef void (*UartIntFunc)(void*);
typedef Bool (*RecvByteFunc)(void*);
typedef Bool (*SendByteFunc)(void*);

typedef struct
{
    Uint8 index;
    SerialPortType type;
    Uint32 eventCode;
    char txTaskName[10];
    char rxTaskName[10];

    UartConfigDef uartConfig;

    Bool isSendable;
    Uint8 sendBuffer[MAX_TRANSMITTER_SEND_BUFFER_LEN];
    Uint16 sendBufferLen;
    volatile Uint16 sendBufferPos;
    TransmitterSendState sendState;

    Uint8 recvBuffer[MAX_TRANSMITTER_RECV_BUFFER_LEN];
    Uint16 recvBufferLen;
    volatile Uint16 recvBufferPos;
    TransmitterRecvState recvState;
    TickType_t recvTime;

    TransmitterStatus sendStatus;
    TaskHandle_t sendTaskHandle;
    SendByteFunc sendFunc;
    TransmitterFrameEvent sendFrameEvent;
    QueueHandle_t sendEventQueue;

    TransmitterStatus recvStatus;
    TaskHandle_t recvTaskHandle;
    RecvByteFunc recvFunc;
    TransmitterFrameEvent recvFrameEvent;
    QueueHandle_t recvEventQueue;
}DataTransmitter;

void DataTransmitter_Init(DataTransmitter* transmitter);
void DataTransmitter_Reset(DataTransmitter* transmitter);
Bool DataTransmitter_SendData(DataTransmitter* transmitter, Uint8* data, Uint16 len);
Bool DataTransmitter_IsSendable(DataTransmitter* transmitter);
void DataTransmitter_PrintData(Uint8* buffer, Uint8 len);
int DataTransmitter_HexStringToCharArray(char* hexStr, char* data);

#ifdef __cplusplus
}
#endif


#endif /* SRC_DATATRANSMIT_DATATRANSMITTER_H_ */
