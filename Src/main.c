/**
 * @file main.c
 * @brief 
 * @details
 *
 * @version 1.0.0
 * @author xingfan
 * @date 2016-4-28
 */
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "System.h"
#include "console/Console.h"
#include "CmdLine.h"
#include "DncpStack.h"
#include "DeviceInfo.h"
#include "DeviceIndicatorLED.h"
#include "Watchdog.h"
#include "DeviceStatus.h"
#include "DeviceUpdate/UpdateHandle.h"
#include "SolenoidValve/ValveManager.h"
#include "LiquidDriver/StepperMotorTimer.h"
#include "PeristalticPump/PumpEventScheduler.h"
#include "PeristalticPump/PeristalticPumpManager.h"
#include "PeristalticPump/DisplacementMotorManager.h"
#include "PeristalticPump/DisplacementSteperMotorManager.h"
#include "PeristalticPump/DCMotorManager.h"
#include "PeristalticPump/StirMotorManager.h"
#include "TemperatureControl/TempCollecterManager.h"
#include "TemperatureControl/ThermostatDeviceManager.h"
#include "TemperatureControl/ThermostatManager.h"
#include "PeristalticPump/TMCConfig.h"
#include "LiquidDriver/WaterPump.h"
#include "LiquidDriver/DrainWaterPump.h"
#include "LiquidDriver/DoorLock.h"
#include "AnalogControl/AnalogControl.h"
#include "DigitalControl/DigitalControl.h"
#include "DataTransmit/DataTransmitManager.h"
#include "LiquidDriver/StirMotorDriver.h"

int main(void)
{
    System_Init();
    DeviceIndicatorLED_Init();
    Watchdog_Init();

    Console_Init();					// 功能模块初始化
    CmdLine_Init();

    DncpStack_Init();

    ValveManager_Init();			//阀初始化
    StepperMotorTimer_Init();		//步进电机定时器初始化
    PumpEventScheduler_Init();		//发送泵动作完成事件任务

    PeristalticPumpManager_Init();	//蠕动泵初始化
    DisplacementMotor_Init();		//留样定位电机初始化
    TMCConfig_Init();				//TMC电机驱动芯片(用于留样电机)初始化
    DisplacementSteperMotor_Init(); //留样步进电机初始化

    DCMotorManager_Init();			//试剂泵初始化
    StirMotorManager_Init();		//搅拌电机动作初始化

    WaterPump_Init();				//采水泵初始化
    DrainWaterPump_Init();			//排水泵初始化
    ElectronicLockControl_Init();	//电子锁初始化
    DoorDetection_Init();			//门检测初始化

    TempCollecterManager_Init();	//温度采集设备初始化
    ThermostatDeviceManager_Init();	//风扇、冰箱电源继电器初始化
    ThermostatManager_Init();		//恒温器初始化

    AnalogController_Init();		//模拟输入信号采集初始化
    DigitalController_Init();		//数字信号输入输出初始化
    DataTransmitManager_Init();		//串口数据传输管理器初始化

    DeviceInfo_Init();
 	UpdateHandle_Init();
    DeviceStatus_ReportResetEvent(DEVICE_RESET_POWER_ON); // 报告复位事件

    vTaskStartScheduler();

    /* Infinite loop */
    while (1)
    {
    }
}
#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
 * @}
 */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
