/*
 * MotorControl.h
 *
 *  Created on: 2018年3月8日
 *      Author: LIANG
 */

#ifndef SRC_DNCPSTACK_MOTORCONTROL_H_
#define SRC_DNCPSTACK_MOTORCONTROL_H_

#include "Common/Types.h"
#include "DNCP/App/DscpDevice.h"
#include "LuipApi/MotorControlInterface.h"

void MotorControl_GetTotalPumps(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_GetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_SetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_GetStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_GetMaxSteps(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_GetInitSteps(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_GetCurrentSteps(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_Start(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_Stop(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_Reset(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_GetSensorStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_DriverReinit(DscpDevice* dscp, Byte* data, Uint16 len);
void MotorControl_DriverCheck(DscpDevice* dscp, Byte* data, Uint16 len);
void DC_Motor_Start(DscpDevice* dscp, Byte* data, Uint16 len);//开直流电机
void DC_Motor_Get_State(DscpDevice* dscp, Byte* data, Uint16 len);//获取直流电机的状态
void DC_Motor_Stop(DscpDevice* dscp, Byte* data, Uint16 len);//关直流电机
void DC_StirMotor_SetLevel(DscpDevice* dscp, Byte* data, Uint16 len);//设置搅拌电机功率

//命令入口，全局宏定义：每一条命令对应一个命令码和处理函数
#define CMD_TABLE_MOTORCONTROL \
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_TOTAL_MOTORS, MotorControl_GetTotalPumps), \
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_MOTION_PARAM, MotorControl_GetMotionParam), \
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_SET_MOTION_PARAM, MotorControl_SetMotionParam),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_MOTOR_STATUS, MotorControl_GetStatus),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_MOTOR_MAX_STEPS, MotorControl_GetMaxSteps), \
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_MOTOR_INIT_STEPS, MotorControl_GetInitSteps), \
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_MOTOR_CURRENT_STEPS, MotorControl_GetCurrentSteps),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_MOTOR_MOVE, MotorControl_Start),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_MOTOR_STOP , MotorControl_Stop),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_MOTOR_RESET, MotorControl_Reset),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_GET_SENSOR_STATUS , MotorControl_GetSensorStatus),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_DRIVER_REINIT , MotorControl_DriverReinit),\
    DSCP_CMD_ENTRY(DSCP_CMD_MCI_DRIVER_CHECK , MotorControl_DriverCheck),\
	DSCP_CMD_ENTRY(DSCP_CMD_MCI_DC_MOTOR_SET_STATE , DC_Motor_Start),\
	DSCP_CMD_ENTRY(DSCP_CMD_MCI_DC_MOTOR_GET_STATE , DC_Motor_Get_State),\
	DSCP_CMD_ENTRY(DSCP_CMD_MCI_DC_MOTOR_STOP_STATE , DC_Motor_Stop),\
	DSCP_CMD_ENTRY(DSCP_CMD_MCI_STIR_MOTOR_SET_LEVEL , DC_StirMotor_SetLevel)

#endif /* SRC_DNCPSTACK_MOTORCONTROL_H_ */
