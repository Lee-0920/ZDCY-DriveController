/**
 * @file
 * @brief 恒温器接口头文件
 * @details 提供温度控制功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2012-10-30
 */

#ifndef THERMOSTAT_H_
#define THERMOSTAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "LuipApi/TemperatureControlInterface.h"
#include "Common/Types.h"
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
    float proportion;         // PID的比例系数。
    float integration;        // PID的积分系数。
    float differential;       // PID的微分系数。
}ThermostatParam;

/**
 * @brief 恒温状态。
 */
typedef enum
{
    THERMOSTAT_IDLE = 0,           ///< 空闲
    THERMOSTAT_BUSY = 1,         ///< 忙绿
}ThermostatStatus;

typedef struct
{
    ThermostatResult result;
    float temperature;
}ThermostatReportResult;

typedef struct
{
    Uint8 number;
    ThermostatMode mode;         //恒温
    float targetTemp;           // 目标温度
    float toleranceTemp;        // 容差
    float timeout;              // 超时时间
    float alreadyTime;
    ThermostatStatus status;    // 恒温状态
    ThermostatReportResult reportResult;
    Bool isThermostatReached;        //恒温完成标志
    Bool isGreaterThanObjTemp;
    Bool isSendEvent;
    Bool isRequestStop;
    ThermostatParam theromstatparam;
    xTaskHandle taskHandle;
    int tempIndex; //温度计索引
    Uint8 heaterTotal; //加热器个数，支持0个或多个，有多个加热器会输出同样的功率
    Uint8 *heaterIndex; //加热器索引集合
    Uint8 refrigeratorTotal;//制冷器个数，支持0个或多个，有多个加热器会输出同样的功率
    Uint8 *refrigeratorIndex;//制冷器索引集合
    float sumError;                         //累计误差值
    float lastError;                        //前1次误差值Error[-1]
    Bool isTempWaring;
    Bool isTempFault;
    Uint8 tempFaultCnt;
}Thermostat;

#define TEMP_WITHOUT -1

void Thermostat_Init(Thermostat *thermostat, ThermostatParam kDefaultThermostatParam);
void Thermostat_SetNumber(Thermostat *thermostat, Uint8 number);
void Thermostat_SetTemp(Thermostat *thermostat, int index);
void Thermostat_SetHeater(Thermostat *thermostat, Uint8 heaterTotal, Uint8* index);
void Thermostat_SetRefrigerator(Thermostat *thermostat, Uint8 refrigeratorTotal, Uint8* index);
ThermostatParam Thermostat_GetPIDParam(Thermostat *thermostat);
void Thermostat_SetPIDParam(Thermostat *thermostat, ThermostatParam thermostatParam);
ThermostatStatus Thermostat_GetStatus(Thermostat *thermostat);
int Thermostat_Start(Thermostat *thermostat, ThermostatMode mode, float targetTemp,
        float toleranceTemp, float timeout);
void Thermostat_SendEventClose(Thermostat *thermostat);
void Thermostat_SendEventOpen(Thermostat *thermostat);
Bool Thermostat_RequestStop(Thermostat *thermostat);
ThermostatParam Thermostat_GetCurrentPIDParam(Thermostat *thermostat);
void Thermostat_SetCurrentPIDParam(Thermostat *thermostat, ThermostatParam thermostatparam);
void Thermostat_TempMonitor(Thermostat *thermostat);
float Thermostat_GetCurrentTemp(Thermostat *thermostat);
void Thermostat_SetRefrigeratorOutput(Thermostat *thermostat, float level);
Bool Thermostat_SetSingleRefrigeratorOutput(Thermostat *thermostat, Uint8 fanIndex, float level);
#ifdef __cplusplus
}
#endif


#endif /* THERMOSTAT_H_ */
