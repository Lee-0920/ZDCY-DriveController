/********** Copyright (c) 2022-2022 SZLABSUN.Co.Ltd. All rights reserved.**********
* File Name          : StirMotorManager.c
* Author             : hyz
* Date               : 06/28/2022
* Description        : This file provides all the StirMotorManager functions.
*******************************************************************************/


#include "LiquidDriver/StirMotorDriver.h"
#include "StirMotorManager.h"

void StirMotorManager_Init()
{
	StirMotorDriver_Init();
}

Bool StirMotorManager_Setlevel(Uint8 index, float level)
{
	return StirMotorDriver_SetLevel(index,level);
}
