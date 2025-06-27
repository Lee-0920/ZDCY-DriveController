
/**
 * @file
 * @brief 温度接口实现
 * @details
 * @version 1.0.0
 * @author lemon.xiaoxun
 * @date 2016-5-27
 */

#include <string.h>
#include "TemperatureControl.h"
#include "Common/Utils.h"
#include "DNCP/App/DscpSysDefine.h"
#include "Tracer/Trace.h"
#include "Driver/System.h"
#include "LuipApi/TemperatureControlInterface.h"
#include "FreeRTOS.h"
#include "TemperatureControl/ThermostatDeviceManager.h"
#include "TemperatureControl/ThermostatManager.h"

/**
 * @brief 查询温度传感器的校准系数。
 * @param index Uint8，要查询的恒温器索引。
 * @return 负输入分压，Float32。
 * @return 参考电压 ，Float32。
 * @return 校准电压 Float32。
 * @see DSCP_CMD_TCI_SET_CALIBRATE_FACTOR
 */
void TempControl_GetCalibrateFactor(DscpDevice* dscp, Byte* data, Uint16 len)
{
    TempCalibrateParam calibrateFactor;
    Uint8 index = 0;

    if (sizeof(index) == len)
    {
        memcpy(&index, data, sizeof(index));
        calibrateFactor = ThermostatManager_GetCalibrateFactor(index);
        TRACE_INFO("\n %s Thermostat \n negativeInput = ", ThermostatManager_GetName(index));
        System_PrintfFloat(TRACE_LEVEL_INFO, calibrateFactor.negativeInput, 8);
        TRACE_INFO("\n vref = ");
        System_PrintfFloat(TRACE_LEVEL_INFO, calibrateFactor.vref, 8);
        TRACE_INFO("\n vcal = ");
        System_PrintfFloat(TRACE_LEVEL_INFO, calibrateFactor.vcal, 8);
    }
    else
    {
        TRACE_ERROR("\nGetCalibrateFactor Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    // 发送回应
    DscpDevice_SendResp(dscp, &calibrateFactor, sizeof(calibrateFactor));
}

/**
 * @brief 设置温度传感器的校准系数。
 * @details 因为个体温度传感器有差异，出厂或使用时需要校准。该参数将永久保存。
 *   @param index Uint8，要设置的恒温器索引。
 *   @param negativeInput Float32，负输入分压 。
 *   @param vref Float32，参考电压。
 *   @param vcal Float32，校准电压。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
void TempControl_SetCalibrateFactor(DscpDevice* dscp, Byte* data, Uint16 len)
{
    TempCalibrateParam calibratefactor;
    int size = 0;
    Uint16 ret = DSCP_OK;
    Uint8 index = 0;

    //设置数据正确性判断
    size = sizeof(TempCalibrateParam) + sizeof(index);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n SetCalibrateFactor Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    //修改并保存
    if (DSCP_OK == ret)
    {
        memcpy(&index, data, sizeof(index));
        memcpy(&calibratefactor, data + sizeof(index), sizeof(TempCalibrateParam));
        if (FALSE == ThermostatManager_SetCalibrateFactor(index, calibratefactor))
        {
            ret = DSCP_ERROR;
        }
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询当前恒温器温度。
 * @param index Uint8，要查询的恒温器索引。
 * @return 当前恒温室温度，Float32。
 */
void TempControl_GetTemperature(DscpDevice* dscp, Byte* data, Uint16 len)
{
    float temp = 0;
    Uint8 index = 0;

    if (sizeof(index) == len)
    {
        memcpy(&index, data, sizeof(index));
        temp = ThermostatManager_GetCurrentTemp(index);
        TRACE_INFO("\n %s Thermostat Temp : ", ThermostatManager_GetName(index));
        System_PrintfFloat(TRACE_LEVEL_INFO, temp, 1);
    }
    else
    {
        TRACE_ERROR("\nGetTemperature Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    // 发送回应
    DscpDevice_SendResp(dscp, (void *) &temp, sizeof(temp));
}

/**
 * @brief 查询恒温控制参数。
 * @param index Uint8，要查询的恒温器索引。
 * @return 恒温控制参数，格式如下：
 *  - proportion Float32，PID的比例系数。
 *  - integration Float32，PID的积分系数。
 *  - differential Float32，PID的微分系数。
 * @see DSCP_CMD_TCI_SET_THERMOSTAT_PARAM
 */
void TempControl_GetThermostatParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index = 0;
    ThermostatParam thermostatparam;

    if (sizeof(index) == len)
    {
        memcpy(&index, data, sizeof(index));
        thermostatparam = ThermostatManager_GetPIDParam(index);
        TRACE_INFO("\n %s Thermostat p =", ThermostatManager_GetName(index));
        System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.proportion, 3);
        TRACE_INFO("\n i =");
        System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.integration, 3);
        TRACE_INFO("\n d =");
        System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.differential, 3);
    }
    else
    {
        TRACE_ERROR("\n GetThermostatParam Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    // 发送回应
    DscpDevice_SendResp(dscp, (void *) &thermostatparam,
            sizeof(thermostatparam));
}

/**
 * @brief 设置恒温控制参数。
 * @details 该参数将永久保存。系统启动时同步到当前恒温控制参数上。
 * @param index Uint8，要设置的恒温器索引。
 * @param proportion Float32，PID的比例系数。
 * @param integration Float32，PID的积分系数。
 * @param differential Float32，PID的微分系数。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
void TempControl_SetThermostatParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ThermostatParam thermostatparam;
    int size = 0;
    Uint16 ret = DSCP_OK;
    Uint8 index = 0;
    //设置数据正确性判断
    size = sizeof(thermostatparam) + sizeof(index);
    if (len > size)
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n SetThermostatParam Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    if (DSCP_OK == ret)
    {
        memcpy(&index, data, sizeof(index));
        memcpy(&thermostatparam, data + sizeof(index), sizeof(ThermostatParam));
        if (FALSE == ThermostatManager_SetPIDParam(index, thermostatparam))
        {
            ret = DSCP_ERROR;
        }
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询恒温器的工作状态。
 * @param index Uint8，要查询的恒温器索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_IDLE 空闲；
 *  - @ref DSCP_BUSY 忙碌，需要停止后才能做下一个动作；
 */
void TempControl_GetThermostatStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 index = 0;
    StatusCode statusCode = DSCP_BUSY;
    if (sizeof(index) == len)
    {
        memcpy(&index, data, sizeof(index));
        if (THERMOSTAT_IDLE == ThermostatManager_GetStatus(index))
        {
            TRACE_INFO("\n Get %s Thermostat Status IDLE \n", ThermostatManager_GetName(index));
            statusCode = DSCP_IDLE;
        }
        else
        {
            TRACE_INFO("\n Get %s Thermostat Status BUSY \n", ThermostatManager_GetName(index));
            statusCode = DSCP_BUSY;
        }
    }
    else
    {
        TRACE_ERROR("\n GetThermostatParam Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    DscpDevice_SendStatus(dscp, statusCode);
}

/**
 * @brief 开始恒温。
 * @details 恒温开始后，系统将根据设定的恒温控制参数进行自动温度控制，尝试达到指定温度。
 *  不管成功与否，操作结果都将以事件的形式上传给上位机，但恒温器将继续工作，
 *  直到接收到DSCP_CMD_TCI_STOP_THERMOSTAT才停止。关联的事件有：
 *   - @ref DSCP_EVENT_TCI_THERMOSTAT_RESULT
 * @param index Uint8，要操作的恒温器索引。
 * @param mode Uint8，恒温模式，支持的模式见： @ref ThermostatMode 。
 * @param targetTemp Float32，恒温目标温度。
 * @param toleranceTemp Float32，容差温度，与目标温度的差值在该参数范围内即认为到达目标温度。
 * @param timeout Float32，超时时间，单位为秒。超时仍未达到目标温度，也将返回结果事件。
 * @param isExpectEvent bool,是否等待事件。
 * @return 状态回应，Uint16，支持的状态有：
 *   - @ref DSCP_OK  操作成功；
 *   - @ref DSCP_BUSY 操作失败，如恒温已经工作，需要先停止；
 *   - @ref DSCP_ERROR_PARAM 恒温参数错误；
 *   - @ref DSCP_ERROR 温度传感器异常；
 * @note 该命令将立即返回，恒温完成将以事件的形式上报。
 */
void TempControl_StartThermostat(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ThermostatMode mode;         //恒温
    float targetTemp;           // 目标温度
    float toleranceTemp;        // 容差
    float timeout;              // 超时时间
    Uint8 index = 0;
    Bool isExpectEvent;

    int size = 0;
    unsigned short ret = DSCP_OK;
    //设置数据正确性判断
    size =  sizeof(ThermostatMode) + sizeof(float) * 3 + sizeof(index) + sizeof(Bool);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n StartThermostat Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    else
    {
        //修改并保存
        memcpy(&index, data, sizeof(index));
        size = sizeof(index);

        memcpy(&mode, data + size, sizeof(ThermostatMode));
        size += sizeof(ThermostatMode);

        memcpy(&targetTemp, data + size, sizeof(float));
        size += sizeof(float);

        memcpy(&toleranceTemp, data + size, sizeof(float));
        size += sizeof(float);

        memcpy(&timeout, data + size, sizeof(float));
        size += sizeof(float);

        memcpy(&isExpectEvent, data + size, sizeof(Bool));
        if (TRUE == isExpectEvent)
        {
            ThermostatManager_SendEventOpen(index);
        }
        else
        {
            ThermostatManager_SendEventClose(index);
        }
        ret = (Uint16)ThermostatManager_Start(index, mode, targetTemp, toleranceTemp, timeout);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 停止恒温控制。
 * @details 停止后，加热器和冷却器将不工作。
 * @param index Uint8，要操作的恒温器索引。
 * @param isExpectEvent bool,是否等待事件。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @note 该命令将立即返回。
 */
void TempControl_StopThermostat(DscpDevice* dscp, Byte* data, Uint16 len)
{
    unsigned short ret = DSCP_ERROR;
    Uint8 index = 0;
    Bool isExpectEvent;

    int size = sizeof(index) + sizeof(Bool);
    if (size == len)
    {
        memcpy(&index, data, sizeof(index));
        size = sizeof(index);

        memcpy(&isExpectEvent, data + size, sizeof(Bool));

        if (TRUE == isExpectEvent)
        {
            ThermostatManager_SendEventOpen(index);
        }
        else
        {
            ThermostatManager_SendEventClose(index);
        }
        if(TRUE == ThermostatManager_RequestStop(index))
        {
            ret = DSCP_OK;
        }
    }
    else
    {
        TRACE_ERROR("\n StopThermostat Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 设置温度上报周期。
 * @details 系统将根据设定的周期，定时向上发出温度上报事件。
 * @param period Float32，温度上报周期，单位为秒。0表示不需要上报，默认为0。
 * @see DSCP_EVENT_TCI_TEMPERATURE_NOTICE
 * @note 所设置的上报周期将在下一次启动时丢失，默认为0，不上报。
 */
void TempControl_SetTemperatureNotifyPeriod(DscpDevice* dscp, Byte* data,
        Uint16 len)
{
    Float32 period;
    int size = 0;
    Uint16 ret = DSCP_OK;
    //设置数据正确性判断
    size = sizeof(period);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n SetTemperatureNotifyPeriod Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    //修改并保存
    if (DSCP_OK == ret)
    {
        //修改并保存
        memcpy(&period, data, sizeof(period));
        ThermostatManager_SetTempReportPeriod(period);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 设置机箱风扇。
 * @details 根据设定的占空比，调节风扇速率
 * @param level float ,风扇速率，对应高电平占空比。默认为0，机箱风扇关闭。
 * @see
 * @note
 */
void TempControl_BoxFanSetOutput(DscpDevice* dscp, Byte* data,
        Uint16 len)
{
    float level= 0.0;
    int size = 0;
    unsigned short ret = DSCP_OK;   // 默认系统正常
    // 数据正确性判断
    size = sizeof(level);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n BoxFanSetOutput Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    // 设置机箱风扇占空比 0为关
    if (DSCP_OK == ret)
    {
       memcpy(&level, data, sizeof(level));
       TRACE_DEBUG("\n %s Device  SetOutput :", ThermostatDeviceManager_GetName(BOX_FAN));
       System_PrintfFloat(TRACE_LEVEL_DEBUG, level * 100, 3);
       TRACE_DEBUG(" %%");
       if (FALSE == ThermostatDeviceManager_SetOutput(BOX_FAN, level))
       {
           ret = DSCP_ERROR;
       }
    }

    DscpDevice_SendStatus(dscp, ret);// 发送状态回应
}

/** * @brief 设置恒温器风扇。
 * @details 根据设定的占空比，调节风扇速率
 * @param thermostatIndex Uint8，要设置的恒温器索引。
   @param fanIndex Uint8，要设置的恒温器索引。
 * @param level float ,风扇速率，对应高电平占空比。默认为0，恒温器风扇关闭。
 * @see
 * @note
 */
void TempControl_ThermostatFanSetOutput(DscpDevice* dscp, Byte* data,
        Uint16 len)
{
    Uint8 thermostatIndex = 0, fanIndex = 0;
    float level= 0.0;
    int size = 0;
    unsigned short ret = DSCP_OK;   // 默认系统正常
    // 数据正确性判断
    size = sizeof(level) + sizeof(Uint8) * 2;
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n ThermostatFanSetOutput Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    // 设置恒温器风扇占空比 0为关
    if (DSCP_OK == ret)
    {
        for (int i = 0; i < len; i++)
        {
            TRACE_INFO(" %d", *(data + i));
        }
       memcpy(&thermostatIndex, data, sizeof(thermostatIndex));
       size = sizeof(thermostatIndex);
       memcpy(&fanIndex, data + size, sizeof(fanIndex));
       size += sizeof(fanIndex);
       memcpy(&level, data + size, sizeof(level));
       TRACE_DEBUG("\n %s Device %d SetOutput :", ThermostatManager_GetName(thermostatIndex), fanIndex);
       System_PrintfFloat(TRACE_LEVEL_DEBUG, level * 100, 5);
       TRACE_DEBUG(" %%");
       if (FALSE == ThermostatManager_SetSingleRefrigeratorOutput(thermostatIndex, fanIndex, level))
       {
           ret = DSCP_ERROR;
       }
    }

    DscpDevice_SendStatus(dscp, ret);// 发送状态回应
}

/**
 * @brief 获取恒温器加热丝输出的最大占空比。
 * @param index Uint8，要获取的恒温器索引。
 * @param maxDutyCycle float ,加热丝输出的最大占空比
 * @see DSCP_CMD_TCI_SET_HEATER_MAX_DUTY_CYCLE
 * @note
 */
void TempControl_GetHeaterMaxDutyCycle(DscpDevice* dscp, Byte* data,Uint16 len)
{
    Uint8 index = 0;
    float valve;

    if (sizeof(index) == len)
    {
       memcpy(&index, data, sizeof(index));
       valve = ThermostatManager_GetHeaterMaxDutyCycle(index);
       TRACE_INFO("\n %s Device  GetHeaterMaxDutyCycle :", ThermostatDeviceManager_GetName(index));
       System_PrintfFloat(TRACE_LEVEL_INFO, valve * 100, 3);
       TRACE_INFO(" %%");
    }
    else
    {
        TRACE_ERROR("\n GetHeaterMaxDutyCycle Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    DscpDevice_SendResp(dscp, (void *) &valve,
            sizeof(valve));
}

/**
 * @brief 设置恒温器加热丝输出的最大占空比。
 * @param index Uint8，要设置的恒温器索引。
 * @param maxDutyCycle float，加热丝输出的最大占空比
 * @see DSCP_CMD_TCI_GET_HEATER_MAX_DUTY_CYCLE
 */
void TempControl_SetHeaterMaxDutyCycle(DscpDevice* dscp, Byte* data,Uint16 len)
{
    Uint8 index = 0;
    float valve;
    int size = 0;
    unsigned short ret = DSCP_OK;

    size = sizeof(valve) + sizeof(index);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n SetHeaterMaxDutyCycle Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    if (DSCP_OK == ret)
    {
       memcpy(&index, data, sizeof(index));
       memcpy(&valve, data + sizeof(index), sizeof(valve));

       TRACE_INFO("\n %s Device  SetHeaterMaxDutyCycle :", ThermostatDeviceManager_GetName(index));
       System_PrintfFloat(TRACE_LEVEL_INFO, valve * 100, 3);
       TRACE_INFO(" %%");

       if (FALSE == ThermostatManager_SetMaxDutyCycle(index, valve))
       {
           ret = DSCP_ERROR;
       }
    }

    DscpDevice_SendStatus(dscp, ret);// 发送状态回应
}

/**
 * @brief 查询恒温器当前恒温控制参数。
 * @param index Uint8，要查询的恒温器索引。
 * @return 恒温控制参数，格式如下：
 *  - proportion Float32，PID的比例系数。
 *  - integration Float32，PID的积分系数。
 *  - differential Float32，PID的微分系数。
 * @see DSCP_CMD_TCI_SET_CURRENT_THERMOSTAT_PARAM
 */
void TempControl_GetCurrentThermostatParam(DscpDevice* dscp, Byte* data,Uint16 len)
{
    ThermostatParam thermostatparam;
    Uint8 index = 0;
    if (sizeof(index) == len)
    {
        memcpy(&index, data, sizeof(index));
        thermostatparam = ThermostatManager_GetCurrentPIDParam(index);

        TRACE_INFO("\n %s Thermostat Get Cur p =", ThermostatManager_GetName(index));
        System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.proportion, 3);
        TRACE_INFO("\n i =");
        System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.integration, 3);
        TRACE_INFO("\n d =");
        System_PrintfFloat(TRACE_LEVEL_INFO, thermostatparam.differential, 3);
    }
    else
    {
        TRACE_ERROR("\n GetCurrentThermostatParam Parame Len Error , right len : %d, cur len : %d", sizeof(index), len);
    }
    DscpDevice_SendResp(dscp, (void *) &thermostatparam,
            sizeof(thermostatparam));
}

/**
 * @brief 设置当前恒温控制参数。
 * @details 恒温系统将根据设置的参数进行温度调节。此参数上电时获取FLASH的PID。
 * @param index Uint8，要设置的恒温器索引。
 * @param proportion Float32，PID的比例系数。
 * @param integration Float32，PID的积分系数。
 * @param differential Float32，PID的微分系数。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
void TempControl_SetCurrentThermostatParam(DscpDevice* dscp, Byte* data,Uint16 len)
{
    ThermostatParam thermostatparam;
    int size = 0;
    Uint16 ret = DSCP_OK;
    Uint8 index = 0;

    size = sizeof(thermostatparam) + sizeof(index);
    if (len > size)
    {
        ret = DSCP_ERROR;
    }
    if (DSCP_OK == ret)
    {
        memcpy(&index, data, sizeof(index));
        memcpy(&thermostatparam, data + sizeof(index), sizeof(ThermostatParam));
        ThermostatManager_SetCurrentPIDParam(index, thermostatparam);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询系统支持的恒温器数目。
 * @return 总数目， Uint16。
 */
void TempControl_GetTotalThermostat(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 number = TOTAL_THERMOSTAT;
    TRACE_INFO("\n GetTotalThermostat %d", number);
    DscpDevice_SendResp(dscp, (void *) &number, sizeof(number));
}

/**
 * @brief 查询当前环境温度。
 * @return 当前恒温室温度，Float32。
 */
void TempControl_GetEnvTemperature(DscpDevice* dscp, Byte* data, Uint16 len)
{
    float temp = TempCollecterManager_GetEnvironmentTemp();
    TRACE_INFO("\n EnvironmentTemp : ");
    System_PrintfFloat(TRACE_LEVEL_INFO, temp, 1);
    DscpDevice_SendResp(dscp, (void *) &temp, sizeof(temp));
}

/** * @brief 设置独立风扇。
 * @details 根据设定的占空比，调节风扇速率
   @param fanIndex Uint8，要设置风扇索引。
 * @param level float ,风扇速率，对应高电平占空比。默认为0，风扇关闭。
 * @see
 * @note
 */
void TempControl_FanSetOutput(DscpDevice* dscp, Byte* data,
        Uint16 len)
{
    Uint8 fanIndex = 0;
    float level= 0.0;
    int size = 0;
    unsigned short ret = DSCP_OK;   // 默认系统正常
    // 数据正确性判断
    size = sizeof(level) + sizeof(fanIndex);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n FanSetOutput Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    // 设置机箱风扇占空比 0为关
    if (DSCP_OK == ret)
    {
       memcpy(&fanIndex, data, sizeof(fanIndex));
       size = sizeof(fanIndex);
       memcpy(&level, data + size, sizeof(level));
       TRACE_DEBUG("\n %s Device  SetOutput :", ThermostatDeviceManager_GetName(fanIndex));
       System_PrintfFloat(TRACE_LEVEL_DEBUG, level * 100, 3);
       TRACE_DEBUG(" %%");
       if (FALSE == ThermostatDeviceManager_SetOutput(fanIndex, level))
       {
           ret = DSCP_ERROR;
       }
    }

    DscpDevice_SendStatus(dscp, ret);// 发送状态回应
}

/** * @brief 温控器初始化
 * @details 温控器各功能模块初始化
 */
void TempControl_Initialize(DscpDevice* dscp, Byte* data, Uint16 len)
{
    short ret = DSCP_OK;

    ThermostatManager_RestoreInit();
    ThermostatDeviceManager_RestoreInit();

    DscpDevice_SendStatus(dscp, ret); // 发送状态回应
}

/**
 * @brief PT1000温度自动校准
 * @details 输入目标传感器的真实温度，下位机自动校准出参数
 * @param Uint8 index , PT1000温度传感器索引。
 * @param float temp , 真实温度。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
void TempControl_TempCalibrate(DscpDevice* dscp, Byte* data,Uint16 len)
{
	short ret = DSCP_OK;
    float realTemp= 0.0;
    Uint8 index = 0;
    int size = 0;
    size = sizeof(index) + sizeof(realTemp);
    if ((len > size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("\n FanSetOutput Parame Len Error , right len : %d, cur len : %d", size, len);
    }
    // 设置机箱风扇占空比 0为关
    if (DSCP_OK == ret)
    {
        memcpy(&index, data, sizeof(index));
        size = sizeof(index);
        memcpy(&realTemp, data + size, sizeof(realTemp));
        if(FALSE ==TempCollecterManager_AutoTempCalibrate(index,realTemp))
        {
        	ret = DSCP_ERROR;
        }
    }
	DscpDevice_SendStatus(dscp, ret);
}

