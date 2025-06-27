/**
 * @addtogroup module_Manufacture
 * @{
 */

/**
 * @file
 * @brief 定义生产信息支持模块的一些公用类型。
 * @details
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-6-21
 */

#include "Common/Types.h"

#ifndef MANUFACTURE_MANUF_TYPES_H_
#define MANUFACTURE_MANUF_TYPES_H_

/**
 * @brief 生产序列号，32位。
 */
typedef unsigned char ManufSn[32];//

/**
 * @brief 设备的类型名称,16个字符。
 */
typedef unsigned char ManufType[16];

/**
 * @brief 产品型号，24位。
 */
typedef unsigned char ManufModel[24];//24

/**
 * @brief 生产厂商。
 */
typedef char ManufVenderString[20];


/**
 * @brief 生产日期。
 */
typedef struct
{
    Uint16 year;        ///< 年
    Uint8 month;        ///< 月
    Uint8 day;          ///< 日
}
ManufDate;

/**
 * @brief 生产信息版本号。
 * @details 对于没有全部实现的版本号，只取其最前的版本域。
 *  假如 BOM 版本号设计时只用到一段，则应该使用 major 域。
 */
typedef struct
{
    Uint8 major;       ///< 主版本号
    Uint8 minor;       ///< 次版本号
    Uint8 revision;    ///< 修订版本号
    Uint8 build;       ///< 编译版本号
}
ManufVersion;


/**
 * @brief 生产信息。
 * @details 定义所有生产信息的存储格式。
 */
typedef struct
{
    ManufSn sn;                     ///< 序列号
    ManufType type;                 ///< 设备的类型名称
    ManufModel model;               ///< 产品型号
    ManufVenderString vender;       ///< 生产厂商字串
    ManufDate mdate;                ///< 生产日期
}
ManufInfo;

/**
 * @brief 生产信息块。
 * @details 定义所有生产信息的存储格式。
 */
typedef struct
{
    ManufInfo info;                 ///< 生产信息
    unsigned short chksum;          ///< 校验和（CRC16），已弃用（2014.8.27）
}
ManufBlock;

#endif  // MANUFACTURE_MANUF_TYPES_H_

/** @} */
