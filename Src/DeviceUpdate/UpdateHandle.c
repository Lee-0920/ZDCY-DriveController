/*
 * UpdateHandle.c
 *
 *  Created on: 2016年7月19日
 *      Author: LIANG
 */

#include <DeviceUpdate/UpdateHandle.h>
#include "DNCP/App/DscpSysDefine.h"
#include "string.h"
#include "LuipApi/DeviceUpdateInterface.h"
#include "Tracer/trace.h"
#include "Common/MessageDigest.h"
#include "UpdateDriver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "System.h"
#include "DncpStack/DncpStack.h"
#include "SystemConfig.h"

#define MAX_FRAGMENT_SIZE             192 // 最大允许一次传输的文件内容片的大小，取64的倍数
static Uint32 s_offsetAddr = 0;
static Uint32 s_fileHead;  // 文件的头几个字节，升级完成后，再写入到flash中，用于bootloader分辨是否有完整的应用程序
static Uint16 s_seq = 0;
Uint8 buffer[MAX_FRAGMENT_SIZE] =
{ 0 };

#ifdef _CS_APP
static DeviceRunMode s_runMode = DEVICE_MODE_APPLICATION;
#else
static DeviceRunMode s_runMode = DEVICE_MODE_UPDATER;
#endif

static void UpdateHandle_EraseTask(void *argument);
static xTaskHandle s_eraseHandle;
static Bool s_isSendEvent = FALSE;
typedef enum
{
    ERASE_IDLE, ERASE_BUSY
} EraseStatus;
static EraseStatus s_eraseStatus = ERASE_IDLE;

void UpdateHandle_Init(void)
{
    xTaskCreate(UpdateHandle_EraseTask, "EraseTask",
            UPDATER_ERASE_STK_SIZE, NULL,
            UPDATER_ERASE_TASK_PRIO, &s_eraseHandle);
}

void UpdateHandle_SendEventOpen(void)
{
    s_isSendEvent = TRUE;
}

void UpdateHandle_EraseTask(void *argument)
{
    EraseResult result;
    vTaskSuspend(NULL);
    while (1)
    {
        switch (s_eraseStatus)
        {
        case ERASE_IDLE:
            vTaskSuspend(NULL);
            break;
        case ERASE_BUSY:
            System_Delay(2000);
#ifdef _CS_APP
            s_runMode = DEVICE_MODE_UPGRADER;
#endif
            if(TRUE == UpdateDriver_Erase())
            {
                result = ERASE_RESULT_FINISHED;
            }
            else
            {
                result = ERASE_RESULT_CHECK_ERROR;
            }
            s_offsetAddr = 0;
            s_seq = 0;
            s_eraseStatus = ERASE_IDLE;
            if (TRUE == s_isSendEvent)
            {
                DncpStack_SendEvent(DSCP_EVENT_DUI_ERASE_RESULT, &result,
                        sizeof(EraseResult));
                DncpStack_BufferEvent(DSCP_EVENT_DUI_ERASE_RESULT, &result,
                        sizeof(EraseResult));
            }
            s_isSendEvent = FALSE;
            TRACE_INFO("\n Erase result %d", result);
            break;
        }
    }
}

Bool UpdateHandle_StartErase(void)
{
    Bool ret = TRUE;
    if (s_eraseStatus == ERASE_IDLE)             // 采集处于空闲
    {
        DncpStack_ClearBufferedEvent();
        s_eraseStatus = ERASE_BUSY;
        TRACE_INFO("\n StartAcquirer Erase");
        vTaskResume(s_eraseHandle);
    }
    else
    {
        TRACE_ERROR("\n Erase is busy");
        ret = FALSE;
    }
    return ret;
}

/**
 * @note 注意：此函数是不会检查data的长度是否和length记录的长度相等
 * @param data
 * @param length
 * @return
 */
WriteProgramResult UpdateHandle_WriteProgram(Byte *data, Uint16 length, Uint16 seq)
{
    WriteProgramResult result;
    if(seq == s_seq)
    {
        if (length <= MAX_FRAGMENT_SIZE)
        {
            // 升级程序的首4字节暂时不写入FLASH，等升级完成后再写入，bootloader用于识别是否有完整的APP
            if (0 == s_offsetAddr)
            {
                if(length >= 4)
                {
                    memcpy(&s_fileHead, data, sizeof(s_fileHead));

                    data += 4;
                    length -= 4;
                    s_offsetAddr += 4;
                    TRACE_DEBUG("\n 0x%x", s_fileHead);
                }
                else
                {
                    TRACE_ERROR("\n length < 4 Wrong data length.");
                    result.status = DUI_WRITE_SIZE_ERROR;
                    return result;
                }
            }

            if (TRUE == UpdateDriver_Write(s_offsetAddr, length, data))
            {
                UpdateDriver_Read(s_offsetAddr, length, buffer);
                if(0 == memcmp(buffer, data, length))
                {
                    s_offsetAddr += length;
                    TRACE_DEBUG("\n new s_offsetAddr: 0x%x, length = %d, seq = %d", s_offsetAddr,
                            length, s_seq);
                    s_seq++;
                    result.status = DSCP_OK;
                }
                else
                {
                    TRACE_ERROR("\n Write check error, the result is not consistent with expectations .");
                    result.status = DUI_WRITE_CHECK_ERROR;
                }
            }
            else
            {
                result.status = DUI_WRITE_FAILED;
                TRACE_ERROR("\n write failed.");
            }
        }
        else
        {
            TRACE_ERROR("\n Wrong data length.");
            result.status = DUI_WRITE_SIZE_ERROR;
        }
    }
    else
    {
        TRACE_ERROR("\n seq error.");
        result.status = DUI_WRITE_SEQ_ERROR;
    }
    result.seq = s_seq;

    return result;
}


Bool UpdateHandle_CheckIntegrity(Uint16 checksum)
{
    Bool ret = TRUE;
    Uint16 crc16 = 0;

    if (0 != checksum && 0 != s_offsetAddr)
    {
        Uint32 offset = 4;

        crc16 = MessageDigest_Crc16Ccitt(crc16, (Uint8 *) &s_fileHead, 4);
        //当前写入程序比一片数据大
        if (s_offsetAddr > MAX_FRAGMENT_SIZE)
        {
            //如果当前的偏移地址读取的程序还够一片数据，则按片大小读取
            for (; offset < (s_offsetAddr - MAX_FRAGMENT_SIZE); offset +=
                    MAX_FRAGMENT_SIZE)
            {
                UpdateDriver_Read(offset, MAX_FRAGMENT_SIZE, buffer);
                crc16 = MessageDigest_Crc16Ccitt(crc16, buffer,
                        MAX_FRAGMENT_SIZE);
            }
        }

        UpdateDriver_Read(offset, s_offsetAddr - offset, buffer);
        crc16 = MessageDigest_Crc16Ccitt(crc16, buffer, s_offsetAddr - offset);
    }

    if (0 == checksum || crc16 == checksum)
    {
        UpdateDriver_Write(0, 4, (Uint8 *) &s_fileHead);
    }
    else
    {
        TRACE_ERROR("\n checksum = %d crc16 = %d.", checksum, crc16);
        TRACE_ERROR("\n Program integrity check failed.");
        ret = FALSE;
    }
    s_offsetAddr = 0;
    s_seq = 0;
    return ret;
}
#ifdef _CS_APP

void UpdateHandle_EnterUpdater(void)
{
    UpdateDriver_JumpToUpdater();
}

#else

void UpdateHandle_EnterApplication(void)
{
    UpdateDriver_JumpToApplication();
}

#endif

DeviceRunMode UpdateHandle_GetRunMode(void)
{
    return s_runMode;
}

Uint16 UpdateHandle_GetMaxFragmentSize(void)
{
    s_offsetAddr = 0;
    return MAX_FRAGMENT_SIZE;
}

