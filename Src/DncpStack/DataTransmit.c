/*
 * DataTransmit.c
 *
 *  Created on: 2020年5月21日
 *      Author: Administrator
 */
#include <DataTransmit/DataTransmitManager.h>
#include <string.h>
#include "DNCP/App/DscpSysDefine.h"
#include "Tracer/Trace.h"
#include "Common/Types.h"
#include "SystemConfig.h"
#include "DataTransmit.h"

#define MAX_DATATRANSMIT_LEN   240

/**
 * @brief 查询系统支持的串口总数。
 * @return  num Uint16，串口数。
 */
void DataTransmit_GetPortNumber(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 num = 0;

    num = DataTransmitManager_TotalPortNumber();

    DscpDevice_SendResp(dscp, &num, sizeof(num));
}

/**
 * @brief 发送数据到指定的串口。
 * @param index Uint16，串口编号。
 * @param data Uint8*，需要发送的数据。
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
void DataTransmit_SendData(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 index = 0;
    Uint8 sendData[MAX_DATATRANSMIT_LEN] = {0};
    Uint16 sendLen = len - sizeof(index);

    if ((sendLen > MAX_DATATRANSMIT_LEN))
    {
        ret = DSCP_ERROR_PARAM;
        TRACE_ERROR("Param Len Error %d\n", len);
    }
    else
    {
        memcpy(&index, data, sizeof(index));
        memcpy(sendData, data+sizeof(index), sendLen);
//        TRACE_INFO("\nDSCP Send Data : ");
//        DataTransmitter_PrintData(sendData, len);
        if(FALSE == DataTransmitManager_SendData(index, sendData, sendLen))
        {
            ret = DSCP_ERROR;
            TRACE_ERROR("Port %d Data Transmit Error %d\n", index);
        }
    }

    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询指定串口的配置参数。
 * @param index Uint16，串口索引。
 * @return 串口配置参数
 * @see SerialPortParam
 *  - baud Uint32，波特率
 *  - wordLen Uint8，数据位
 *  - stopBits Uint8，停止位
 *  - parity Uint8，校验位
 */
void DataTransmit_GetPortParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 index = 0;
    SerialPortParam param;

    if(len == sizeof(index))
    {
        memcpy(&index, data, sizeof(index));
        DataTransmitManager_GetPortParam(index, &param);
    }
    else
    {
        ret = DSCP_ERROR_PARAM;
        TRACE_ERROR("Param Len Error %d\n", len);
    }

    DscpDevice_SendResp(dscp, &param, sizeof(param));
}

/**
 * @brief 设置指定串口的配置参数，参数永久保存到flash。
 * @param index Uint16，串口索引。
 * @param param SerialPortParam，串口配置参数
 * @see SerialPortParam
 *  - baud Uint32，波特率
 *  - wordLen Uint8，数据位
 *  - stopBits Uint8，停止位
 *  - parity Uint8，校验位
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
void DataTransmit_SetPortParam(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 index = 0;
    SerialPortParam param;

    if(len == sizeof(index) + sizeof(Uint32)+ 3*sizeof(Uint8))
    {
        memcpy(&index, data, sizeof(index));
        memcpy(&param.baud, data+2, sizeof(Uint32));
        memcpy(&param.wordLen, data+6, sizeof(Uint8));
        memcpy(&param.stopBits, data+7, sizeof(Uint8));
        memcpy(&param.parity, data+8, sizeof(Uint8));
        if(FALSE == DataTransmitManager_SetPortParam(index, param))
        {
            ret = DSCP_ERROR;
            TRACE_ERROR("Set Port %d Param Error %d\n", index);
        }
    }
    else
    {
        ret = DSCP_ERROR_PARAM;
        TRACE_ERROR("Param Len Error %d\n", len);
    }

    DscpDevice_SendStatus(dscp, ret);
}


