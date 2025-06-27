/*
 * UartDriver.h
 *
 *  Created on: 2020年3月18日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_UARTDRIVER_UARTDRIVER_H_
#define SRC_DRIVER_UARTDRIVER_UARTDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"
#include "LuipApi/DataTransmitInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*RCCClockCmd)(uint32_t rcc, FunctionalState state);

typedef enum
{
    RS232 = 0,
    RS485 = 1,
}SerialPortType;

typedef struct
{
    USART_TypeDef * uart;
    uint32_t uartRcc;
    RCCClockCmd uartRccCmd;
    uint32_t clk;
    uint8_t afMap;
    uint8_t irq;

    uint16_t txPin;
    GPIO_TypeDef * txPort;
    uint8_t txPinSource;
    uint32_t txRcc;
    RCCClockCmd txRccCmd;

    uint16_t rxPin;
    GPIO_TypeDef * rxPort;
    uint8_t rxPinSource;
    uint32_t rxRcc;
    RCCClockCmd rxRccCmd;

    uint16_t swPin;
    GPIO_TypeDef * swPort;
    uint8_t swPinSource;
    uint32_t swRcc;
    RCCClockCmd swRccCmd;

    SerialPortParam portParam;
}UartConfigDef;

void UartDriver_Init(UartConfigDef* config, SerialPortType type);
void UartDriver_RS485_SwitchToTx(UartConfigDef* config);
void UartDriver_RS485_SwitchToRx(UartConfigDef* config);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DRIVER_UARTDRIVER_UARTDRIVER_H_ */
