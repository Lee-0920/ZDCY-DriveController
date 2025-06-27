/**
 * @file
 * @brief MCU Flash读写驱动。
 * @details 提供flash读写功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2012-10-30
 */

#include "McuFlash.h"
#include "Tracer/trace.h"
#include "SystemConfig.h"

#define STM32_FLASH_BASE        0x08000000


static u8 tempBuffer[FLASH_USE_SIZE] = 
{ 0 };

const static u32 s_flashSectorAddr[25] = {0x08000000, 0x08004000, 0x08008000, 0x0800C000, 0x08010000,
        0x08020000, 0x08040000, 0x08060000, 0x08080000, 0x080A0000, 0x080C0000, 0x080E0000,
        0x08100000, 0x08104000, 0x08108000, 0x0810C000, 0x08110000, 0x08120000, 0x08140000,
        0x08160000, 0x08180000, 0x081A0000, 0x081C0000, 0x081E0000, 0x081FFFFF};

const static u16 s_flashSector[24] = {FLASH_Sector_0, FLASH_Sector_1, FLASH_Sector_2, FLASH_Sector_3, FLASH_Sector_4,
        FLASH_Sector_5, FLASH_Sector_6, FLASH_Sector_7, FLASH_Sector_8, FLASH_Sector_9,
        FLASH_Sector_10, FLASH_Sector_11, FLASH_Sector_12, FLASH_Sector_13, FLASH_Sector_14,
        FLASH_Sector_15, FLASH_Sector_16, FLASH_Sector_17, FLASH_Sector_18, FLASH_Sector_19,
        FLASH_Sector_20, FLASH_Sector_21, FLASH_Sector_22, FLASH_Sector_23};

/**
 * @brief 获取某个地址所在的flash扇区
 * @param addr flash地址
 * @return 扇区(0~22)
 */
static uint16_t McuFlash_GetFlashSector(u32 addr)
{
    u8 i;
    for (i = 0; i < 24; i++)
    {
        if (addr < s_flashSectorAddr[i + 1])
        {
            break;
        }
    }
    if(i >= 24)  i = 23;
    return i;

}

/**
 * @brief 删除index号扇区
 * @param index 扇区序号
 */
static void McuFlash_EraseSectorOfNum(u8 index)
{
    FLASH_Unlock(); //解锁
    FLASH_DataCacheCmd(DISABLE); //FLASH擦除期间必须禁止数据缓存

    FLASH_EraseSector(index, VoltageRange_2);

    FLASH_DataCacheCmd(ENABLE);
    FLASH_Lock();
}

/**
 * @brief 删除addr所在扇区
 * @param addr
 */
void McuFlash_EraseSector(u32 addr)
{
    u16 index = McuFlash_GetFlashSector(addr);
    McuFlash_EraseSectorOfNum(s_flashSector[index]);
}

/**
 * @brief 读取半字(16位)数据
 * @param faddr 读取数据的地址
 * @return 读取的数据
 */
u16 McuFlash_ReadHalfWord(u32 faddr)
{
    return *(vu16*) faddr;
}

/**
 * @brief 从指定地址开始读出指定长度的数据
 * @param readAddr 起始地址(为偶数)
 * @param numToRead 字节数(为偶数)
 * @param buffer 数据指针
 */
void McuFlash_Read(u32 readAddr, u32 numToRead, u8 *buffer)
{
    u32 i;
    TRACE_MARK("\n readAddr: 0x%x ", readAddr);
    for (i = 0; i < numToRead; i += 2)
    {
        *(u16 *) (buffer + i) = McuFlash_ReadHalfWord(readAddr);
        readAddr += 2;
    }
}

/**
 * @brief 数据写入扇区,写入会擦除该扇区内容，不保留，暂时不支持跨扇区写入
 * @param WriteAddr
 * @param NumToWrite
 * @param buffer
 */

Bool McuFlash_DeleteWrite(u32 writeAddr, u32 numToWrite, u8 *buffer)
{
    FLASH_Status status = FLASH_COMPLETE;
    u32 addrx = 0;
    u32 endaddr = 0;

    if (writeAddr < STM32_FLASH_BASE || writeAddr % 2) //非法地址
    {
        TRACE_ERROR("\n writeAddr: 0x%x ", writeAddr);
        TRACE_ERROR("\n Illegal address, write failed.");
        return FALSE;
    }
    TRACE_MARK("\n writeAddr: 0x%x ", writeAddr);
    FLASH_Unlock(); //解锁
    FLASH_DataCacheCmd(DISABLE); //FLASH擦除期间必须禁止数据缓存

    addrx = writeAddr;
    endaddr = writeAddr + numToWrite;

    if (addrx < 0X1FFF0000) //只有主存储区才能被擦除
    {
        while (addrx < endaddr) //寻找是否有数据，有则擦除
        {
            //判断当前地址是否没被擦除，没擦除则擦除该扇区，否则无法写入数据
            if (McuFlash_ReadHalfWord(addrx) != 0XFFFF)
            {
                u16 index = McuFlash_GetFlashSector(addrx);
                status = FLASH_EraseSector(s_flashSector[index], VoltageRange_2);
                if (status != FLASH_COMPLETE)
                {
                    TRACE_DEBUG("1");
                    break;
                }
            }
            else
            {
                addrx += 2;
            }
        }
    }

    if (status == FLASH_COMPLETE)
    {
        while (writeAddr < endaddr) //写数据
        {
            if (FLASH_ProgramHalfWord(writeAddr, *(u16*) buffer)
                    != FLASH_COMPLETE)
            {
                TRACE_DEBUG("2");
                break;
            }
            writeAddr += 2;
            buffer += 2;
        }
    }
    FLASH_DataCacheCmd(ENABLE);
    FLASH_Lock();
    return TRUE;
}

/**
 * @brief 写入数据，保留WriteAddr所在扇区的0 - FLASH_USE_SIZE字节的内容
 * @param WriteAddr
 * @param NumToWrite
 * @param buffer
 */
void McuFlash_Write(u32 WriteAddr, u32 NumToWrite, u8 *buffer)
{
    u32 sectorAddr = s_flashSectorAddr[McuFlash_GetFlashSector(WriteAddr)];
    u32 secoff = 0;
    u32 i = 0;

    secoff = WriteAddr - sectorAddr; //计算在扇区内的偏移地址

    McuFlash_Read(sectorAddr, FLASH_USE_SIZE, tempBuffer); //读出该地址所在的扇区所使用的区域的数据，避免被擦除掉

    if (NumToWrite + secoff > FLASH_USE_SIZE) //写入的数据超过设置的使用区域范围
    {
        TRACE_ERROR("\n secoff %d,NumToWrite %d, FLASH_USE_SIZE %d ", secoff, NumToWrite, FLASH_USE_SIZE);
        TRACE_ERROR("\n Write area is greater than the area FLASH_USE_SIZE defined ");
        return;
    }

    for (i = 0; i < NumToWrite; i++)
    { //校验数据
        if (tempBuffer[secoff + i] != 0XFF)
        {
            break; //需要擦除
        }
    }
    if(i < NumToWrite) //需要擦除
    {
        //把要写入的内容覆盖到地址对应的区域缓存
        for (i = 0; i < NumToWrite; i++)
        {
            tempBuffer[i + secoff] = buffer[i];
        }
        //把更新后缓存重新写入
        McuFlash_DeleteWrite(sectorAddr, FLASH_USE_SIZE, tempBuffer);
    }
    else
    {
        McuFlash_DeleteWrite(WriteAddr, NumToWrite, buffer);
    }
}

Uint32  McuFlash_GetStatus()
{
	return  (Uint32)FLASH_GetStatus();
}

bool  McuFlash_GetFlagStatus(Uint32 Reg)
{
	return  (bool)FLASH_GetFlagStatus(Reg);
}

