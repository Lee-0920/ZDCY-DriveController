
/**
 * @file
 * @brief 恒温器接口
 * @details 提供温度控制功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2012-10-30
 */

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "Driver/System.h"
#include "Driver/McuFlash.h"
#include "DncpStack/DncpStack.h"
#include "Tracer/Trace.h"
#include <string.h>
#include "SystemConfig.h"
#include "DNCP/App/DscpSysDefine.h"
#include "TempCollecterManager.h"
#include "ThermostatDeviceManager.h"
#include "Thermostat.h"

static void Thermostat_Handle(void *argument);
static void Thermostat_InitParam(Thermostat *thermostat, ThermostatParam kDefaultThermostatParam);

#define TEMP_WARNING_VALUE  200 //温度监控的警戒值
#define TEMP_UNUSUAL_VALUE  -50 //温度传感器异常值
/**
 * @brief    恒温初始化
 * @details
 */
void Thermostat_Init(Thermostat *thermostat, ThermostatParam kDefaultThermostatParam)
{
    thermostat->mode = THERMOSTAT_MODE_AUTO;
    thermostat->targetTemp = 0;
    thermostat->toleranceTemp = 0;
    thermostat->timeout = 0;
    thermostat->alreadyTime = 0;
    thermostat->status = THERMOSTAT_IDLE;
    thermostat->reportResult.result = THERMOSTAT_RESULT_REACHED;
    thermostat->reportResult.temperature = 0;
    thermostat->isSendEvent = FALSE;
    thermostat->isThermostatReached = FALSE;
    thermostat->isGreaterThanObjTemp = FALSE;
    thermostat->isRequestStop = FALSE;

    Thermostat_InitParam(thermostat, kDefaultThermostatParam);
    //恒温器任务
    /*xTaskCreate(Thermostat_Handle, "Thermostat_Handle", THERMOSTAT_STK_SIZE, (void*)thermostat,
            THERMOSTAT_TASK_PRIO, &thermostat->taskHandle);*/

    thermostat->tempIndex = TEMP_WITHOUT;
    thermostat->heaterIndex = NULL;
    thermostat->heaterTotal = 0;
    thermostat->refrigeratorIndex = NULL;
    thermostat->refrigeratorTotal = 0;
    thermostat->sumError = 0;
    thermostat->lastError = 0;
    thermostat->isTempWaring = FALSE;
}

void Thermostat_SetNumber(Thermostat *thermostat, Uint8 number)
{
    thermostat->number = number;
}

void Thermostat_SetTemp(Thermostat *thermostat, int index)
{
    thermostat->tempIndex = index;
}

void Thermostat_SetHeater(Thermostat *thermostat, Uint8 heaterTotal, Uint8* index)
{
    thermostat->heaterTotal = heaterTotal;
    thermostat->heaterIndex = pvPortMalloc(heaterTotal);
    memcpy(thermostat->heaterIndex, index, heaterTotal);
}

void Thermostat_SetRefrigerator(Thermostat *thermostat, Uint8 refrigeratorTotal, Uint8* index)
{
    thermostat->refrigeratorTotal = refrigeratorTotal;
    thermostat->refrigeratorIndex = pvPortMalloc(refrigeratorTotal);
    memcpy(thermostat->refrigeratorIndex, index, refrigeratorTotal);
}
/**
 * @brief 获取恒温参数
 * @param
 */
ThermostatParam Thermostat_GetPIDParam(Thermostat *thermostat)
{
    Uint8 readData[DEVICE_THERMOSTAT_PARAM_LEN] =
    { 0 };
    ThermostatParam thermostatparam;

    McuFlash_Read(DEVICE_THERMOSTAT_PARAM_ADDRESS + DEVICE_THERMOSTAT_PARAM_LEN * thermostat->number,
    DEVICE_THERMOSTAT_PARAM_LEN, readData);
    memcpy(&thermostatparam, readData, sizeof(ThermostatParam));
    return thermostatparam;
}

/**
 * @brief 设置恒温参数
 * @param
 */
void Thermostat_SetPIDParam(Thermostat *thermostat, ThermostatParam thermostatparam)
{
    Uint8 writeData[DEVICE_THERMOSTAT_PARAM_LEN] =
    { 0 };

    memcpy(writeData, &thermostatparam, sizeof(ThermostatParam));

    McuFlash_Write(DEVICE_THERMOSTAT_PARAM_ADDRESS + DEVICE_THERMOSTAT_PARAM_LEN * thermostat->number,
    DEVICE_THERMOSTAT_PARAM_LEN, writeData);

    thermostat->theromstatparam = thermostatparam;
    TRACE_INFO("\n No %d Thermostat Set p = ", thermostat->number);
    System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.proportion, 3);
    TRACE_INFO("\n i =");
    System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.integration, 3);
    TRACE_INFO("\n d =");
    System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.differential, 3);
}

/**
 * @brief 初始化参数
 * @param
 */
static void Thermostat_InitParam(Thermostat *thermostat, ThermostatParam kDefaultThermostatParam)
{
    Uint8 buffer[TEMPERATURE_FACTORY_SIGN_FLASH_LEN] =
    { 0 };
    Uint32 flashFactorySign = 0;

    McuFlash_Read(THERMOSTAT_FACTORY_SIGN_FLASH_BASE_ADDR + THERMOSTAT_FACTORY_SIGN_FLASH_LEN * thermostat->number,
    THERMOSTAT_FACTORY_SIGN_FLASH_LEN, buffer);                //读取出厂标志位
    memcpy(&flashFactorySign, buffer, THERMOSTAT_FACTORY_SIGN_FLASH_LEN);

    if (FLASH_FACTORY_SIGN == flashFactorySign)                      //表示已经过出厂设置
    {
        thermostat->theromstatparam = Thermostat_GetPIDParam(thermostat);
    }
    else                       //未设置,使用默认值，并写入出厂标志
    {
        Thermostat_SetPIDParam(thermostat, kDefaultThermostatParam);

        flashFactorySign = FLASH_FACTORY_SIGN;
        memcpy(buffer, &flashFactorySign, THERMOSTAT_FACTORY_SIGN_FLASH_LEN);
        McuFlash_Write(THERMOSTAT_FACTORY_SIGN_FLASH_BASE_ADDR + THERMOSTAT_FACTORY_SIGN_FLASH_LEN * thermostat->number,
        THERMOSTAT_FACTORY_SIGN_FLASH_LEN, buffer);
    }
}

/**
 * @brief 获取恒温状态
 * @param
 */
ThermostatStatus Thermostat_GetStatus(Thermostat *thermostat)
{
    return (thermostat->status);
}

float Thermostat_GetCurrentTemp(Thermostat *thermostat)
{
    float temp = 0;
    if (thermostat->tempIndex != TEMP_WITHOUT)
    {
        temp = TempCollecterManager_GetTemp(thermostat->tempIndex);
    }
    return temp;
}

static Bool Thermostat_IsSupportHeater(Thermostat *thermostat)
{
    if (thermostat->heaterTotal > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static Bool Thermostat_IsSupportRefrigerator(Thermostat *thermostat)
{
    if (thermostat->refrigeratorTotal > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void Thermostat_SetHeaterOutput(Thermostat *thermostat, float level)
{
    for (Uint8 i = 0; i < thermostat->heaterTotal; i++)
    {
        ThermostatDeviceManager_SetOutput(thermostat->heaterIndex[i], level);
    }
}

Bool Thermostat_SetSingleRefrigeratorOutput(Thermostat *thermostat, Uint8 fanIndex, float level)
{
    Bool ret = FALSE;
    if (fanIndex < thermostat->refrigeratorTotal)
    {
        ret = ThermostatDeviceManager_SetOutput(thermostat->refrigeratorIndex[fanIndex], level);
    }
    else
    {
        TRACE_ERROR("\n SetOutput  No. %d Refrigerator.", fanIndex);
    }
    return ret;
}

void Thermostat_SetRefrigeratorOutput(Thermostat *thermostat, float level)
{
    for (Uint8 i = 0; i < thermostat->refrigeratorTotal; i++)
    {
        ThermostatDeviceManager_SetOutput(thermostat->refrigeratorIndex[i], level);
    }
}

/**
 * @brief 开始恒温
 * @param
 */
int Thermostat_Start(Thermostat *thermostat, ThermostatMode mode, float targetTemp,
        float toleranceTemp, float timeout)
{
    if(THERMOSTAT_BUSY == Thermostat_GetStatus(thermostat))
    {
        TRACE_ERROR("\n Thermostat busy error");
        return DSCP_BUSY;
    }
    if( Thermostat_GetCurrentTemp(thermostat) <= TEMP_UNUSUAL_VALUE
            || Thermostat_GetCurrentTemp(thermostat) >= TEMP_WARNING_VALUE)
    {
        TRACE_ERROR("\n Thermostat temperature unusual error");
        return DSCP_ERROR;
    }
    if ((mode == THERMOSTAT_MODE_AUTO || mode == THERMOSTAT_MODE_HEATER)
            && FALSE == Thermostat_IsSupportHeater(thermostat))
    {
        TRACE_ERROR("\n Thermostat start parametric error, without heater");
        return DSCP_ERROR_PARAM;
    }
    if ((mode == THERMOSTAT_MODE_AUTO || mode == THERMOSTAT_MODE_REFRIGERATE)
            && FALSE == Thermostat_IsSupportRefrigerator(thermostat))
    {
        TRACE_ERROR("\n Thermostat start parametric error, without refrigerator");
        return DSCP_ERROR_PARAM;
    }
    if (targetTemp > TEMP_UNUSUAL_VALUE && targetTemp < TEMP_WARNING_VALUE && toleranceTemp >= 0 && timeout > 0)
    {
        thermostat->sumError = 0;
        thermostat->lastError = 0;

        thermostat->isThermostatReached = FALSE;
        thermostat->isRequestStop = FALSE;
        thermostat->mode = mode;
        thermostat->status = THERMOSTAT_BUSY;
        thermostat->targetTemp = targetTemp;
        thermostat->toleranceTemp = toleranceTemp;
        thermostat->timeout = timeout;
        thermostat->alreadyTime = 0;
        thermostat->isTempFault = FALSE;
        thermostat->tempFaultCnt = 0;

        if(Thermostat_GetCurrentTemp(thermostat) >= thermostat->targetTemp)
        {
            thermostat->isGreaterThanObjTemp = TRUE;
        }
        else
        {
            thermostat->isGreaterThanObjTemp = FALSE;
        }
        DncpStack_ClearBufferedEvent();

        switch (thermostat->mode)
        {
        case THERMOSTAT_MODE_AUTO:
            Thermostat_SetHeaterOutput(thermostat, 0);
            Thermostat_SetRefrigeratorOutput(thermostat, 0);
            TRACE_INFO("\n mode: auto ,targetTemp:");
            break;

        case THERMOSTAT_MODE_HEATER:
            Thermostat_SetHeaterOutput(thermostat, 0);
            TRACE_INFO("\n mode: heater ,targetTemp:");
            break;

        case THERMOSTAT_MODE_REFRIGERATE:
            Thermostat_SetRefrigeratorOutput(thermostat, 0);
            TRACE_INFO("\n mode: refrigerate ,targetTemp:");
            break;
        }

        vTaskResume(thermostat->taskHandle);

        System_PrintfFloat(TRACE_LEVEL_INFO, targetTemp, 2);
        TRACE_INFO("  ,toleranceTemp:");
        System_PrintfFloat(TRACE_LEVEL_INFO, toleranceTemp, 2);
        TRACE_INFO("  ,timeout:");
        System_PrintfFloat(TRACE_LEVEL_INFO, timeout,3);
        return DSCP_OK;
    }
    else
    {
        TRACE_ERROR("Thermostat start parametric error\n");
        return DSCP_ERROR_PARAM;
    }
}
/**
 * @brief 打开停止发送事件功能
 */
void Thermostat_SendEventOpen(Thermostat *thermostat)
{
    thermostat->isSendEvent = TRUE;
}

/**
 * @brief 关闭停止发送事件功能
 */
void Thermostat_SendEventClose(Thermostat *thermostat)
{
    thermostat->isSendEvent = FALSE;
}

static void Thermostat_SendEvent(Thermostat *thermostat)
{
    if(TRUE == thermostat->isSendEvent)
    {
        Uint8 data[6] = {0};
        data[0] = thermostat->number;
        data[1] = thermostat->reportResult.result;
        memcpy(data + 2, &thermostat->reportResult.temperature, sizeof(thermostat->reportResult.temperature));
        DncpStack_SendEvent(DSCP_EVENT_TCI_THERMOSTAT_RESULT,
                (void *)data, sizeof(data));
        DncpStack_BufferEvent(DSCP_EVENT_TCI_THERMOSTAT_RESULT,
                (void *)data, sizeof(data));
    }
}

Bool Thermostat_RequestStop(Thermostat *thermostat)
{
    if (THERMOSTAT_BUSY == thermostat->status)
    {
        thermostat->isRequestStop = TRUE;
        TRACE_INFO("\n thermostat is request stop.");
        return TRUE;
    }
    else
    {
        TRACE_ERROR("\n Failed to stop the thermostat, thermostat for idle.");
        return FALSE;
    }
}

static void Thermostat_Stop(Thermostat *thermostat)
{
    switch (thermostat->mode)
     {
         case THERMOSTAT_MODE_AUTO:
             Thermostat_SetHeaterOutput(thermostat, 0);
             Thermostat_SetRefrigeratorOutput(thermostat, 0);
             break;

         case THERMOSTAT_MODE_HEATER:
             Thermostat_SetHeaterOutput(thermostat, 0);
             break;

         case THERMOSTAT_MODE_REFRIGERATE:
             Thermostat_SetRefrigeratorOutput(thermostat, 0);
             break;
     }
    if (FALSE == thermostat->isTempFault)
    {
        thermostat->reportResult.result = THERMOSTAT_RESULT_STOPPED;
        Thermostat_SendEvent(thermostat);
    }
    else
    {
        TRACE_INFO("\n No stop event is sent");
        thermostat->isTempFault = FALSE;
    }
    TRACE_INFO("\n Stop Thermostat %d", thermostat->number);
    thermostat->status = THERMOSTAT_IDLE;
    vTaskSuspend(thermostat->taskHandle);
}

/**
 * @brief 恒温计算  位置式PID
 * @param
 */
static float Thermostat_LocPIDCalc(Thermostat *thermostat, float temperature)
{
    float nowerror, derror;
    float pidout;

    nowerror = thermostat->targetTemp - temperature;  //当前控制误差

    if (nowerror > 15.0)
    {
        pidout = 1000;
    }
    else if (nowerror < -15.0)
    {
        pidout = -1000;
    }
    else
    {
        thermostat->sumError += nowerror;                       //计算当前误差的积分值
        derror = nowerror - thermostat->lastError;            //计算当前误差的微分值
        thermostat->lastError = nowerror;                       //保存误差值

        pidout = (thermostat->theromstatparam.proportion * nowerror        //比例项
        + thermostat->theromstatparam.integration * thermostat->sumError             //积分项
        + thermostat->theromstatparam.differential * derror);            //微分项

        if (pidout > 1000)
            pidout = 1000;
        else if (pidout < -1000)
            pidout = -1000;
        TRACE_MARK("\n pidout:");
        System_PrintfFloat(TRACE_LEVEL_MARK, pidout, 4);
    }
    return pidout;
}

/**
 * @brief 恒温控制
 * @param
 */
static void Thermostat_ControlTemp(Thermostat *thermostat)
{
    float leve;

    thermostat->reportResult.temperature = Thermostat_GetCurrentTemp(thermostat);
    TRACE_MARK("\n targetTemp:");
    System_PrintfFloat(TRACE_LEVEL_MARK, thermostat->targetTemp, 1);
    TRACE_MARK("  CurrentTemp:");
    System_PrintfFloat(TRACE_LEVEL_MARK, thermostat->reportResult.temperature, 1);

    switch (thermostat->mode)
    {
    case THERMOSTAT_MODE_AUTO:
        leve = Thermostat_LocPIDCalc(thermostat, thermostat->reportResult.temperature)
                / 1000;
        if (leve < 0)
        {
            Thermostat_SetHeaterOutput(thermostat, 0);
            Thermostat_SetRefrigeratorOutput(thermostat, -leve);
        }
        else
        {
            Thermostat_SetHeaterOutput(thermostat, leve);
            Thermostat_SetRefrigeratorOutput(thermostat, 0);
        }
        break;

    case THERMOSTAT_MODE_HEATER:
        leve = Thermostat_LocPIDCalc(thermostat, thermostat->reportResult.temperature)
                / 1000;
        if (leve >= 0)
        {
            Thermostat_SetHeaterOutput(thermostat, leve);
        }
        else
        {
            Thermostat_SetHeaterOutput(thermostat, 0);
        }
        break;
        //纯制冷模式
    case THERMOSTAT_MODE_REFRIGERATE:
        leve = Thermostat_LocPIDCalc(thermostat, thermostat->reportResult.temperature)
                / 1000;
        if (leve <= 0)
        {
            Thermostat_SetRefrigeratorOutput(thermostat, -leve);
        }
        else
        {
            Thermostat_SetRefrigeratorOutput(thermostat, 0);
        }
        break;
    default:
        break;
    }
}

/**
 // * @brief 恒温控制任务处理
 // * @param
 // */
static void Thermostat_Handle(void *argument)
{
    Thermostat *thermostat;
    thermostat = (Thermostat *)argument;
    vTaskSuspend(thermostat->taskHandle);
    while(1)
    {
        vTaskDelay(400 / portTICK_RATE_MS);
        float currentThermostatTemp = Thermostat_GetCurrentTemp(thermostat);
        thermostat->reportResult.temperature = currentThermostatTemp;

        if(currentThermostatTemp <= TEMP_UNUSUAL_VALUE
                    || currentThermostatTemp >= TEMP_WARNING_VALUE)// 温度异常判断
        {
            thermostat->tempFaultCnt++;
            if (thermostat->tempFaultCnt >= 10)
            {
                thermostat->tempFaultCnt = 0;
                thermostat->reportResult.result = THERMOSTAT_RESULT_FAILED;   // 温度传感器温度异常。
                TRACE_ERROR("\n THERMOSTAT_RESULT_FAILED");
                Thermostat_SendEvent(thermostat);
                thermostat->isRequestStop = TRUE;
                thermostat->isTempFault = TRUE;
            }
        }
        else    // 温度正常
        {
            thermostat->tempFaultCnt = 0;
            if (FALSE == thermostat->isThermostatReached)
            {
                thermostat->alreadyTime += 0.5;
                if ((TRUE == thermostat->isGreaterThanObjTemp &&
                        thermostat->reportResult.temperature <= (thermostat->targetTemp + thermostat->toleranceTemp))
                        || (FALSE == thermostat->isGreaterThanObjTemp &&
                                thermostat->reportResult.temperature >= (thermostat->targetTemp - thermostat->toleranceTemp)))
                {
                    thermostat->isThermostatReached = TRUE;
                    thermostat->reportResult.result = THERMOSTAT_RESULT_REACHED;
                    TRACE_ERROR("\n THERMOSTAT_RESULT_REACHED");
                    Thermostat_SendEvent(thermostat);
                }
                else if (thermostat->alreadyTime >= thermostat->timeout)
                {
                    thermostat->isThermostatReached = TRUE;
                    thermostat->reportResult.result = THERMOSTAT_RESULT_TIMEOUT;//恒温超时，指定时间内仍未达到目标温度。
                    TRACE_ERROR("\n THERMOSTAT_RESULT_TIMEOUT");
                    Thermostat_SendEvent(thermostat);
                }
            }

            Thermostat_ControlTemp(thermostat);   // 恒温控制执行
            if (FALSE == thermostat->isThermostatReached)
            {
                // 如果当前温度小于目标温度 ，并且模式是制冷，直接返回reached
                if(THERMOSTAT_MODE_REFRIGERATE == thermostat->mode)
                {
                    if((thermostat->targetTemp - currentThermostatTemp) >= 0.000001)
                    {
                        thermostat->isThermostatReached = TRUE;
                        thermostat->reportResult.result = THERMOSTAT_RESULT_REACHED;
                        TRACE_ERROR("\n THERMOSTAT_RESULT_REACHED");
                        Thermostat_SendEvent(thermostat);
                    }
                }
                // 如果当前温度大于目标温度，并且模式是纯加热模式， 直接返回reached
                if(THERMOSTAT_MODE_HEATER == thermostat->mode)
                {
                    if((currentThermostatTemp - thermostat->targetTemp) >= 0.000001)
                    {
                        thermostat->isThermostatReached = TRUE;
                        thermostat->reportResult.result = THERMOSTAT_RESULT_REACHED;
                        TRACE_ERROR("\n THERMOSTAT_RESULT_REACHED");
                        Thermostat_SendEvent(thermostat);
                    }
                }
            }
        }
        //用户手动停止恒温
        if(TRUE == thermostat->isRequestStop)
        {
            thermostat->isRequestStop = FALSE;
            Thermostat_Stop(thermostat);
        }
    }
}

ThermostatParam Thermostat_GetCurrentPIDParam(Thermostat *thermostat)
{
    return thermostat->theromstatparam;
}

void Thermostat_SetCurrentPIDParam(Thermostat *thermostat, ThermostatParam thermostatparam)
{
    thermostat->theromstatparam = thermostatparam;

    TRACE_INFO("\n SetCurrentPIDParam p =");
    System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.proportion, 3);
    TRACE_INFO("\n i =");
    System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.integration, 3);
    TRACE_INFO("\n d =");
    System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.differential, 3);
}

void Thermostat_TempMonitor(Thermostat *thermostat)
{
    if(Thermostat_GetCurrentTemp(thermostat) >= TEMP_WARNING_VALUE)
    {
        thermostat->isTempWaring = TRUE;
        if (THERMOSTAT_BUSY == Thermostat_GetStatus(thermostat))
        {
//            Thermostat_RequestStop(thermostat);
            while (THERMOSTAT_IDLE != Thermostat_GetStatus(thermostat))
            {
                System_Delay(5);
            }
        }
        TRACE_ERROR("\n No %d Thermostat Heater close", thermostat->number);
        for (Uint8 i = 0; i < thermostat->heaterTotal; i++)
        {
            if(TRUE == ThermostatDeviceManager_IsOpen(thermostat->heaterIndex[i]))
            {
                ThermostatDeviceManager_SetOutput(thermostat->heaterIndex[i], 0);
            }
        }
        TRACE_ERROR("\n No %d Thermostat Fan open", thermostat->number);
        for (Uint8 i = 0; i < thermostat->refrigeratorTotal; i++)
        {
            if(FALSE == ThermostatDeviceManager_IsOpen(thermostat->refrigeratorIndex[i]))
            {
                ThermostatDeviceManager_SetOutput(thermostat->refrigeratorIndex[i], 1);
            }
        }
    }
    if(Thermostat_GetCurrentTemp(thermostat) < TEMP_WARNING_VALUE && TRUE == thermostat->isTempWaring)
    {
        thermostat->isTempWaring = FALSE;
        TRACE_ERROR("\n No %d Thermostat Fan close", thermostat->number);
        for (Uint8 i = 0; i < thermostat->refrigeratorTotal; i++)
        {
            ThermostatDeviceManager_SetOutput(thermostat->refrigeratorIndex[i], 0);
        }
    }

    if(Thermostat_GetCurrentTemp(thermostat) < TEMP_UNUSUAL_VALUE)
    {
        if (THERMOSTAT_BUSY == Thermostat_GetStatus(thermostat))
        {
//            Thermostat_RequestStop(thermostat);
            while (THERMOSTAT_IDLE != Thermostat_GetStatus(thermostat))
            {
                System_Delay(5);
            }
        }
    }
}
