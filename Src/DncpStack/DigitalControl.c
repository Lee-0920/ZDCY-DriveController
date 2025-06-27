/**
 * @file
 * @brief 数字控制接口实现
 */
#include "DigitalControl.h"
#include <DigitalControlDriver/DigitalInputDriver.h>
#include <DigitalControlDriver/DigitalOutputDriver.h>
#include <DigitalControl/DigitalControl.h>
#include <string.h>
#include "DNCP/App/DscpSysDefine.h"
#include "Tracer/Trace.h"
#include "Common/Types.h"
#include "SystemConfig.h"
#include "AnalogControl.h"
#include "AnalogControl/AnalogControl.h"
#include "DigitalControl.h"
#include "LiquidDriver/DoorLock.h"

/**
 * @brief 查询系统支持的输入开关量数目。
 * @return 输入开关量数目， Uint16。
 */
void DigitalControl_GetDINumber(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 num = 0;

    num = DigitalController_GetDINumber();

    DscpDevice_SendResp(dscp, &num, sizeof(num));
}

/**
 * @brief 查询输入开关量的状态。
 * @param index Uint16，要查询的开关量索引。
 * @return 开关量的状态，Uint8，FALSE 0 = 断开，TRUE 1 = 闭合。
 */
void DigitalControl_GetDIStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 size = 0;
    Uint16 index = 0;
    Bool status = FALSE;

    size = sizeof(index);
    if ((len != size))
    {
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", len);
    }
    else
    {
        memcpy(&index, data, len);
        status = DigitalController_GetDIStatus(index);
    }

    DscpDevice_SendResp(dscp, &status, sizeof(status));
}

/**
 * @brief 查询所有输入开关量的状态映射图。
 * @return 状态映射图，Uint32，按位表示开关量状态：位0为关，1为开。
 */
void DigitalControl_GetDIStatusMap(DscpDevice* dscp, Byte* data, Uint16 len)
{

    Uint32 map = 0;

    map = DigitalController_GetDIStatusMap();

    DscpDevice_SendResp(dscp, &map, sizeof(map));
}

/**
 * @brief 查询系统支持的输出开关量数目。
 * @return 开关量数目， Uint16。
 */
void DigitalControl_GetDONumber(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 num = 0;

    num = DigitalController_GetDONumber();

    DscpDevice_SendResp(dscp, &num, sizeof(num));
}

/**
 * @brief 查询输出开关量的工作状态。
 * @param index Uint16，要查询的输出开光量索引。
 * @return 开关量的状态，Uint8，FALSE 0 = 断开，TRUE 1 = 闭合。
 */
void DigitalControl_GetDOStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 size = 0;
    Uint16 index = 0;
    Bool status = FALSE;

    size = sizeof(index);
    if ((len != size))
    {
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", len);
    }
    else
    {
        memcpy(&index, data, len);
        status = DigitalController_GetDOStatus(index);
    }

    DscpDevice_SendResp(dscp, &status, sizeof(status));
}

/**
 * @brief 设置输出开关量状态
 * @details 断开或闭合继电器
 * @param index Uint16， 输出开关量索引。
 * @param state Uint8， FALSE 0 = 断开，TRUE 1 = 闭合。
 */
void DigitalControl_SetDOStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 ret = DSCP_OK;
    Uint16 size = 0;
    Uint16 index = 0;
    Bool status = FALSE;

    size = sizeof(index) + sizeof(status);
    if ((len != size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", len);
    }
    else
    {
        memcpy(&index, data, sizeof(index));
        memcpy(&status, data+sizeof(index), sizeof(status));

        DigitalController_SetDOStatus(index, status);
    }

    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询所有输入开关量的状态映射图。
 * @return 状态映射图，Uint32，按位表示开关量状态：位0为关，1为开。
 */
void DigitalControl_GetDOStatusMap(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint32 map = 0;

    map = DigitalController_GetDOStatusMap();

    DscpDevice_SendResp(dscp, &map, sizeof(map));
}

/**
 * @brief 设置所有输入开关量的状态映射。
 * @param map Uint32，状态映射图，位0为关，1为开。
 */
void DigitalControl_SetDOStatusMap(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 size = 0;
    Uint32 map = 0;

    size = sizeof(map);
    if ((len != size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", len);
    }
    else
    {
        memcpy(&map, data, sizeof(map));
        DigitalController_SetDOStatusMap(map);
    }

    DscpDevice_SendStatus(dscp, ret);
}

//查询门禁状态
void DigitalControl_GetGateStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 status = TRUE;

	status = DoorDetection_ReadStatus();

	DscpDevice_SendResp(dscp, &status, sizeof(status));

}

//开关电子锁
void DigitalControl_SetLockState(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 ret = DSCP_OK;
	Uint16 Data = 0;

	memcpy(&Data, data, sizeof(data));

	ret = ElectronicLock_Control(Data);

	DscpDevice_SendStatus(dscp, ret);
}

//查询电子锁状态
void DigitalControl_GetLockStatus(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 status = TRUE;

	status = ElectronicLock_ReadStatus();

	DscpDevice_SendResp(dscp, &status, sizeof(status));
}

void DigitalControl_SetFlowMeter(DscpDevice* dscp, Byte* data, Uint16 len)
{
	Uint16 ret = DSCP_OK;
	Uint16 Data = 0;

	memcpy(&Data, data, sizeof(Uint8));

	Printf("\n data = %d ",Data);

	if(1 == Data)
	{
		AnalogController_StartFlow();
	}
	else
	{
		AnalogController_StopFlow();
	}

	DscpDevice_SendStatus(dscp, ret);
}

