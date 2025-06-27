/**
 * @file
 * @brief MCU 核心系统驱动。
 * @details 配置 MCU 的时钟、功耗模式、中断等系统核心参数，提供复位、延时、电源管理等功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2012-10-30
 */


#ifndef DRIVER_SYSTEM_H_
#define DRIVER_SYSTEM_H_

#include "Common/Types.h"

// 系统初始化
extern void System_Init(void);

// 总中断控制
extern void System_EnableMasterInterrupt(void);
extern void System_DisableMasterInterrupt(void);

// 软件延时
extern void System_Delay(unsigned int mSec);
extern void System_DelayUs(unsigned int uSec);
extern void System_DelaySec(unsigned int sec);
extern void System_NonOSDelay(unsigned int mSec);
// 电源管理
extern void System_PowerOn(void);
extern void System_PowerOff(void);
extern void System_Reset(void);

void System_PrintfFloat(unsigned char level, float value, unsigned char len);

void System_StateTimerInit(void);
Uint32 System_StateTimerValue(void);
void System_TaskStatePrintf(void);
// 以下宏定义与上述函数一一对应
// 供某些效率要求较高的场合使用

/**
 * @brief 打开系统的总中断。
 */
#define MASTER_INTERRUPT_ENABLE()   System_EnableMasterInterrupt();

/**
 * @brief 关闭系统的总中断。
 */
#define MASTER_INTERRUPT_DISABLE()  System_DisableMasterInterrupt();


#endif // DRIVER_SYSTEM_H_
