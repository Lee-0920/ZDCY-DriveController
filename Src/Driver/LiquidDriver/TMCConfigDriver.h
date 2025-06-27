/*
 * TMCConfigDriver.h
 *
 *  Created on: 2020年1月7日
 *      Author: Administrator
 */

#ifndef SRC_DRIVER_LIQUIDDRIVER_TMCCONFIGDRIVER_H_
#define SRC_DRIVER_LIQUIDDRIVER_TMCCONFIGDRIVER_H_

#include "stm32f4xx.h"
#include "Common/Types.h"

typedef struct
{
    Uint8 destAddr;
    Uint8 regAddr;
    Uint32 data;
}TMC_Data;

void TMCConfigDriver_UARTInit(void);
void TMCConfigDriver_ClearResp(void);
Bool TMCConfigDriver_WaitRespData(TMC_Data* resp, Uint16 timeoutMS);
Uint32 TMCConfigDriver_WriteData(Uint8 *pTx_Buff, Uint8 len);
void TMCConfigDriver_CRC8(unsigned char* datagram, unsigned char len);

#endif /* SRC_DRIVER_LIQUIDDRIVER_TMCCONFIGDRIVER_H_ */
