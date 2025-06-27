/**
 * @file
 * @brief 设备信息接口实现
 * @details 
 * @version 1.0.0
 * @author lemon.xiaoxun
 * @date 2013-5-17
 */

#include <string.h>
#include "DeviceInfo.h"
#include "Common/Utils.h"
#include "Manufacture/ManufTypes.h"
#include "Manufacture/VersionInfo.h"
#include "DNCP/App/DscpSysDefine.h"
#include "Tracer/Trace.h"
#include "Driver/McuFlash.h"
#include "Driver/System.h"
#include "SystemConfig.h"

const static ManufInfo s_kmanuf =
{
        .sn = "B21531204",
        .type = "LUDC-S",
        .model = "LULC8201",
        .vender = "LABSUN",
        .mdate.year = 2017,
        .mdate.month = 11,
        .mdate.day = 15,
};

void DeviceInfo_Init(void)
{
    Uint8 buffer[DEVICE_INFO_SIGN_FLASH_LEN] =
    { 0 };

    Uint32 flashFactorySign = 0;

    McuFlash_Read(DEVICE_INFO_SIGN_FLASH_BASE_ADDR,
    DEVICE_INFO_SIGN_FLASH_LEN, buffer);             //读取出厂标志位
    memcpy(&flashFactorySign, buffer, DEVICE_INFO_SIGN_FLASH_LEN);

    if (FLASH_FACTORY_SIGN == flashFactorySign)             //表示已经过出厂设置
    {

    }
    else             //未设置,使用默认值，并写入出厂标志
    {

        flashFactorySign = FLASH_FACTORY_SIGN;

        McuFlash_Write(DEVICE_INFO_TYPE_ADDRESS, strlen((const char*) s_kmanuf.type) + 1, (Uint8*) &s_kmanuf.type);
        McuFlash_Write(DEVICE_INFO_SN_ADDRESS, strlen((const char*) s_kmanuf.sn) + 1, (Uint8*) &s_kmanuf.sn);
        McuFlash_Write(DEVICE_INFO_MODEL_ADDRESS, strlen((const char*) s_kmanuf.model) + 1,(Uint8*) &s_kmanuf.model);
        McuFlash_Write(DEVICE_INFO_DATE_ADDRESS, DEVICE_INFO_DATE_LEN, (Uint8*) &s_kmanuf.mdate);
        McuFlash_Write(DEVICE_INFO_MANUF_ADDRESS, strlen((const char*) s_kmanuf.vender) + 1 , (Uint8*) &s_kmanuf.vender);

        memcpy(buffer, &flashFactorySign, DEVICE_INFO_SIGN_FLASH_LEN);

        McuFlash_Write(
        DEVICE_INFO_SIGN_FLASH_BASE_ADDR,
        DEVICE_INFO_SIGN_FLASH_LEN, buffer);
    }
}

/**
 * @brief 查询设备的类型名称
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetType(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;

    McuFlash_Read(DEVICE_INFO_TYPE_ADDRESS, DEVICE_INFO_TYPE_LEN,
            (Uint8*) &manuf.info.type);
    TRACE_MARK("\n Type: %s", manuf.info.type);

    DscpDevice_SendResp(dscp, (void *) &manuf.info.type,
            strlen((const char*) manuf.info.type) + 1);
}

/**
 * @brief 设置设备的类型名称
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_SetType(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;
    int size = 0;
    unsigned short ret = DSCP_OK;

    size = sizeof(manuf.info.type);
    if ((len > size) || (data[len - 1] != '\0'))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(manuf.info.type, data, len);
        McuFlash_Write(DEVICE_INFO_TYPE_ADDRESS, len,
                (Uint8*) &manuf.info.type);
        TRACE_MARK("\n Type: %s", manuf.info.type);
    }

    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 获取设备序列号
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetSn(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;

    McuFlash_Read(DEVICE_INFO_SN_ADDRESS, DEVICE_INFO_SN_LEN,
            (Uint8*) &manuf.info.sn);
    TRACE_MARK("\n SN: %s", manuf.info.sn);

    DscpDevice_SendResp(dscp, (void *) manuf.info.sn,
            strlen((const char*) manuf.info.sn) + 1);
}

/**
 * @brief 设置设备序列号
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_SetSn(DscpDevice* dscp, Byte* data, Uint16 len)
{
    unsigned short ret = DSCP_OK;
    ManufBlock manuf;
    int size = 0;

    //设置数据正确性判断
    size = sizeof(manuf.info.sn);
    if ((len > size) || (data[len - 1] != '\0'))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(manuf.info.sn, data, len);
        McuFlash_Write(DEVICE_INFO_SN_ADDRESS, len,
                (Uint8*) &manuf.info.sn);
        TRACE_MARK("\nSN: %s", manuf.info.sn);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 获取板卡型号
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetModel(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;

    McuFlash_Read(DEVICE_INFO_MODEL_ADDRESS, DEVICE_INFO_MODEL_LEN,
            (Uint8*) &manuf.info.model);
    TRACE_MARK("\n model: %s", manuf.info.model);

    DscpDevice_SendResp(dscp, (void *) &manuf.info.model,
            strlen((const char*) manuf.info.model) + 1);
}

/**
 * @brief 设置板卡型号
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_SetModel(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;
    int size = 0;
    unsigned short ret = DSCP_OK;

    //设置数据正确性判断
    size = sizeof(manuf.info.model);
    if ((len > size) || (data[len - 1] != '\0'))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(manuf.info.model, data, len);
		McuFlash_Write(DEVICE_INFO_MODEL_ADDRESS, len,(Uint8*) &manuf.info.model);
        TRACE_MARK("\n model: %s", manuf.info.model);
    }

    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 获取设备软件版本号
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetSoftwareVersion(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufVersion version = VersionInfo_GetSoftwareVersion();
    TRACE_MARK("\n Version: %u.%u.%u.%u",
            version.major,
            version.minor,
            version.revision,
            version.build
            );
    DscpDevice_SendResp(dscp, &version, sizeof(version));
}

/**
 * @brief 获取设备软件标识
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetSoftwareLabel(DscpDevice* dscp, Byte* data, Uint16 len)
{
    const char *softwarelabel = VersionInfo_GetSoftwareLabel();
    // 发送回应
    DscpDevice_SendResp(dscp, (void *) softwarelabel,
            strlen(softwarelabel) + 1);
}

/**
 * @brief 获取设备硬件版本号
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetHardwareVersion(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufVersion version = VersionInfo_GetHardwareVersion();
    TRACE_MARK("\n Version: %u.%u.%u.%u",
            version.major,
            version.minor,
            version.revision,
            version.build
            );
    DscpDevice_SendResp(dscp, &version, sizeof(version));
}

/**
 * @brief 查询设备的生产日期
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetDate(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;
    Uint8 buffer[sizeof(manuf.info.mdate)] = { 0 };
    Uint8 offadd = 0;

    McuFlash_Read(DEVICE_INFO_DATE_ADDRESS, DEVICE_INFO_DATE_LEN,
            (Uint8*) &manuf.info.mdate);

    TRACE_MARK("Date:%d.%d.%d", manuf.info.mdate.year, manuf.info.mdate.month, manuf.info.mdate.day);

    len = sizeof(manuf.info.mdate.year);
    memcpy(buffer, &manuf.info.mdate.year, len);
    offadd = len;
    len = sizeof(manuf.info.mdate.month);
    memcpy(buffer + offadd, &manuf.info.mdate.month, len);
    offadd += len;
    len = sizeof(manuf.info.mdate.day);
    memcpy(buffer + offadd, &manuf.info.mdate.day, len);

    DscpDevice_SendResp(dscp, (void *) &manuf.info.mdate,
            sizeof(manuf.info.mdate));
}

/**
 * @brief 设置设备的生产日期
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_SetDate(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;
    short ret = DSCP_OK;
    int size = 0;

    // 导入生产信息
    size = sizeof(manuf.info.mdate);
    if (len > size)
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(&manuf.info.mdate.year, data, sizeof(manuf.info.mdate.year));
        size = sizeof(manuf.info.mdate.year);
        memcpy(&manuf.info.mdate.month, data + size, sizeof(manuf.info.mdate.month));
        size += sizeof(manuf.info.mdate.month);
        memcpy(&manuf.info.mdate.day, data + size, sizeof(manuf.info.mdate.day));

        McuFlash_Write(DEVICE_INFO_DATE_ADDRESS, DEVICE_INFO_DATE_LEN,
                (Uint8*) &manuf.info.mdate);
        TRACE_MARK("Date:%d.%d.%d", manuf.info.mdate.year, manuf.info.mdate.month, manuf.info.mdate.day);
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 查询设备的生产厂商
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_GetManufacter(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;

    McuFlash_Read(DEVICE_INFO_MANUF_ADDRESS, DEVICE_INFO_MANUF_LEN,
            (Uint8*) &manuf.info.vender);

    TRACE_MARK("\n vender: %s", manuf.info.vender);

    DscpDevice_SendResp(dscp, (void *) &manuf.info.vender,
            strlen((const char*) manuf.info.vender) + 1);
}

/**
 * @brief 设置设备的生产厂商。
 * @param dscp
 * @param data
 * @param len
 */
void DeviceInfo_SetManufacter(DscpDevice* dscp, Byte* data, Uint16 len)
{
    ManufBlock manuf;
    Uint16 ret = DSCP_ERROR;
    int size = 0;

    size = sizeof(manuf.info.vender);
    if ((len > size) || (data[len - 1] != '\0'))
    {
        ret = DSCP_ERROR;
        TRACE_ERROR("Parame Len Error\n");
        TRACE_ERROR("%d \n", size);
    }
    else
    {
        memcpy(manuf.info.vender, data, len);

        McuFlash_Write(DEVICE_INFO_MANUF_ADDRESS, len, (Uint8*) &manuf.info.vender);

        TRACE_MARK("\n vender: %s", manuf.info.vender);
    }
    DscpDevice_SendStatus(dscp, ret);
}

