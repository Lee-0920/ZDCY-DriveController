/*
 * SystemConfig.h
 *
 *  Created on: 2016年6月7日
 *      Author: Administrator
 */

#ifndef SRC_SYSTEMCONFIG_H_
#define SRC_SYSTEMCONFIG_H_
#include "PeristalticPump/PeristalticPumpManager.h"
#include "Driver/TempDriver/TempCollecterMap.h"
#include "TemperatureControl/ThermostatManager.h"
#include "PeristalticPump/DisplacementMotorManager.h"

//外设中断抢占式优先级配置============================================================================
#define    LAIRS485ADAPTER_IRQ_PRIORITY             7
#define    CONSOLEUART_IRQ_PRIORITY                 7
#define    TRANSMITUART_IRQ_PRIORITY             8
#define    STEPPERMOTOR_TIMERA_PRIORITY             9
#define    STEPPERMOTOR_TIMERB_PRIORITY             9
#define    STEPPERMOTOR_TIMERC_PRIORITY             9
#define    STEPPERMOTOR_TIMERD_PRIORITY             9
#define    PWM_TIMER_PRIORITY                     9
#define    SYSTEM_STATE_TIMER_PRIORITY              5
#define    OPT_XONEN_TIMER_IQR_PRIORITY             9
#define    PMT_COUNTER_TIMER_IQR_PRIORITY               4
#define    PMT_CONTROL_TIMER_IQR_PRIORITY               8
#define    TMC_UART_IRQ_PRIORITY                          6
//统一使用蠕动泵DNCP接口配置==========================================================================
#define    DNCP_PUMP_TOTAL                          7
#define    SYRINGE_OFFSET                           0
#define    PERISTALTICPUMP_OFFSET                   0
#define    DCMOTOR_OFFSET                           1
//freeRTOS 任务优先级和堆栈大小配置===================================================================
//基本任务
#define CONSOLESCHEDULER_TASK_PRIO                  5   // 控制台命令处理任务
#define CONSOLESCHEDULER_STK_SIZE                   256

#define LAIRS485COMMITTOUPPER_TASK_PRIO             6   // LAI485接收任务
#define LAIRS485COMMITTOUPPER_STK_SIZE              256

#define LAIRS485SENDREQUEST_TASK_PRIO               6   // LAI485发送任务
#define LAIRS485SENDREQUEST_STK_SIZE                256

#define LAIRS485MONITORHOST_TASK_PRIO               6   // LAI485监控任务
#define LAIRS485MONITORHOST_STK_SIZE                128

#define DNCPSTACKDSCPCMD_TASK_PRIO                  6   // DNCP命令处理任务
#define DNCPSTACKDSCPCMD_STK_SIZE                   256

#define DEVICEINDICATOR_LED_TASK_PRIO               4   // 运行指示灯任务
#define DEVICEINDICATOR_LED_STK_SIZE                128

#define UPDATER_ERASE_TASK_PRIO                     4   // 升级擦除任务
#define UPDATER_ERASE_STK_SIZE                      128

#define FEED_WATCHDOG_TASK_PRIO                     4   // 喂狗任务
#define FEED_WATCHDOG_STK_SIZE                      128

//控制台循环输出信息任务
#define CMDCLINE_INFOOUTPUT_TASK_PRIO               4   // 命令台常规信息输出
#define CMDCLINE_INFOOUTPUT_STK_SIZE                256

// 液路功能任务
#define PUMP_EVENT_TASK_PRIO                        6  //发送泵动作完成事件任务
#define PUMP_EVENT_STK_SIZE                         128

#define DISPLACEMENTMOTOR_TASK_PRIO                 6   // 位移泵过程控制任务
#define DISPLACEMENTMOTOR_STK_SIZE                  256

#define DCMOTOR_TASK_PRIO                 			5   // 试剂泵计时控制任务
#define DCMOTOR_STK_SIZE                  			128

#define PUMPTOCHECKPOINT_TASK_PRIO                 6   // 定量过程控制任务
#define PUMPTOCHECKPOINT_STK_SIZE                  256

//TMC驱动芯片监控任务
#define TMC_MONITOR_TASK_PRIO                      4
#define TMC_MONITOR_STK_SIZE                       128

#define DCMOTOR_TIMER_PRIO                          2

// 温控功能
#define TEMP_MONITOR_TASK_PRIO                      4   //温度监控任务
#define TEMP_MONITOR_STK_SIZE                       256

#define THERMOSTAT_TASK_PRIO                        5   //恒温器任务
#define THERMOSTAT_STK_SIZE                         256

#define TEMP_REPORT_TASK_PRIO                       3   //温度上报任务
#define TEMP_REPORT_STK_SIZE                        256

//模拟信号采集任务
#define ANALOGCONTROLLER_AI_COLLECT_TASK_PRIO          6
#define ANALOGCONTROLLER_AI_COLLECT_STK_SIZE           256

//数字输入信号检测任务
#define DIGITALCONTROLLER_DI_CHECK_TASK_PRIO          6
#define DIGITALCONTROLLER_DI_CHECK_STK_SIZE           128

//模拟信号上报软定时
#define  ANALOGCONTROLLER_AI_UPLOAD_TIMER_PRIO         3

//串口数据传输任务
#define UART_DATATRANSMITTER_SEND_TASK_PRIO          6
#define UART_DATATRANSMITTER_SEND_STK_SIZE           128

//串口数据传输任务
#define UART_DATATRANSMITTER_RECV_TASK_PRIO          6
#define UART_DATATRANSMITTER_RECV_STK_SIZE           256

//串口数据超时定时器
#define UART_DATATRANSMITTER_TIMER_PRIO        3

// 用户FLASH地址和大小配置==============================================================================
#define UPDATE_FLASH_START                          0x08000000//UPDATE程序空间48K
#define UPDATE_FLASH_END                            0x0800BFFF
#define UPDATE_DATA_FLASH_START                     0x0800C000//UPDATE数据空间16K
#define UPDATE_DATA_FLASH_END                       0x0800FFFF
#define APP_DATA_FLASH_START                        0x08010000//APP数据空间64K
#define APP_DATA_FLASH_END                          0x0801FFFF
#define APP_FLASH_START                             0x08020000 
#define APP_FLASH_END                               0x081FFFFF


#define FLASH_FACTORY_SIGN                          0xAA55AA55
#define FLASH_USE_BASE                              APP_DATA_FLASH_START
//板卡信息：共100byte
#define DEVICE_INFO_SIGN_FLASH_BASE_ADDR            FLASH_USE_BASE
#define DEVICE_INFO_SIGN_FLASH_LEN                  4

#define DEVICE_INFO_TYPE_ADDRESS                    (DEVICE_INFO_SIGN_FLASH_BASE_ADDR + DEVICE_INFO_SIGN_FLASH_LEN)
#define DEVICE_INFO_TYPE_LEN                        16
#define DEVICE_INFO_MODEL_ADDRESS                   (DEVICE_INFO_TYPE_ADDRESS + DEVICE_INFO_TYPE_LEN)
#define DEVICE_INFO_MODEL_LEN                       24
#define DEVICE_INFO_SN_ADDRESS                      (DEVICE_INFO_MODEL_ADDRESS + DEVICE_INFO_MODEL_LEN)
#define DEVICE_INFO_SN_LEN                          32
#define DEVICE_INFO_MANUF_ADDRESS                   (DEVICE_INFO_SN_ADDRESS +  DEVICE_INFO_SN_LEN)
#define DEVICE_INFO_MANUF_LEN                       20
#define DEVICE_INFO_DATE_ADDRESS                    (DEVICE_INFO_MANUF_ADDRESS + DEVICE_INFO_MANUF_LEN)
#define DEVICE_INFO_DATE_LEN                        4

//蠕动泵参数：共16yte = (4+8+4) * 1
#define PUMP_SIGN_FLASH_BASE_ADDR                   (DEVICE_INFO_DATE_ADDRESS + DEVICE_INFO_DATE_LEN)
#define PUMP_SIGN_FLASH_LEN                         4

#define PUMP_MOTIONPARAM_FLASH_BASE_ADDR            (PUMP_SIGN_FLASH_BASE_ADDR + PUMP_SIGN_FLASH_LEN * PERISTALTICPUMPMANAGER_TOTAL_PUMP)
#define PUMP_MOTIONPARAM_FLASH_LEN                  sizeof(StepperMotorParam)

#define PUMP_FACTOR_FLASH_BASE_ADDR                 (PUMP_MOTIONPARAM_FLASH_BASE_ADDR + PUMP_MOTIONPARAM_FLASH_LEN * PERISTALTICPUMPMANAGER_TOTAL_PUMP)
#define PUMP_FACTOR_FLASH_LEN                       sizeof(float)

//注射器：共16yte = (4+8+4) * 1
#define SYRINGE_SIGN_FLASH_BASE_ADDR                   (PUMP_FACTOR_FLASH_BASE_ADDR + PUMP_FACTOR_FLASH_LEN * PERISTALTICPUMPMANAGER_TOTAL_PUMP)
#define SYRINGE_SIGN_FLASH_LEN                         4

#define SYRINGE_MOTIONPARAM_FLASH_BASE_ADDR            (SYRINGE_SIGN_FLASH_BASE_ADDR + SYRINGE_SIGN_FLASH_LEN)
#define SYRINGE_MOTIONPARAM_FLASH_LEN                  sizeof(StepperMotorParam)

#define SYRINGE_FACTOR_FLASH_BASE_ADDR                 (SYRINGE_MOTIONPARAM_FLASH_BASE_ADDR + SYRINGE_MOTIONPARAM_FLASH_LEN)
#define SYRINGE_FACTOR_FLASH_LEN                       sizeof(float)

//位移泵器：共12yte = (4+8) * 2
#define DISPLACEMENTMOTOR_SIGN_FLASH_BASE_ADDR        (SYRINGE_FACTOR_FLASH_BASE_ADDR + SYRINGE_FACTOR_FLASH_LEN)
#define DISPLACEMENTMOTOR_SIGN_FLASH_LEN              4

#define DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_BASE_ADDR            (DISPLACEMENTMOTOR_SIGN_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_SIGN_FLASH_LEN * DISPLACEMENTMOTOR_TOTAL_PUMP)
#define DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN                  sizeof(StepperMotorParam)

//温度采集功能参数：64 byte = 4 * 4  + 12 * 4
#define DEVICE_TEMPERATURE_BASE                                 (DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_BASE_ADDR + DISPLACEMENTMOTOR_MOTIONPARAM_FLASH_LEN * DISPLACEMENTMOTOR_TOTAL_PUMP)
#define TEMPERATURE_FACTORY_SIGN_FLASH_BASE_ADDR                (DEVICE_TEMPERATURE_BASE + 0)
#define TEMPERATURE_FACTORY_SIGN_FLASH_LEN                      4
#define DEVICE_TEMPERATURE_CALIBRATE_FACTOR_ADDRESS             (TEMPERATURE_FACTORY_SIGN_FLASH_BASE_ADDR + TEMPERATURE_FACTORY_SIGN_FLASH_LEN * TOTAL_TEMP)
#define DEVICE_TEMPERATURE_CALIBRATE_FACTOR_LEN                 sizeof(TempCalibrateParam)

//恒温器功能参数：32 byte = 4 * 2 + 12 * 2
#define THERMOSTAT_FACTORY_SIGN_FLASH_BASE_ADDR                 (DEVICE_TEMPERATURE_CALIBRATE_FACTOR_ADDRESS + DEVICE_TEMPERATURE_CALIBRATE_FACTOR_LEN * TOTAL_TEMP)
#define THERMOSTAT_FACTORY_SIGN_FLASH_LEN                       4
#define DEVICE_THERMOSTAT_PARAM_ADDRESS                         (THERMOSTAT_FACTORY_SIGN_FLASH_BASE_ADDR + THERMOSTAT_FACTORY_SIGN_FLASH_LEN * TOTAL_THERMOSTAT)
#define DEVICE_THERMOSTAT_PARAM_LEN                             sizeof(ThermostatParam)

//AD5175默认设置标志与参数 : 16byte = 4 + 2*2 + 8(预留)
#define AD5175_CONTROL_SIGN_FLASH_BASE_ADDR                 (DEVICE_THERMOSTAT_PARAM_ADDRESS + DEVICE_THERMOSTAT_PARAM_LEN*TOTAL_THERMOSTAT)
#define AD5175_CONTROL_SIGN_FLASH_LEN                       4
#define AD5175_CONTROL_PARAM_FLASH_BASE_ADDR                 (AD5175_CONTROL_SIGN_FLASH_BASE_ADDR + AD5175_CONTROL_SIGN_FLASH_LEN)
#define AD5175_CONTROL_PARAM_FLASH_LEN                       12

//串口配置参数：8个串口数据传输器
#define TRANSMITTER_SIGN_FLASH_BASE_ADDR            	AD5175_CONTROL_PARAM_FLASH_BASE_ADDR + AD5175_CONTROL_PARAM_FLASH_LEN
#define TRANSMITTER_SIGN_FLASH_LEN                  	4
#define TRANSMITTER_SIGN_FLASH_LEN_TOTAL        		4
#define TRANSMITTER_PARAM_FLASH_BASE_ADDR            	TRANSMITTER_SIGN_FLASH_BASE_ADDR + TRANSMITTER_SIGN_FLASH_LEN_TOTAL
#define TRANSMITTER_PARAM_FLASH_LEN                  	16         //预留16个字节  //实际使用7字节
#define TRANSMITTER_PARAM_FLASH_LEN_TOTAL      			16

//所有使用的FLASH 244
#define FLASH_USE_SIZE                              ((u16)400)

//系统时钟参数
#define CLK_SYS   180000000
#define CLK_AHB1  CLK_SYS
#define CLK_APB1  (CLK_SYS/4)
#define CLK_APB2  (CLK_SYS/2)

#endif /* SRC_SYSTEMCONFIG_H_ */
