/**
 * @file
 * @brief MCU Flash读写驱动。
 * @details 提供flash读写功能接口。
 * @version 1.1.0
 * @author kim.xiejinqiang
 * @date 2012-10-30
 */
#ifndef SRC_DRIVER_MCUFLASH1_H_
#define SRC_DRIVER_MCUFLASH1_H_

#include "stm32f4xx.h"
#include "Common/Types.h"
void McuFlash_EraseSector(u32 addr);
void McuFlash_Read(u32 readAddr, u32 numToRead, u8 *buffer);
Bool McuFlash_DeleteWrite(u32 writeAddr, u32 numToWrite, u8 *buffer);
void McuFlash_Write(u32 WriteAddr, u32 NumToWrite, u8 *buffer);


#endif /* SRC_DRIVER_MCUFLASH1_H_ */
