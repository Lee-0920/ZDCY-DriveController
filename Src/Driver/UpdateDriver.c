/*
 * UpdateDriver.c
 *
 *  Created on: 2016年7月19日
 *      Author: LIANG
 */

#include "Tracer/trace.h"
#include "SystemConfig.h"
#include "string.h"
#include <UpdateDriver.h>

#ifdef _CS_APP
#define CODE_START_ADDR              UPDATE_FLASH_START
#define CODE_END_ADDR                UPDATE_FLASH_END
#else
#define CODE_START_ADDR              APP_FLASH_START
#define CODE_END_ADDR                APP_FLASH_END
#endif

#define RESET_VECTOR_OFFSET          4   // 复位向量偏移地址，为起始位置偏移4字节

#define BASE_ADDR_BSL_FLAG           0x2002FFF0
#define BL_ENTER_APP_FLAG            "enterAPP"
#define BL_ENTER_APP_FLAG_LEN        9
#define BL_ENTER_UPDATE_FLAG         "enterUp"
#define BL_ENTER_UPDATE_FLAG_LEN     8

#define CODE_EXIST_FLAG_LEN          4

const static u32 s_flashSectorAddr[25] =
{ 0x08000000, 0x08004000, 0x08008000, 0x0800C000, 0x08010000, 0x08020000,
        0x08040000, 0x08060000, 0x08080000, 0x080A0000, 0x080C0000, 0x080E0000,
        0x08100000, 0x08104000, 0x08108000, 0x0810C000, 0x08110000, 0x08120000,
        0x08140000, 0x08160000, 0x08180000, 0x081A0000, 0x081C0000, 0x081E0000,
        0x081FFFFF };
const static u16 s_flashSector[24] =
{ FLASH_Sector_0, FLASH_Sector_1, FLASH_Sector_2, FLASH_Sector_3,
FLASH_Sector_4,
FLASH_Sector_5, FLASH_Sector_6, FLASH_Sector_7, FLASH_Sector_8,
FLASH_Sector_9,
FLASH_Sector_10, FLASH_Sector_11, FLASH_Sector_12, FLASH_Sector_13,
FLASH_Sector_14,
FLASH_Sector_15, FLASH_Sector_16, FLASH_Sector_17, FLASH_Sector_18,
FLASH_Sector_19,
FLASH_Sector_20, FLASH_Sector_21, FLASH_Sector_22, FLASH_Sector_23 };

typedef void (*JumpFun)(void); //定义一个函数类型的参数.

/**
 * @brief 获取地址所在的扇区号 0-23
 * @param addr
 * @return
 */
static uint16_t UpdateDriver_GetFlashSector(u32 addr)
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

static u16 UpdateDriver_ReadHalfWord(u32 faddr)
{
    return *(vu16*) faddr;
}

void UpdateDriver_Read(u32 OffsetAddr, u32 numToRead, u8 *buffer)
{

    u32 i, readAddr = CODE_START_ADDR + OffsetAddr;

    TRACE_MARK("\n readAddr: 0x%x ", readAddr);
    for (i = 0; i < numToRead; i += 2)
    {
        *(u16 *) (buffer + i) = UpdateDriver_ReadHalfWord(readAddr);
        readAddr += 2;
    }
}

/**
 * @note 注意此写函数在写的过程中遇到没擦除的FLAHSH的话，会把地址所在的整片擦除，
 *      之前写入的内容不会保存。
 * @param offsetAddr
 * @param numToWrite
 * @param buffer
 * @return
 */
Bool UpdateDriver_Write(u32 offsetAddr, u32 numToWrite, u8 *buffer)
{
    FLASH_Status status = FLASH_COMPLETE;
    Bool ret = TRUE;
    u32 addrx = 0;
    u32 endaddr = 0;
    u32 writeAddr = CODE_START_ADDR + offsetAddr;

    if (writeAddr < CODE_START_ADDR || writeAddr > CODE_END_ADDR
            || writeAddr % 2) //非法地址
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
            if (UpdateDriver_ReadHalfWord(addrx) != 0XFFFF)
            {
                u16 index = UpdateDriver_GetFlashSector(addrx);
                status = FLASH_EraseSector(s_flashSector[index],
                        VoltageRange_2);
                if (status != FLASH_COMPLETE)
                {
                    ret = FALSE;
                    TRACE_ERROR("\n EraseSector Write failed.");
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
                ret = FALSE;
                TRACE_ERROR("\n ProgramHalfWord Write failed.");
                break;
            }
            writeAddr += 2;
            buffer += 2;
        }
    }
    FLASH_DataCacheCmd(ENABLE);
    FLASH_Lock();
    return ret;
}

/**
 * @brief 删除index号扇区
 * @param index 扇区序号
 */
static void UpdateDriver_EraseSector(u16 index)
{
    FLASH_Unlock(); //解锁
    FLASH_DataCacheCmd(DISABLE); //FLASH擦除期间必须禁止数据缓存

    FLASH_EraseSector(index, VoltageRange_2);

    FLASH_DataCacheCmd(ENABLE);
    FLASH_Lock();
}

/**
 * @brief 擦除由几个扇区组成的区域
 * @param startAddr 区域的起始地址
 * @param endAddr 区域的终止地址
 */
static void UpdateDriver_EraseRegion(u32 startAddr, u32 endAddr)
{
    u8 startSector;
    u8 endSector;

    startSector = UpdateDriver_GetFlashSector(startAddr);//获取起始地址所在的扇区号
    endSector = UpdateDriver_GetFlashSector(endAddr);

    for (u8 i = startSector; i <= endSector; i++)
    {
        UpdateDriver_EraseSector(s_flashSector[i]);
    }
    TRACE_MARK("\n startSector: 0x%x endSector: 0x%x",
            s_flashSector[startSector], s_flashSector[endSector]);
}

static Bool UpdateDriver_EraseCheck(u32 startAddr, u32 endAddr)
{
    for (; startAddr <= endAddr; startAddr += 2)
    {
        if (UpdateDriver_ReadHalfWord(startAddr) != 0xffff)
        {
            TRACE_ERROR("\n startAddr = 0x%x , data 0x%x", startAddr,
                    UpdateDriver_ReadHalfWord(startAddr));
            return FALSE;
        }
    }
    return TRUE;
}

Bool UpdateDriver_Erase(void)
{
    UpdateDriver_EraseRegion(CODE_START_ADDR, CODE_END_ADDR);
    if (TRUE != UpdateDriver_EraseCheck(CODE_START_ADDR, CODE_END_ADDR))
    {
        TRACE_ERROR(
                "\n Code erase verify error,found that the region is not all of the 1.");
        return FALSE;
    }

    return TRUE;

}

/**
 * @brief 系统复位。
 * @details 复位后，系统重新启动。
 */
static void UpdateDriver_Reset(void)
{
    NVIC_SystemReset();
}

static Bool UpdateDriver_CheckCodeExist(void)
{

    u8* address = (u8*) CODE_START_ADDR;

    for (u8 i = 0; i < CODE_EXIST_FLAG_LEN; i++)
    {
        if (address[i] == 0xff)
        {
            TRACE_ERROR("\n Code does not exist.");
            return FALSE;
        }
    }
    return TRUE;
}

#ifdef _CS_APP
/**
 * @note 由于Update程序是在FLASH的最前面，每次复位启动都是先启动Update程序然后在跳转到APP程序。
 * 如果Update程序被破坏后有很大可能进不了APP程序，从而整个子控器变砖。所以只有Update存在并且是栈顶是
 * 正确的。才会复位程序。
 */
void UpdateDriver_JumpToUpdater(void)
{
    u8* ptr = (u8*) BASE_ADDR_BSL_FLAG;
    char flg[] = BL_ENTER_UPDATE_FLAG;

    if (TRUE == UpdateDriver_CheckCodeExist())
    {
        if (((*(vu32*) CODE_START_ADDR) & 0x2FFE0000) == 0x20000000) //检查栈顶地址是否合法.
        {
            memcpy(ptr, flg, BL_ENTER_UPDATE_FLAG_LEN);
            UpdateDriver_Reset();
            while (1)
                ;
        }
        else
        {
            TRACE_ERROR("\n App stack top address illegal.");
        }
    }
}
#else
void UpdateDriver_JumpToApplication(void)
{
    u8* ptr = (u8*) BASE_ADDR_BSL_FLAG;
    char flg[] = BL_ENTER_APP_FLAG;
    memcpy(ptr, flg, BL_ENTER_APP_FLAG_LEN);
    UpdateDriver_Reset();
    while(1);
}

static void UpdateDriver_RunAPP(void)
{
    JumpFun jumpToAPP;
    uint32_t jumpAddress;

    if (((*(vu32*) CODE_START_ADDR) & 0x2FFE0000) == 0x20000000) //检查栈顶地址是否合法.
    {
        jumpAddress = *(vu32*) (CODE_START_ADDR + RESET_VECTOR_OFFSET);
        jumpToAPP = (JumpFun) jumpAddress;
        //用户代码区第二个字为程序开始地址(复位地址)
        __set_MSP(*(vu32*) CODE_START_ADDR);
        //初始化 APP 堆栈指针(用户代码区的第一个字用于存放栈顶地址)
        jumpToAPP();//跳转到 APP.
    }
    else
    {
        TRACE_ERROR("\n App stack top address illegal.");
    }
}

void UpdateDriver_Boot(void)
{
    u8* ptr = (u8*) BASE_ADDR_BSL_FLAG;
    char flg[] = BL_ENTER_UPDATE_FLAG;
    if (memcmp(flg, ptr, BL_ENTER_UPDATE_FLAG_LEN) != 0)//判断是否要求留在升级程序
    {
        if (TRUE == UpdateDriver_CheckCodeExist())
        {
            UpdateDriver_RunAPP();
        }
    }
}
#endif
