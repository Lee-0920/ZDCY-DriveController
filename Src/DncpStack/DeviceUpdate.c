/*
 * DeviceUpdate.c
 *
 *  Created on: 2016年7月19日
 *      Author: LIANG
 */
#include <string.h>
#include "DeviceUpdate/UpdateHandle.h"
#include "System.h"
#include "DNCP/App/DscpSysDefine.h"
#include "Manufacture/VersionInfo.h"
#include "Manufacture/ManufTypes.h"
#include "Tracer/Trace.h"
#include "DeviceUpdate.h"

/**
 * @brief 查询 Updater 的版本号。
 * @details 不同的 Updater 版本，使用的升级参数和调用时序可能不一致，请参考：
 *   @ref sec_DUI_ChangeLog 。
 * @return 版本号，Byte[4]：
 *  - @b major Uint8，主版本号；
 *  - @b minor Uint8，次版本号；
 *  - @b revision Uint8，修订版本号；
 *  - @b build Uint8，编译版本号；
 */
void DeviceUpdate_GetVersion(DscpDevice* dscp, Byte* data, Uint16 len)
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
 * @brief 查询 Updater 支持的最大分片大小。
 * @details 在调用 @ref DSCP_CMD_DUI_WRITE_PROGRAM 命令进行数据烧写时，
 *  最大的分片大小不能超过设备的限制长度。
 * @return 最大分片大小，Uint16，单位为字节。
 */
void DeviceUpdate_GetMaxFragmentSize(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 size = UpdateHandle_GetMaxFragmentSize();
    TRACE_MARK("\n size %d", size);
    DscpDevice_SendResp(dscp, &size, sizeof(Uint16));
}

#ifdef _CS_APP
/**
 * @brief 进入Updater模式。
 * @details 在 App 模式下调用本命令，程序将跳转到升级模式，等待升级相关的指令。
 * @return 无回应。
 * @note 仅在 App 模式下支持本命令。
 */
void DeviceUpdate_EnterUpdater(DscpDevice* dscp, Byte* data, Uint16 len)
{
    UpdateHandle_EnterUpdater();
}
#else
/**
 * @brief 进入应用程序。
 * @details 在 Updater 模式下调用本命令，程序将跳转到应用程序。
 *  通常在升级未开始前不打算继续升级，或者在升级完成后调用。
 * @return 无回应。
 */
void DeviceUpdate_EnterApplication(DscpDevice* dscp, Byte* data, Uint16 len)
{
    UpdateHandle_EnterApplication();
}
#endif

/**
 * @brief 擦除程序存储区域。
 * @details 在调用 Write 写数据之前，必须先调用本操作。对于没有擦除的区域，
 *  直接写入程序可能会造成数据混乱。由于擦除需要耗费较长时间，不能在此回应擦除是否真正的成功。
 *  如果擦除失败，程序完整性校验会失败。
 *  @ref DSCP_CMD_DUI_WRITE_PROGRAM 操作。
 *  <p>设备知道自己的可擦写块大小，具体的擦写算法由设备自行决定，而不需要上位机额外干预。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 */
void DeviceUpdate_Erase(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    UpdateHandle_SendEventOpen();
    if (FALSE == UpdateHandle_StartErase())
    {
        ret = DSCP_BUSY;
    }
    DscpDevice_SendStatus(dscp, ret);
}

/**
 * @brief 向设备写程序数据。
 * @details 升级管理程序需要对程序文件进行分片，然后针对每一分片，按序调用本操作更新程序。
 *  设备支持的最大分片大小限制参见： @ref DSCP_CMD_DUI_GET_MAX_FRAGMENT_SIZE 命令。
 * @param length Uint16，分片数据的长度，单位为字节
 * @param seq Uint16，分片序号
 * @param data[length] Byte[]，分片数据，具体长度取决于length参数。
 * @return 执行状态和期望序号：
 *  - @b status Uint16，执行状态，支持的状态有：
 *      - @ref DSCP_OK 操作成功；
 *      - @ref DUI_WRITE_CHECK_ERROR 写入校验错误，写完后立即检查，发现与写入的数据不同；
 *      - @ref DUI_WRITE_FAILED 写入失败；
 *      - @ref DUI_WRITE_SIZE_ERROR 错误的数据长度；
 *  - @b seq Uint16，期望序号；
 * @note 请确保已先正确调用 @ref DSCP_CMD_DUI_ERASE 命令。
 */
void DeviceUpdate_WriteProgram(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 length;
    Uint16 seq;
    WriteProgramResult result;

    memcpy(&length, data + sizeof(Uint16), sizeof(Uint16));
    memcpy(&seq, data , sizeof(Uint16));

    if  (len == (length + sizeof(Uint16) * 2))
    {
        data = data + sizeof(Uint16) * 2;
        result = UpdateHandle_WriteProgram(data, length, seq);
        TRACE_INFO("\n seq:%d status %d", seq, result.status);
    }
    else
    {
        result.status = DUI_WRITE_SIZE_ERROR;
        result.seq = seq;
        TRACE_ERROR("\n size error.");
    }
    DscpDevice_SendResp(dscp, &result, sizeof(result));
}

void DeviceUpdate_CheckIntegrity(DscpDevice* dscp, Byte* data, Uint16 len)
{
    Uint16 ret = DSCP_OK;
    Uint16 checksum;

    memcpy(&checksum, data, sizeof(Uint16));

    if(FALSE == UpdateHandle_CheckIntegrity(checksum))
    {
        ret = DSCP_ERROR;
    }
    DscpDevice_SendStatus(dscp, ret);
}
