/**
 * @file
 * @brief 设备状态接口实现
 * @details
 * @version 1.0.0
 * @author nick.shushaohua
 * @date Dec 29, 2014
 */
#include "DeviceStatus.h"
#include <string.h>
#include "Common/Utils.h"
#include "Console/Console.h"
#include "DNCP/App/DscpSysDefine.h"
#include "Tracer/Trace.h"
#include "SolenoidValve/ValveManager.h"
#include "PeristalticPump/PeristalticPumpManager.h"
#include "DeviceUpdate/UpdateHandle.h"
#include "DeviceIndicatorLED.h"
#include "DncpStack/DncpStack.h"
#include "TemperatureControl/ThermostatDeviceManager.h"
#include "TemperatureControl/ThermostatManager.h"
#include "PeristalticPump/DCMotorManager.h"
#include "PeristalticPump/DisplacementMotorManager.h"

/**
 * @brief 闪烁设备指示灯。
 * @details 本命令只是抽象定义了设备的指示操作，而未指定是哪一盏灯，
 *  具体的指示灯（一盏或多盏）由设备实现程序决定。
 *  <p>闪烁方式由参数指定。通过调节参数，可设置LED灯为常亮或常灭：
 *  - onTime 为 0 表示灭灯
 *  - offTime 为 0 表示亮灯
 *  - onTime 和 offTime 都为0 时，不动作
 *  <p> 当持续时间结束之后，灯的状态将返回系统状态，受系统控制。
 * @param duration Uint32，持续时间，单位为毫秒。0 表示不起作用，-1 表示一直持续。
 * @param onTime Uint16，亮灯的时间，单位为毫秒。
 * @param offTime Uint16，灭灯的时间，单位为毫秒。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_NOT_SUPPORTED 操作不被支持；
 *
 */
void DeviceStatus_BlinkDevice(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    int size = 0;
    Uint16 onTime;
    Uint16 offTime;
    Uint32 duration;

    size = sizeof(Uint16) * 2 + sizeof(Uint32);
    if (len > size)
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(&duration, data, sizeof(Uint32));
        size = sizeof(Uint32);
        memcpy(&onTime, data + size, sizeof(Uint16));
        size += sizeof(Uint16);
        memcpy(&offTime, data + size, sizeof(Uint16));
        DeviceIndicatorLED_SetBlink(duration, onTime, offTime);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询程序当前的运行模式。
 * @details 本操作通常用于升级管理程序，以确保设备处于期望的运行模式。
 * @return 运行模式，Uint8。请参考 @ref DeviceRunMode 。
 * @note App 模式和 Updater 模式都支持本命令。见： @ref DSCP_CMD_DUI_GET_RUN_MODE ，
 *  注意这两条命令的码值是一致，只是名称不同而已。
 */
void DeviceStatus_GetRunMode(DscpDevice* dscp, Byte* data, Uint16 len)
{
    DeviceRunMode mode = UpdateHandle_GetRunMode();
    TRACE_MARK("\n mode:%d", mode);
    DscpDevice_SendResp(dscp, &mode, sizeof(DeviceRunMode));
}

/**
 * @brief 查询设备的电源供应类型。
 * @details 这里的电源供给类型是指设备实际的电源类型。
 * @return 电源供应类型，Uint8。请参考 @ref DevicePowerSupplyType 。
 */
void DeviceStatus_GetPowerSupplyType(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint8 powerSupply = DEVICE_POWER_TYPE_ONGOING_AC;
    DscpDevice_SendResp(dscp, &powerSupply, sizeof(powerSupply));
}

/**
 * @brief 设备初始化。
 * @details 恢复业务功能到初始状态。
 *
 */
void DeviceStatus_Initialize(DscpDevice* dscp, Byte* data, Uint16 len)
{
    short ret = DSCP_OK;
    TRACE_INFO("\r\n ************************** DeviceStatus_Initialize");
    //以下操作可能会因为没有启动而控制台出现错误提醒。
    PeristalticPumpManager_Restore();//依次关闭所有的泵
    ValveManager_SetValvesMap(0);

    DisplacementMotor_Restore(1);
    DCMotorManager_Restore();

    DscpDevice_SendStatus(dscp, ret);
}

void DeviceStatus_Reset(DscpDevice* dscp, Byte* data, Uint16 len)
{
    TRACE_INFO("\r\n DeviceStatus_Reset");
    System_Delay(10);
    System_Reset();
}

/**
 * @brief 复位事件发生 向上位机报告
 * @param 无
 */
void DeviceStatus_ReportResetEvent(DeviceResetCause resetCause)
{
    DncpStack_SendEvent(DSCP_EVENT_DSI_RESET, &resetCause,
            sizeof(DeviceResetCause));
    TRACE_INFO(" \n DEVICE_RESET");
}

