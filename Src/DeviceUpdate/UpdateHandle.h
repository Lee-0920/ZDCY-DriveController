/*
 * UpdateHandle.h
 *
 *  Created on: 2016年7月19日
 *      Author: LIANG
 */

#ifndef SRC_DEVICEUPDATE_UPDATEHANDLE_H_
#define SRC_DEVICEUPDATE_UPDATEHANDLE_H_

#include "Common/Types.h"
#include "LuipApi/DeviceStatusInterface.h"

typedef struct
{
    Uint16 status;
    Uint16 seq;
}WriteProgramResult;

void UpdateHandle_Init(void);
Bool UpdateHandle_StartErase(void);
void UpdateHandle_SendEventOpen(void);
WriteProgramResult UpdateHandle_WriteProgram(Byte *data, Uint16 length, Uint16 seq);
Bool UpdateHandle_CheckIntegrity(Uint16 checksum);

#ifdef _CS_APP
void UpdateHandle_EnterUpdater(void);
#else
void UpdateHandle_EnterApplication(void);
#endif
DeviceRunMode UpdateHandle_GetRunMode(void);
Uint16 UpdateHandle_GetMaxFragmentSize(void);

#endif /* SRC_DEVICEUPDATE_UPDATEHANDLE_H_ */
