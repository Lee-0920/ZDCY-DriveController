/**
 * @page page_DeviceInfoInterface 设备信息接口
 *  设备信息接口提供了设备基本信息查询的一序列操作，同时提供了生产信息的写入操作。
 *
 *  具体命令见： @ref module_DeviceInfoInterface
 *
 * @section sec_DeviceInfoInterface_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.26)，由原各设备不同的命令整理而成，主要修改内容为：
 *      - 版本号由8字节缩减为4字节，PCB版本号和BOM版本号合并为硬件版本号的主次版本域
 *      - 设备名称和设备型号改为字符串
 *
 */

/**
 * @addtogroup module_DeviceInfoInterface 设备信息接口
 * @{
 */

/**
 * @file
 * @brief 设备信息接口定义。
 * @details 定义了一序列用于查询或者设置设备（板卡）信息的操作。
 * @version 1.0.0
 * @author xiejinqiang
 * @date 2016-4-26
 */

#ifndef DSCP_DAVICE_INFO_INTERFACE_H_
#define DSCP_DAVICE_INFO_INTERFACE_H_

#define DSCP_DII_CBASE                  0x0000 + 0x0100     ///< 命令基值
#define DSCP_DII_EBASE                  0x8000 + 0x0100     ///< 事件基值
#define DSCP_DII_SBASE                  0x0000 + 0x0100     ///< 状态基值


// *******************************************************************
// 命令和回应


/**
 * @brief 查询设备的类型名称。
 * @details 设备类型是区分不同类别的产品称号，上位机（PC、终端）将根据这个字段
 *  识别不同的设备。各类型的字条串列举如下：
 *  - 主控板：LUMC-S
 *  - 液控板：LULC-S
 *  - 温控板：LUTC-S
 *  - 信号板：LUSC-S
 *  - 驱动板：LUDC-S
 * @return 设备类型，String，以0结尾的ASCII字符串，长度限制为16字符以内。
 * @note 1、各设备在实现该命令时，必须把本字段固化在程序中，即在烧录软件时就设置了正确
 *  的设备类型字串。一般情况下，本字段不允许任何修改，即使生产工装也不会修改。
 * \n 2、具体的设备组件对应的类型名称请参考：《板卡类型规范.pdf》。
 */
#define DSCP_CMD_DII_GET_TYPE                       (DSCP_DII_CBASE + 0x00)

/**
 * @brief 设置设备的类型名称。
 * @param name 设备类型，String，以0结尾的ASCII字符串，长度限制为16字符以内。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @note 通常情况下，本命令不对外公开，上位机（包括生产工具）不能随便更改设备类型。
 * @see DSCP_CMD_DII_GET_TYPE
 */
#define DSCP_CMD_DII_SET_TYPE                       (DSCP_DII_CBASE + 0x01)

/**
 * @brief 查询设备的型号。
 * @details 设备型号包含了设备的类型名称和具体的型别，如某钥匙型号为：LULC8201。
 * @return 设备型号，String，以0结尾的ASCII字符串，长度限制为24字符以内。
 */
#define DSCP_CMD_DII_GET_MODEL                      (DSCP_DII_CBASE + 0x02)

/**
 * @brief 设置设备的型号。
 * @details 设备型号包含了设备的名称和具体的型别，如某钥匙型号为：LULC8201。
 * @param model 设备型号，String，以0结尾的ASCII字符串，长度限制为24字符以内。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @see DSCP_CMD_DII_GET_MODEL
 */
#define DSCP_CMD_DII_SET_MODEL                      (DSCP_DII_CBASE + 0x03)

/**
 * @brief 查询设备的序列号。
 * @details 设备序列号唯一标识了个体设备的整个生命周期，供产品追溯时使用。
 * @return 设备序列号，String，以0结尾的ASCII字符串，长度限制为32字符以内。
 * @note 设备序列号的具体格式请参考公司产品相关文档。
 */
#define DSCP_CMD_DII_GET_SN                         (DSCP_DII_CBASE + 0x04)

/**
 * @brief 设置设备的序列号。
 * @details 设备序列号唯一标识了个体设备的整个生命周期，供产品追溯时使用。
 * @param sn 设备序列号，String，以0结尾的ASCII字符串，长度限制为32字符以内。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @note 设备序列号的具体格式请参考公司产品相关文档。
 * @see DSCP_CMD_DII_GET_SN
 */
#define DSCP_CMD_DII_SET_SN                         (DSCP_DII_CBASE + 0x05)

/**
 * @brief 查询设备的生产厂商。
 * @details 生产厂商指明了生产该产品个体的厂商，可用于产品的追溯。
 * @return 设备的生产厂商，String，以0结尾的ASCII字符串，长度限制为20字符以内。
 */
#define DSCP_CMD_DII_GET_MANUFACTURER               (DSCP_DII_CBASE + 0x08)

/**
 * @brief 设置设备的生产厂商。
 * @details 生产厂商指明了生产该产品个体的厂商，可用于产品的追溯。
 * @param manufacturer 设备的生产厂商，String，以0结尾的ASCII字符串，长度限制为20字符以内。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @see DSCP_CMD_DII_GET_MODEL
 */
#define DSCP_CMD_DII_SET_MANUFACTURER               (DSCP_DII_CBASE + 0x09)

/**
 * @brief 查询设备的生产日期。
 * @return 设备的生产日期，Byte[4]：
 *  - @b year Uint16，年。
 *  - @b month Uint8，月。
 *  - @b day Uint8，日。
 */
#define DSCP_CMD_DII_GET_MANUF_DATE                 (DSCP_DII_CBASE + 0x0A)

/**
 * @brief 设置设备的生产日期。
 * @param year Uint16，年。
 * @param month Uint8，月。
 * @param day Uint8，日。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @see DSCP_CMD_DII_GET_MANUF_DATE
 */
#define DSCP_CMD_DII_SET_MANUF_DATE                 (DSCP_DII_CBASE + 0x0B)


/**
 * @brief 查询软件版本号。
 * @details 软件版本号通常固化在软件代码中，可作后期运维管理用。
 * @return 版本号，Byte[4]：
 *  - @b major Uint8，主版本号；
 *  - @b minor Uint8，次版本号；
 *  - @b revision Uint8，修订版本号；
 *  - @b build Uint8，编译版本号；
 */
#define DSCP_CMD_DII_GET_SOFTWARE_VERSION           (DSCP_DII_CBASE + 0x0C)

/**
 * @brief 查询软件标识。
 * @details 软件标识可作为软件版本的补充，如客户定制版本、分支标号、编译日期等。
 *  标准版本（非定制）的格式统一为： STD-[SVN Revision]-[YYYY.MM.DD]
 * @return 软件标识，String，以0结尾的ASCII字符串，长度限制为64字符以内。
 */
#define DSCP_CMD_DII_GET_SOFTWARE_LABEL             (DSCP_DII_CBASE + 0x0D)

/**
 * @brief 查询硬件版本号。
 * @details 硬件版本号通常分为PCB和BOM两级，如果硬件上不支持则设为0。其余字段为0。
 * @return 版本号，Byte[4]：
 *  - @b major Uint8，主版本号，也即是 PCB 版本号；
 *  - @b minor Uint8，次版本号，也即是 BOM 版本号；
 *  - @b revision Uint8，修订版本号，保留；
 *  - @b build Uint8，编译版本号，保留；
 */
#define DSCP_CMD_DII_GET_HARDWARE_VERSION           (DSCP_DII_CBASE + 0x0E)



// *******************************************************************
// 事件


// *******************************************************************
// 状态返回


#endif // DSCP_DAVICE_INFO_INTERFACE_H_

/** @} */
