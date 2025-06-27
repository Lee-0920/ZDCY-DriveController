/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 网络包格式定义。
 * @details 提供网络层数据包格式的统一定义。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-22
 */

#include "DNCP/Net/NetPack.h"

// 网络地址层域Mask表，注意索引0处为第1层级
static const NetAddress s_kLayerFieldMaskTable[] = 
{
    0x00000F00,
    0x0000F000,
    0x00FF0000,
    0xFF000000
};

// 网络地址层域右移位数表，注意索引0处为第1层级
static const Uint8 s_kLayerFieldShiftsTable[] = 
{
    8,
    12,
    16,
    24
};

Uint8 NetAddress_GetDownlinkField(NetAddress addr, Uint8 layer)
{
    --layer;
    return (Uint8)((addr & s_kLayerFieldMaskTable[layer]) >> s_kLayerFieldShiftsTable[layer]);
}

void NetAddress_SetDownlinkField(NetAddress* addr, Uint8 layer, Uint8 layerAddr)
{
    --layer;
    *addr = ((*addr) & (~s_kLayerFieldMaskTable[layer])) | (layerAddr << s_kLayerFieldShiftsTable[layer]);
}


/** @} */
