/*
 * PeristalticPump.h
 *
 *  Created on: 2016年6月4日
 *      Author: Administrator
 */

#ifndef SRC_DNCPSTACK_PERISTALTICPUMP_H_
#define SRC_DNCPSTACK_PERISTALTICPUMP_H_

#include "Common/Types.h"
#include "DNCP/App/DscpDevice.h"
#include "LuipApi/PeristalticPumpInterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

//声明处理函数
void PeristalticPump_GetTotalPumps(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_GetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_SetMotionParam(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_GetFactor(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_SetFactor(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_GetStatus(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_GetVolume(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_Start(DscpDevice* dscp, Byte* data, Uint16 len);
void PeristalticPump_Stop(DscpDevice* dscp, Byte* data, Uint16 len);
void WaterPump_SetStatus(DscpDevice* dscp, Byte* data, Uint16 len);//开启采水泵
void WaterPump_GetStatus(DscpDevice* dscp, Byte* data, Uint16 len);//获取采水泵的状态
void PeristalticPump_StartToPoint(DscpDevice* dscp, Byte* data, Uint16 len);//泵至液位传感器

//命令入口，全局宏定义：每一条命令对应一个命令码和处理函数
#define CMD_TABLE_PERISTALTIC_PUMP \
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_GET_TOTAL_PUMPS, PeristalticPump_GetTotalPumps), \
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_GET_MOTION_PARAM, PeristalticPump_GetMotionParam), \
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_SET_MOTION_PARAM, PeristalticPump_SetMotionParam),\
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_GET_PUMP_FACTOR, PeristalticPump_GetFactor),\
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_SET_PUMP_FACTOR, PeristalticPump_SetFactor), \
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_GET_PUMP_VOLUME, PeristalticPump_GetVolume), \
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_START_PUMP, PeristalticPump_Start),\
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_STOP_PUMP, PeristalticPump_Stop),\
    DSCP_CMD_ENTRY(DSCP_CMD_PPI_GET_PUMP_STATUS , PeristalticPump_GetStatus),\
	DSCP_CMD_ENTRY(DSCP_CMD_PPI_WATER_PUMP_SET_STATE , WaterPump_SetStatus),\
	DSCP_CMD_ENTRY(DSCP_CMD_PPI_WATER_PUMP_GET_STATE , WaterPump_GetStatus),\
	DSCP_CMD_ENTRY(DSCP_CMD_PPI_START_PUMP_TO_POINT , PeristalticPump_StartToPoint)
#ifdef __cplusplus
}
#endif

#endif /* SRC_DNCPSTACK_PERISTALTICPUMP_H_ */
