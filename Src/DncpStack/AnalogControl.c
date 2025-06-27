/*
 * AnalogControl.c
 *
 *  Created on: 2020年5月20日
 *      Author: Administrator
 */
#include "AnalogControl.h"
#include "DNCP/App/DscpSysDefine.h"
#include "AnalogControl/AnalogControl.h"
#include "Tracer/Trace.h"
#include <string.h>

#define  BYTE_BUFFER_LENGTH  128
#define  DATA_BUFFER_LENGTH  32

/**
 * @brief 查询模拟输入量接口数目。
 * @return 模拟输入量数目，Uint16。
 */
void AnalogControl_GetAINumber(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 num = 0;

    num = AnalogController_GetAINumber();

    DscpDevice_SendResp(dscp, &num, sizeof(num));
}

/**
 * @brief 查询模拟量数值。
 * @param index Uint16，要查询的模拟输入量索引。
 * @return 当前模拟输入量，Uint32。
 */
void AnalogControl_GetAIData(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 size = 0;
    Uint16 index = 0;
    Uint32 ad = 0;

    size = sizeof(index);
    if ((len != size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", len);
    }
    else
    {
        memcpy(&index, data, len);
        ad = AnalogController_GetAIData(index);
    }

    DscpDevice_SendResp(dscp, &ad, sizeof(ad));
}

/**
 * @brief 获取所有模拟量数据。
 * @return 所有模拟输入量数据
 * - num Uint16，当前模拟量个数
 * - data Uint32*，模拟量AD值数组
 */
void AnalogControl_GetAllAIData(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    static Byte sendBuffer[BYTE_BUFFER_LENGTH] = {0};
    Uint16 sendLen = 0;
    static Uint32 dataBuffer[DATA_BUFFER_LENGTH] = {0};
    Uint16 num = 0;

    memset(sendBuffer, 0, sizeof(sendBuffer));
    memset(dataBuffer, 0, sizeof(dataBuffer));

    AnalogController_GetAllAIData(&num, dataBuffer);

    sendLen = sizeof(Uint16) + sizeof(Uint32)*num;

    memcpy(sendBuffer, &num, sizeof(Uint16));
    memcpy(sendBuffer+sizeof(Uint16), dataBuffer, sizeof(Uint32)*num);

    DscpDevice_SendResp(dscp, sendBuffer, sendLen);
}

/**
 * @brief 设置模拟输入量上报间隔
 * @param time Uint16， 上报间隔，单位：秒。（0表示不需要主动上传）
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
void AnalogControl_SetAIUploadPeriod(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 size = 0;
    Uint16 period = 0;

    size = sizeof(period);
    if ((len != size))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error %d\n", len);
    }
    else
    {
        memcpy(&period, data, len);
        AnalogController_SetAIUploadPeriod(period);
    }

    DscpDevice_SendStatus(dscp, ret);
}
