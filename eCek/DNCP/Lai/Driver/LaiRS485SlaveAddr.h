/**
 * @file LaiRS485SlaveAddr.h
 * @brief LaiRS485从机硬件地址。
 * @details 对各个下位机板的各个硬件地址引脚进行宏定义。
 * @version 1.0.0
 * @author xingfan
 * @date 2016-05-25
 */
#ifndef ECEK_DNCP_LAI_DRIVER_LAIRS485SLAVEADDR_H_
#define ECEK_DNCP_LAI_DRIVER_LAIRS485SLAVEADDR_H_
#include "Common/Types.h"

extern void LaiRS485SlaveAddr_GPIOConfig(void);
Uint8 LaiRS485SlaveAddr_GetAddr(void);
#endif /* ECEK_DNCP_LAI_DRIVER_LAIRS485SLAVEADDR_H_ */
