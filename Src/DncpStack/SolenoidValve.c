/**
 * @file SolenoidValveInterface.h
 * @brief 电磁阀控制执行文件。
 * @version 1.0.0
 * @author xingfan
 * @date 2016-05-27
 */
#include "Tracer/Trace.h"
#include "Dncp/App/DscpSysDefine.h"
#include <string.h>
#include "SolenoidValve/ValveManager.h"
#include "SolenoidValve.h"

/**
 * @brief 查询系统支持的总电磁阀数目。
 * @param dscp
 * @param data
 * @param len
 */
void SolenoidValve_GetTotalValves(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 TotalValves = ValveManager_GetTotalValves();
    DscpDevice_SendResp(dscp, &TotalValves, sizeof(TotalValves));
}

/**
 * @brief 查询当前开启的阀门映射图。
 * @param dscp
 * @param data
 * @param len
 */
void SolenoidValve_GetValvesMap(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint32 map = ValveManager_GetValvesMap();
    TRACE_MARK("\nDNCP GetMap: 0x%x",map);
    DscpDevice_SendResp(dscp, &map, sizeof(Uint32));
}

/**
 * @brief 设置要开启的阀门映射图。
 * @param dscp
 * @param data
 * @param len
 */
void SolenoidValve_SetValvesMap(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint32 map =  0;
    Uint16 ret = DSCP_OK;

    memcpy(&map, data, sizeof(Uint32));
    TRACE_MARK("\nDNCP SetMap: 0x%x", map);
    if (FALSE == ValveManager_SetValvesMap(map))
    {
        ret = DSCP_ERROR;
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 设置要开启的阀门映射图。
 * @param dscp
 * @param data
 * @param len
 */
void SolenoidValve_TurnOnValvesMap(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint32 map      = 0;
    Uint32 valveMap = 0;
    Uint16 ret = DSCP_OK;

    memcpy(&map, data, sizeof(Uint32));

    valveMap = ValveManager_GetValvesMap();

    map = valveMap | map;

    if (FALSE == ValveManager_SetValvesMap(map))
    {
        ret = DSCP_ERROR;
    }

    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 设置要关闭的阀门映射图。
 * @param dscp
 * @param data
 * @param len
 */
void SolenoidValve_TurnOffValvesMap(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint32 map      = 0;
    Uint32 valveMap = 0;
    Uint16 ret = DSCP_OK;

    memcpy(&map, data, sizeof(Uint32));

    valveMap = ValveManager_GetValvesMap();

    map = valveMap & (~map);

    if (FALSE == ValveManager_SetValvesMap(map))
    {
        ret = DSCP_ERROR;
    }

    DscpDevice_SendStatus(dscp, ret);
}

