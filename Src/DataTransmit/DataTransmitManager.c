/*
 * DataTransmitManager.c
 *
 *  Created on: 2020年3月17日
 *      Author: Administrator
 */

#include <DataTransmit/DataTransmitManager.h>
#include <DataTransmit/DataTransmitter.h>
#include "Tracer/Trace.h"
#include "Driver/System.h"
#include "Driver/McuFlash.h"
#include "Driver/UartDriver/UartDriver.h"
#include "SystemConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>

#define TRANSMIT_UART_NUM   1
#define TRANSMITTER_1_IRQHANDLER    UART8_IRQHandler
#define UART_PARAM_SIZE  7
#define UART_ZEOR_NUM    9


static const SerialPortParam k_defaultPortParam = {.baud = 9600, .wordLen = 8, .stopBits = StopBits_1, .parity = Parity_No};

static DataTransmitter g_transmitters[TRANSMIT_UART_NUM];

static void DataTransmitManager_SendTesk(void* index)
{
    Uint8 SendData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x08, 0x44, 0x0C};
    while(1)
    {
        vTaskDelay(2000); //2秒
        DataTransmitManager_SendData(0, SendData, 8);
    }
}

void DataTransmitManager_Init(void)
{
    DataTransmitManager_Config();

    for(Uint8 index =0; index < TRANSMIT_UART_NUM; index++)
    {
        DataTransmitter_Init(&g_transmitters[index]);
    }
    xTaskCreate(DataTransmitManager_SendTesk, "SendTesk", 128, NULL, 5, NULL);
}

void DataTransmitManager_PortReset(Uint8 index)
{
    if(index < TRANSMIT_UART_NUM)
    {
        DataTransmitter_Reset(&g_transmitters[index]);
    }
    else
    {
        TRACE_ERROR("\nInvalid transmitter index %d", index);
    }
}

Bool DataTransmitManager_GetPortParam(Uint8 index , SerialPortParam* param)
{
    Uint8 readData[TRANSMITTER_PARAM_FLASH_LEN] = { 0 };

    if(index < TRANSMIT_UART_NUM)
    {
        McuFlash_Read(TRANSMITTER_PARAM_FLASH_BASE_ADDR + TRANSMITTER_PARAM_FLASH_LEN * index, TRANSMITTER_PARAM_FLASH_LEN, readData);
        memcpy(param, readData, sizeof(SerialPortParam));

        return TRUE;
    }
    else
    {
        TRACE_ERROR("\nInvalid transmitter index %d", index);
        return FALSE;
    }
}

Bool DataTransmitManager_CheckIndex(Uint8 index)
{
    if(index < TRANSMIT_UART_NUM)
    {
    	return TRUE;
    }
    else
    {
        TRACE_ERROR("\nInvalid transmitter index %d", index);
        return FALSE;
    }
}

Bool DataTransmitManager_SetPortParam(Uint8 index, SerialPortParam param)
{
    Uint8 writeData[TRANSMITTER_PARAM_FLASH_LEN] = { 0 };

    memcpy(writeData, &param, sizeof(SerialPortParam));

    McuFlash_Write(TRANSMITTER_PARAM_FLASH_BASE_ADDR + TRANSMITTER_PARAM_FLASH_LEN * index, TRANSMITTER_PARAM_FLASH_LEN, writeData);

    g_transmitters[index].uartConfig.portParam = param;

    TRACE_INFO("\nDT %d set param {baud = %d, wordLen = %d, stopBits = %d, parity = %d}", index, param.baud, param.wordLen, (Uint8)param.stopBits, (Uint8)param.parity);

    return TRUE;
}

Bool DataTransmitManager_SetAllPortParam(Uint16 size,SerialPortParam* param)
{
	Uint16 num = 0;               //偏移量
    Uint8* writeData = NULL;
    Uint16 len = size*16;
    writeData = (Uint8 *)malloc(len * sizeof(Uint8));
    for (int i=0; i<size; i++)
    {
    	num =16*i;
    	memcpy(writeData+num, param+i, sizeof(SerialPortParam));
    }
    McuFlash_Write(TRANSMITTER_PARAM_FLASH_BASE_ADDR, size*16, writeData);
    for(int u=0; u<size; u++)
    {
    	g_transmitters[u].uartConfig.portParam = param[u];
    	Printf("\nDT %d {baud = %d, wordLen = %d, stopBits = %d, parity = %d}", u, param[u].baud, param[u].wordLen, (Uint8)param[u].stopBits, (Uint8)param[u].parity);
    }
    free(writeData);
    return TRUE;
}

static void DataTransmitManager_InitParam(Uint8 index, SerialPortParam defaultParam)
{
    Uint8 buffer[TRANSMITTER_SIGN_FLASH_LEN] = { 0 };
    Uint32 flashFactorySign = 0;

    McuFlash_Read(TRANSMITTER_SIGN_FLASH_BASE_ADDR + TRANSMITTER_SIGN_FLASH_LEN * index, TRANSMITTER_SIGN_FLASH_LEN, buffer);                //读取出厂标志位
    memcpy(&flashFactorySign, buffer, TRANSMITTER_SIGN_FLASH_LEN);

    if (FLASH_FACTORY_SIGN == flashFactorySign)                      //表示已经过出厂设置
    {
        DataTransmitManager_GetPortParam(index, &g_transmitters[index].uartConfig.portParam);
    }
    else     //未设置,使用默认值，并写入出厂标志
    {
        DataTransmitManager_SetPortParam(index, defaultParam);

        flashFactorySign = FLASH_FACTORY_SIGN;
        memcpy(buffer, &flashFactorySign, TRANSMITTER_SIGN_FLASH_LEN);
        McuFlash_Write(TRANSMITTER_SIGN_FLASH_BASE_ADDR + TRANSMITTER_SIGN_FLASH_LEN * index, TRANSMITTER_SIGN_FLASH_LEN, buffer);
    }
}

void DataTransmitManager_Config(void)
{
    ///**************************<DataTransmitter1-RS485->**************************///
    g_transmitters[0].index = 0;
    g_transmitters[0].type = RS485;
    g_transmitters[0].eventCode = DSCP_EVENT_DTI_UPLOAD_DATA;
    strcpy(g_transmitters[0].txTaskName, "DT1Send");
    strcpy(g_transmitters[0].rxTaskName, "DT1Recv");

    ///g_transmitters[0].uartConfig.portParam = k_defaultPortParam;
    DataTransmitManager_InitParam(0, k_defaultPortParam);

    g_transmitters[0].uartConfig.uart = UART8;
    g_transmitters[0].uartConfig.uartRcc = RCC_APB1Periph_UART8;
    g_transmitters[0].uartConfig.uartRccCmd = RCC_APB1PeriphClockCmd;
    g_transmitters[0].uartConfig.clk = CLK_APB1;
    g_transmitters[0].uartConfig.afMap = GPIO_AF_UART8;
    g_transmitters[0].uartConfig.irq = UART8_IRQn;

    g_transmitters[0].uartConfig.txPin = GPIO_Pin_1;
    g_transmitters[0].uartConfig.txPort = GPIOE;
    g_transmitters[0].uartConfig.txPinSource = GPIO_PinSource1;
    g_transmitters[0].uartConfig.txRcc = RCC_AHB1Periph_GPIOE;
    g_transmitters[0].uartConfig.txRccCmd = RCC_AHB1PeriphClockCmd;

    g_transmitters[0].uartConfig.rxPin = GPIO_Pin_0;
    g_transmitters[0].uartConfig.rxPort = GPIOE;
    g_transmitters[0].uartConfig.rxPinSource = GPIO_PinSource0;
    g_transmitters[0].uartConfig.rxRcc = RCC_AHB1Periph_GPIOE;
    g_transmitters[0].uartConfig.rxRccCmd = RCC_AHB1PeriphClockCmd;

    g_transmitters[0].uartConfig.swPin = GPIO_Pin_2;
    g_transmitters[0].uartConfig.swPort = GPIOE;
    g_transmitters[0].uartConfig.swPinSource = GPIO_PinSource2;
    g_transmitters[0].uartConfig.swRcc = RCC_AHB1Periph_GPIOE;
    g_transmitters[0].uartConfig.swRccCmd = RCC_AHB1PeriphClockCmd;
}

Bool DataTransmitManager_SendData(Uint8 index, Uint8* data, Uint16 len)
{
    //TRACE_INFO("\nDataTransmitterManager Send Data...");
    if(index < TRANSMIT_UART_NUM)
    {
        return DataTransmitter_SendData(&g_transmitters[index], data, len);
    }
    else
    {
        TRACE_ERROR("\nInvalid DataTransmitter Index %d", index);
        return FALSE;
    }
}

Uint8 DataTransmitManager_TotalPortNumber(void)
{
    return TRANSMIT_UART_NUM;
}

void TRANSMITTER_1_IRQHANDLER()
{
    static DataTransmitter* transmitter = &g_transmitters[0];
    Bool bTaskWoken = FALSE;

    // 接收处理
    if (USART_GetITStatus(transmitter->uartConfig.uart, USART_IT_RXNE) != RESET)
    {
        /* Clear the USART Receive interrupt */
        USART_ClearITPendingBit(transmitter->uartConfig.uart, USART_IT_RXNE);

        // 接收
        bTaskWoken = transmitter->recvFunc((void*)transmitter);
    }

    // 发送处理
    if (USART_GetITStatus(transmitter->uartConfig.uart, USART_IT_TXE) != RESET)
    {
        // 发送
        bTaskWoken = transmitter->sendFunc((void*)transmitter);
    }

    // 发送完成
    if (USART_GetITStatus(transmitter->uartConfig.uart, USART_IT_TC) != RESET)
    {
        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TXE, DISABLE);
        USART_ITConfig(transmitter->uartConfig.uart, USART_IT_TC, DISABLE);
    }

    portEND_SWITCHING_ISR( bTaskWoken ? pdTRUE : pdFALSE );
}
