/*
 * ThermostatManager.c
 *
 *  Created on: 2017年11月17日
 *      Author: LIANG
 */

#include "ThermostatDeviceManager.h"
#include "ThermostatManager.h"
#include "Tracer/Trace.h"
#include "DNCP/App/DscpSysDefine.h"
#include "SystemConfig.h"
#include "DNCP/Lai/LaiRS485Handler.h"
#include "DncpStack/DncpStack.h"

// 默认恒温参数
const static ThermostatParam kDefaultThermostatParam1 =
{ .proportion = 90, .integration = 0.71, .differential = 85, };

Thermostat s_thermostats[TOTAL_THERMOSTAT];
static void ThermostatManager_TempMonitor(void *argument);

static xTaskHandle s_reportHandle;
static Uint32 s_TemperatureReportPeriod = 1000;
static Uint32 s_EnvironmentTemperatureCnt = 0;

static void ThermostatManager_TemperatureReport(void *argument);

void ThermostatManager_Init()
{
    Uint8 index;
    for (Uint8 i = 0; i < TOTAL_THERMOSTAT; i++)
    {
        Thermostat_SetNumber(&s_thermostats[i], i);
    }

    Thermostat_Init(&s_thermostats[0], kDefaultThermostatParam1);
    Thermostat_SetTemp(&s_thermostats[0],  MEAROOM_TEMP);
    index = 0;
    Thermostat_SetRefrigerator(&s_thermostats[0], 1, &index);

    xTaskCreate(ThermostatManager_TempMonitor, "TempMonitor",
            TEMP_MONITOR_STK_SIZE, NULL,
            TEMP_MONITOR_TASK_PRIO, 0);//温度监控任务

    xTaskCreate(ThermostatManager_TemperatureReport, "TempReport",
            TEMP_REPORT_STK_SIZE, NULL,
            TEMP_REPORT_TASK_PRIO, &s_reportHandle);//温度上报任务
}

char* ThermostatManager_GetName(Uint8 index)
{
    static char name[15] = "";
    switch(index)
    {
    case 0:
        strcpy(name, "MeasureRoom");
        break;
    default:
        strcpy(name, "NULL");
        break;
    }
    return name;
}

void ThermostatManager_RestoreInit(void)
{
    for (Uint8 i = 0; i < TOTAL_THERMOSTAT; i++)
    {
        if(THERMOSTAT_BUSY == ThermostatManager_GetStatus(i))
        {
            ThermostatManager_SendEventClose(i);
            ThermostatManager_RequestStop(i);
        }
    }
}

ThermostatParam ThermostatManager_GetPIDParam(Uint8 index)
{
    ThermostatParam thermostatParam;
    if (index < TOTAL_THERMOSTAT)
    {
        thermostatParam = Thermostat_GetPIDParam(&s_thermostats[index]);
    }
    else
    {
        TRACE_ERROR("\n GetPIDParam  No. %d thermostat.", index);
    }
    return thermostatParam;
}

Bool ThermostatManager_SetPIDParam(Uint8 index, ThermostatParam thermostatParam)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTAT)
    {
        TRACE_INFO("\n %s Thermostat ", ThermostatManager_GetName(index));
        Thermostat_SetPIDParam(&s_thermostats[index], thermostatParam);
        ret = TRUE;
    }
    else
    {
        TRACE_ERROR("\n SetPIDParam  No. %d thermostat.", index);
    }
    return ret;
}

ThermostatStatus ThermostatManager_GetStatus(Uint8 index)
{
    ThermostatStatus thermostatStatus;
    if (index < TOTAL_THERMOSTAT)
    {
        thermostatStatus = Thermostat_GetStatus(&s_thermostats[index]);
    }
    else
    {
        TRACE_ERROR("\n GetStatus  No. %d thermostat.", index);
    }
    return thermostatStatus;
}

int ThermostatManager_Start(Uint8 index, ThermostatMode mode, float targetTemp,
        float toleranceTemp, float timeout)
{
    int ret = DSCP_ERROR_PARAM;
    if (index < TOTAL_THERMOSTAT)
    {
        TRACE_INFO("\n %s Thermostat Start", ThermostatManager_GetName(index));
        ret = Thermostat_Start(&s_thermostats[index], mode, targetTemp, toleranceTemp, timeout);
    }
    else
    {
        TRACE_ERROR("\n Start  No. %d thermostat.", index);
    }
    return ret;
}

void ThermostatManager_SendEventClose(Uint8 index)
{
    if (index < TOTAL_THERMOSTAT)
    {
        Thermostat_SendEventClose(&s_thermostats[index]);
        TRACE_INFO("\n SendEventCloses %d thermostat.", index);
    }
    else
    {
        TRACE_ERROR("\n SendEventCloses  No. %d thermostat.", index);
    }
}

void ThermostatManager_SendEventOpen(Uint8 index)
{
    if (index < TOTAL_THERMOSTAT)
    {
        Thermostat_SendEventOpen(&s_thermostats[index]);
        TRACE_INFO("\n SendEventOpen %d thermostat.", index);
    }
    else
    {
        TRACE_ERROR("\n SendEventCloses  No. %d thermostat.", index);
    }
}

Bool ThermostatManager_RequestStop(Uint8 index)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTAT)
    {
        TRACE_INFO("\n %s Thermostat", ThermostatManager_GetName(index));
        ret= Thermostat_RequestStop(&s_thermostats[index]);
    }
    else
    {
        TRACE_ERROR("\n RequestStop  No. %d thermostat.", index);
    }
    return ret;
}

ThermostatParam ThermostatManager_GetCurrentPIDParam(Uint8 index)
{
    ThermostatParam thermostatParam;
    if (index < TOTAL_THERMOSTAT)
    {
        thermostatParam = Thermostat_GetCurrentPIDParam(&s_thermostats[index]);
    }
    else
    {
        TRACE_ERROR("\n GetCurrentPIDParam  No. %d thermostat.", index);
    }
    return thermostatParam;
}

Bool ThermostatManager_SetCurrentPIDParam(Uint8 index, ThermostatParam thermostatParam)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTAT)
    {
        TRACE_INFO("\n %s Thermostat ", ThermostatManager_GetName(index));
        Thermostat_SetCurrentPIDParam(&s_thermostats[index], thermostatParam);
        ret = TRUE;
    }
    else
    {
        TRACE_ERROR("\n SetCurrentPIDParam  No. %d thermostat.", index);
    }
    return ret;
}

void ThermostatManager_TempMonitor(void *argument)
{
    while(1)
    {
        System_Delay(500);
        for (Uint8 i = 0; i < TOTAL_THERMOSTAT; i++)
        {
            Thermostat_TempMonitor(&s_thermostats[i]);
        }
    }
}

/**
 * @brief 设置温度上报参数
 * @param
 */
void ThermostatManager_SetTempReportPeriod(float reportparam)
{
    Uint32  times = (uint32_t)(reportparam * 1000);
    s_TemperatureReportPeriod = times;
    TRACE_INFO("\n SetTempReport: %d ms", times);
    if (times > 0)
    {
        vTaskResume(s_reportHandle);
        if(times < 2000)//获取环境温度需要间隔2s
        {
            s_EnvironmentTemperatureCnt = 2000 / times;
            while(s_EnvironmentTemperatureCnt * times < 2000)
            {
                s_EnvironmentTemperatureCnt++;
            }
            TRACE_INFO("\n s_EnvironmentTemperatureCnt: %d", s_EnvironmentTemperatureCnt);
        }
        else
        {
            s_EnvironmentTemperatureCnt = 1;
        }
    }
    else
    {
        vTaskSuspend(s_reportHandle);
        s_EnvironmentTemperatureCnt = 0;
    }
}

/**
 * @brief 获取温度上报周期
 * @param
 */
static Uint32 ThermostatManager_GetTempReportPeriod(void)
{
    return s_TemperatureReportPeriod;
}
/**
 // * @brief 温度上报任务处理
 // * @param
 // */
static void ThermostatManager_TemperatureReport(void *argument)
{
    Uint8 data[1 + 4 * (TOTAL_TEMP + 1)] = { 0 };
    data[4] = TOTAL_TEMP;
    float environmentTemp = 0;
    float temp = 0;
    Uint8 cnt = 0;
    //vTaskSuspend(s_reportHandle);
    while(1)
    {
        System_Delay(ThermostatManager_GetTempReportPeriod());  // 获取上报周期
        if (TRUE == LaiRS485_GetHostStatus())
        {
            for (Uint8 i = 0; i < TOTAL_TEMP; i++)
            {
                ///temp = Thermostat_GetCurrentTemp(&s_thermostats[i]);
                temp = TempCollecterManager_GetTemp(i);
                
                /******************************************************
                温度修正：y= 0.051xx - 0.5794x + 2.4664  
                范     围：{0.0, 7.55}
                ******************************************************/
                if(temp > 0.0 && temp < 7.55)
                {
                    temp  = 0.051*temp*temp*temp - 0.5794*temp*temp + 2.4664*temp;
                }
                memcpy(data + 5 + i * 4, &temp, sizeof(temp));
            }
            if(++cnt >= s_EnvironmentTemperatureCnt)//为获取精确的环境温度，两次获取温度间隔需要2s
            {
                cnt = 0;
                environmentTemp= TempCollecterManager_GetEnvironmentTemp();
                memcpy(data, &environmentTemp, sizeof(environmentTemp));
            }
            DncpStack_SendEvent(DSCP_EVENT_TCI_TEMPERATURE_NOTICE,
                    (void*) data, sizeof(data));
        }
   }
}

Bool ThermostatManager_SetSingleRefrigeratorOutput(Uint8 thermostatIndex, Uint8 fanIndex, float level)
{
    Bool ret = FALSE;
    if (thermostatIndex < TOTAL_THERMOSTAT)
    {
        ret = Thermostat_SetSingleRefrigeratorOutput(&s_thermostats[thermostatIndex], fanIndex, level);
    }
    else
    {
        TRACE_ERROR("\n SetOutput  No. %d thermostat.", thermostatIndex);
    }
    return ret;
}

float ThermostatManager_GetHeaterMaxDutyCycle(Uint8 index)
{
    float ret = 0;
    if (index < TOTAL_THERMOSTAT)
    {
        if (s_thermostats[index].heaterTotal > 0)
        {
            ret = ThermostatDeviceManager_GetMaxDutyCycle(s_thermostats[index].heaterIndex[1]);
        }
    }
    else
    {
        TRACE_ERROR("\n GetMaxDutyCycle  No. %d thermostat.", index);
    }
    return ret;
}

Bool ThermostatManager_SetMaxDutyCycle(Uint8 index, float value)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTAT)
    {
        for (Uint8 i = 0; i < s_thermostats[index].heaterTotal; i++)
        {
            ret = ThermostatDeviceManager_SetMaxDutyCycle(s_thermostats[index].heaterIndex[i], value);
        }
    }
    else
    {
        TRACE_ERROR("\n SetMaxDutyCycle  No. %d thermostat.", index);
    }
    return ret;
}

TempCalibrateParam ThermostatManager_GetCalibrateFactor(Uint8 index)
{
    TempCalibrateParam tempCalibrateParam;
    if (index < TOTAL_THERMOSTAT)
    {
        tempCalibrateParam = TempCollecterManager_GetCalibrateFactor(s_thermostats[index].tempIndex);
    }
    else
    {
        TRACE_ERROR("\n GetCalibrateFactor  No. %d thermostat.", index);
    }
    return tempCalibrateParam;
}

Bool ThermostatManager_SetCalibrateFactor(Uint8 index, TempCalibrateParam tempCalibrateParam)
{
    Bool ret = FALSE;
    if (index < TOTAL_THERMOSTAT)
    {
        TRACE_INFO("\n %s Thermostat", ThermostatManager_GetName(index));
        ret = TempCollecterManager_SetCalibrateFactor(s_thermostats[index].tempIndex, tempCalibrateParam);
    }
    else
    {
        TRACE_ERROR("\n SetCalibrateFactor  No. %d thermostat.", index);
    }
    return ret;
}

float ThermostatManager_GetCurrentTemp(Uint8 index)
{
    float temp = 0;
    if (index < TOTAL_THERMOSTAT)
    {
        temp = Thermostat_GetCurrentTemp(&s_thermostats[index]);
    }
    else
    {
        TRACE_ERROR("\n GetCurrentTemp  No. %d thermostat.", index);
    }
    return temp;
}
