/*
 * TMC2160.c
 *
 *  Created on: 2020年5月14日
 *      Author: Administrator
 */
#include "TMC2160.h"
#include "TMCConfig.h"
#include "SystemConfig.h"
#include "Driver/System.h"
#include "Tracer/Trace.h"
#include "PeristalticPump/DisplacementMotorManager.h"
#include "PeristalticPump/PeristalticPumpManager.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include "Driver/TMC2160SPIDriver/TMC2160SPIDriver.h"

#define TMC2160_GCONF			0x00    //全局配置标志
#define TMC2160_GSTAT			0x01	//全局状态寄存器
#define TMC2160_DRV_STATUS 		0x6F	//
#define TMC2160_CHOPCONF		0x6C	//<斩波配置寄存器 RW //微步细分设置 b27-b24:%0000-%1000 -> 256-1 (%0100 = 16)
#define TMC2160_IHOLD_IRUN		0x10	//<(只写)电机保持运行电流配置 b19-b16:降电流延时 b12-b8:运行电流 b4-b0:静止电流  0=1/32 31=32/32
#define TMC2160_TPOWERDOWN		0x11

//向地址为slaveAddr的芯片的regAddr寄存器写数据
Bool TMC2160_WriteData(Uint8 slaveAddr,Uint8 regAddr,unsigned long data)
{
	regAddr = regAddr+0x80;
	if(1 == slaveAddr)
	{
		TMC2160_1_sendData(regAddr, data);
	}
	else
	{
		TMC2160_2_sendData(regAddr, data);
	}

	return TRUE;
}

//读取芯片寄存器的数据
Bool TMC2160_ReadData(Uint8 slaveAddr,Uint8 regAddr,unsigned long *data)//需要找出slaveAddr与芯片片选的关系
{
	if(1 == slaveAddr)
	{
		*data = TMC2160_1_ReadData(regAddr);
	}
	else
	{
		*data = TMC2160_2_ReadData(regAddr);
	}

	return TRUE;
}

void TMC2160Config_RegList(Uint8 slaveAddr)
{
	Uint32 data[6] = {0};
	TMC2160_ReadData(slaveAddr, TMC2160_GCONF, &data[0]);
	TMC2160_ReadData(slaveAddr, TMC2160_GCONF, &data[0]);
	System_NonOSDelay(1);
	TMC2160_ReadData(slaveAddr, TMC2160_GSTAT, &data[1]);
	TMC2160_ReadData(slaveAddr, TMC2160_GSTAT, &data[1]);
	System_NonOSDelay(1);
	TMC2160_ReadData(slaveAddr, TMC2160_CHOPCONF, &data[2]);
	TMC2160_ReadData(slaveAddr, TMC2160_CHOPCONF, &data[2]);
	System_NonOSDelay(1);
	TMC2160_ReadData(slaveAddr, TMC2160_DRV_STATUS, &data[3]);
	TMC2160_ReadData(slaveAddr, TMC2160_DRV_STATUS, &data[3]);
	Printf("\n====TMC2160 Driver %d Reg List====", slaveAddr);
	Printf("\nGCONF(%02X): 0x%08X", TMC2160_GCONF, data[0]);
	Printf("\nGSTAT(%02X): 0x%08X", TMC2160_GSTAT, data[1]);
	Printf("\nCHOPCONF(%02X): 0x%08X", TMC2160_CHOPCONF, data[2]);
	Printf("\nDRV_STATUS(%02X): 0x%08X", TMC2160_DRV_STATUS, data[3]);
}

//复位检查
Uint8 TMC2160_DriverCheck(StepperMotor* motor)
{
	Uint32 gstat=0;
	Uint8 slaveAddr = motor->number;

	TMC2160_ReadData(slaveAddr,TMC2160_GSTAT,&gstat);
	memset(&gstat, 0, sizeof(gstat));
	TMC2160_ReadData(slaveAddr,TMC2160_GSTAT,&gstat);

	if(TRUE == (gstat & 0x1))//表示芯片已复位，所有寄存器都清0并重置复位值
	{
	        TRACE_WARN("\nTMC Driver Reset %d", slaveAddr);
	        //如果检查到芯片复位，则需要重新初始化芯片，设置寄存器的值
	}

	return (Uint8)gstat;
}

//检测TMC2160故障
DriverError TMC2160_ReadDriveError(Uint8 slaveAddr)
{
	Uint32 gstat = 0;
	Uint32 drvStatus = 0;
	DriverError error = NoError;

	TMC2160_ReadData(slaveAddr,TMC2160_GSTAT,&gstat);
	memset(&gstat, 0, sizeof(gstat));
	TMC2160_ReadData(slaveAddr,TMC2160_GSTAT,&gstat);

	if(gstat & 0x2)//高温或者驱动出错
	{
		TMC2160_ReadData(slaveAddr,TMC2160_DRV_STATUS,&drvStatus);
		memset(&drvStatus, 0, sizeof(drvStatus));
		TMC2160_ReadData(slaveAddr,TMC2160_DRV_STATUS,&drvStatus);

		if(drvStatus & 0x18000000)//短路
		{
			error = ShortCircuit;
			TRACE_ERROR("\nTMC2160 Driver Status = 0x%08X, Diagnostic : ShortCircuit", drvStatus);
		}
		else if(drvStatus & 0x06000000)//温度过高
		{
			error = OverTemperature;
			TRACE_ERROR("\nTMC2160 Driver Status = 0x%08X, Diagnostic : OverTemperature", drvStatus);
		}
	}
	else if(gstat & 0x4)//欠压
	{
		error = UnderVoltage;
		TRACE_ERROR("\nTMC2160 Driver GSTAT = 0x%08X, Diagnostic : UnderVoltage", gstat);
	}
	else
	{
		TRACE_INFO("\nTMC2160 Driver GSTAT = 0x%08X, Diagnostic : NoError", gstat);
	}
	return error;
}

//读取TMC2160细分系数
Uint32 TMC2160_ReadSubdivision(Uint8 slaveAddr)
{
	Uint32 subdivision = 0;
	Uint32 chopConf = 0;

	TMC2160_ReadData(slaveAddr,TMC2160_CHOPCONF,&chopConf);
	memset(&chopConf, 0, sizeof(chopConf));
	TMC2160_ReadData(slaveAddr,TMC2160_CHOPCONF,&chopConf);

	subdivision = (Uint32)(pow(2, 8 - ((chopConf >> 24) & 0xF)));

	TRACE_INFO("\nTMC2160 read slave %d subdivision = %d", slaveAddr, subdivision);

	return subdivision;
}

//设置TMC2160的细分系数
Bool TMC2160_WriteSubdivision(Uint8 slaveAddr, Uint32 subdivision)
{
	if(subdivision > 256)
	{
		TRACE_ERROR("\nTMC2160 set subdivision = %d out of limit", subdivision);
		return FALSE;
	}

	Uint8 value = 8 - (Uint8)(log2(subdivision));

	Uint32 chopConf = 0;
	TMC2160_ReadData(slaveAddr, TMC2160_CHOPCONF, &chopConf);
	TMC2160_ReadData(slaveAddr, TMC2160_CHOPCONF, &chopConf);

	chopConf = (chopConf & 0xF0FFFFFF) | (value << 24);

	System_NonOSDelay(1);

	TMC2160_WriteData(slaveAddr,TMC2160_CHOPCONF,chopConf);

	return TRUE;
}

//设置电机的电流
Uint32 TMC2160_CurrentSet(Uint8 slaveAddr, Uint8 ihold, Uint8 irun, Uint8 delay)
{
	Uint32 data = ((delay&0xF) << 16) + ((irun&0x1F) << 8) + (ihold&0x1F);
	TRACE_INFO("\nTMC2160 current set ihold = 0x%x, irun = 0x%x, delay = 0x%x, data = 0x%08X", ihold, irun, delay, data);

	TMC2160_WriteData(slaveAddr,TMC2160_IHOLD_IRUN,data);

	return TRUE;
}

Bool TMC2160Config_MotorDriverInit(StepperMotor* motor)
{
	Uint8 slaveAddr = motor->number;//编号为1或2
	Uint32 subdivision = motor->subdivision;
	Uint8 ret = TRUE;

	TRACE_INFO("\nTMC Motor %d Driver Init Subdivision = %d", slaveAddr, subdivision);

	TMC2160_WriteData(slaveAddr, TMC2160_CHOPCONF, 0x000100C3);
	TMC2160_WriteSubdivision(slaveAddr, subdivision);//设置细分系数
	System_NonOSDelay(1);
	TMC2160_CurrentSet(slaveAddr,0,11,6);
	System_NonOSDelay(1);
	TMC2160_WriteData(slaveAddr, TMC2160_GCONF, 0x00000004);//TMC2160_GCONF寄存器只能写
	System_NonOSDelay(1);
	TMC2160_WriteData(slaveAddr, 0x13, 0x00000064);
	System_NonOSDelay(1);
	TMC2160_WriteData(slaveAddr, TMC2160_TPOWERDOWN, 0x0000000A);//TMC2160_TPOWERDOWN寄存器只能写
	System_NonOSDelay(1);
	TMC2160_WriteData(slaveAddr, 0x09, 0x00000F0F);//设置检测灵敏度

	return ret;
}
