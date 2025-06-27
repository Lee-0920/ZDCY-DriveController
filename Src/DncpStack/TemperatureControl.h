/**
 * @file
 * @brief 温度接口实现头文件
 * @details
 * @version 1.0.0
 * @author lemon.xiaoxun
 * @date 2016-5-27
 */


#ifndef TEMPERATURECONTRO_H_
#define TEMPERATURECONTRO_H_

#include "LuipApi/TemperatureControlInterface.h"
#include "DNCP/App/DscpDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

void TempControl_GetCalibrateFactor(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_SetCalibrateFactor(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_GetTemperature(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_GetThermostatParam(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_SetThermostatParam(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_GetThermostatStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_StartThermostat(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_StopThermostat(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_SetTemperatureNotifyPeriod(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_BoxFanSetOutput(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_ThermostatFanSetOutput(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_GetHeaterMaxDutyCycle(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_SetHeaterMaxDutyCycle(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_GetCurrentThermostatParam(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_SetCurrentThermostatParam(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_GetTotalThermostat(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_GetEnvTemperature(DscpDevice* dscp, Byte* data, Uint16 len);
void TempControl_FanSetOutput(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_Initialize(DscpDevice* dscp, Byte* data,Uint16 len);
void TempControl_TempCalibrate(DscpDevice* dscp, Byte* data,Uint16 len);

#define CMD_TABLE_TEMPERATURECONTROL \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_CALIBRATE_FACTOR, TempControl_GetCalibrateFactor), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_SET_CALIBRATE_FACTOR, TempControl_SetCalibrateFactor), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_TEMPERATURE, TempControl_GetTemperature), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_THERMOSTAT_PARAM, TempControl_GetThermostatParam), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_SET_THERMOSTAT_PARAM, TempControl_SetThermostatParam),\
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_THERMOSTAT_STATUS, TempControl_GetThermostatStatus), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_START_THERMOSTAT, TempControl_StartThermostat), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_STOP_THERMOSTAT, TempControl_StopThermostat), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_SET_TEMPERATURE_NOTIFY_PERIOD, TempControl_SetTemperatureNotifyPeriod), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_TURN_BOXFAN, TempControl_BoxFanSetOutput), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_TURN_THERMOSTAT_FAN, TempControl_ThermostatFanSetOutput), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_HEATER_MAX_DUTY_CYCLE, TempControl_GetHeaterMaxDutyCycle), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_SET_HEATER_MAX_DUTY_CYCLE, TempControl_SetHeaterMaxDutyCycle), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_CURRENT_THERMOSTAT_PARAM, TempControl_GetCurrentThermostatParam), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_SET_CURRENT_THERMOSTAT_PARAM, TempControl_SetCurrentThermostatParam), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_TOTAL_THERMOSTAT, TempControl_GetTotalThermostat), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_GET_ENV_TEMPERATURE, TempControl_GetEnvTemperature), \
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_TURN_FAN, TempControl_FanSetOutput),\
    DSCP_CMD_ENTRY(DSCP_CMD_TCI_INIT, TempControl_Initialize),\
	DSCP_CMD_ENTRY(DSCP_CMD_TCI_TEMP_CALIBRATE, TempControl_TempCalibrate)

#ifdef __cplusplus
}
#endif

#endif /* TEMPERATURECONTRO_H_ */

