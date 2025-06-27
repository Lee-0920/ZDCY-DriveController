/**
 * @page page_DeviceUpdateInterface 设备升级接口
 *  设备升级接口提供了设备程序更新的一序列操作，通过该接口，能为设备在线地更新程序。
 *
 *  具体命令见： @ref module_DeviceUpdateInterface
 *
 * @section sec_DUI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.26)
 *      - 支持分区域擦写；
 *      - 支持擦写校验和双重写入校验；
 *      - 支持升级模式进入事件；
 *
 */

/**
 * @addtogroup module_DeviceUpdateInterface 设备升级接口
 * @{
 */

/**
 * @file
 * @brief 设备升级接口定义。
 * @details 定义了一序列用于更新设备软件的操作。
 * @version 1.0.0
 * @author xiejinqiang
 * @date 2016-4-26
 */

#ifndef DSCP_DAVICE_UPDATE_INTERFACE_H_
#define DSCP_DAVICE_UPDATE_INTERFACE_H_

#define DSCP_DUI_CBASE                  0x0000 + 0x4000     ///< 命令基值
#define DSCP_DUI_EBASE                  0x8000 + 0x4000     ///< 事件基值
#define DSCP_DUI_SBASE                  0x0000 + 0x4000     ///< 状态基值


#include "DeviceInfoInterface.h"
#include "DeviceStatusInterface.h"

// *******************************************************************
// 命令和回应


/**
 * @brief 查询程序当前的运行模式。
 * @details 在执行后继升级命令之前，必须确保当前程序处于 Updater 模式。
 * @return 运行模式，Uint8。请参考 @ref DeviceRunMode 。
 * @note App 模式和 Updater 模式都支持本命令。见： @ref DSCP_CMD_DUI_GET_RUN_MODE ，
 *  注意这两条命令的码值是一致，只是名称不同而已。
 */
#define DSCP_CMD_DUI_GET_RUN_MODE                   DSCP_CMD_DSI_GET_RUN_MODE

/**
 * @brief 查询设备的类型。
 * @details 应用时，类型字串应该包含产品类型相关信息。
 * @return 设备的类型，String，以0结尾的ASCII字符串。
 * @note App 模式和 Updater 模式都支持本命令。
 *  App 模式下命令为： @ref DSCP_CMD_DII_GET_TYPE ，注意命令码和定义都是一致的，
 *  仅名称不同而已。
 */
#define DSCP_CMD_DUI_GET_TYPE                       DSCP_CMD_DII_GET_TYPE

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
#define DSCP_CMD_DUI_GET_VERSION                    (DSCP_DUI_CBASE + 0x02)

/**
 * @brief 查询 Updater 支持的最大分片大小。
 * @details 在调用 @ref DSCP_CMD_DUI_WRITE_PROGRAM 命令进行数据烧写时，
 *  最大的分片大小不能超过设备的限制长度。
 * @return 最大分片大小，Uint16，单位为字节。
 */
#define DSCP_CMD_DUI_GET_MAX_FRAGMENT_SIZE          (DSCP_DUI_CBASE + 0x03)

/**
 * @brief 进入Updater模式。
 * @details 在 App 模式下调用本命令，程序将跳转到升级模式，等待升级相关的指令。
 * @return 无回应。
 * @note 仅在 App 模式下支持本命令。
 */
#define DSCP_CMD_DUI_ENTER_UPDATER                  (DSCP_DUI_CBASE + 0x04)

/**
 * @brief 进入应用程序。
 * @details 在 Updater 模式下调用本命令，程序将跳转到应用程序。
 *  通常在升级未开始前不打算继续升级，或者在升级完成后调用。
 * @return 无回应。
 */
#define DSCP_CMD_DUI_ENTER_APPLICATION              (DSCP_DUI_CBASE + 0x05)

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
#define DSCP_CMD_DUI_ERASE                          (DSCP_DUI_CBASE + 0x06)

/**
 * @brief 向设备写程序数据。
 * @details 升级管理程序需要对程序文件进行分片，然后针对每一分片，按序调用本操作更新程序。
 *  设备支持的最大分片大小限制参见： @ref DSCP_CMD_DUI_GET_MAX_FRAGMENT_SIZE 命令。
 *  DSCP_CMD_DUI_WRITE_PROGRAM中使用的帧序列号，在DSCP_CMD_DUI_CHECK_INTEGRITY命令中恢复为0
 * @param length Uint16，分片数据的长度，单位为字节
 * @param seq Uint16，分片序号。Updater 一旦接收到 @ref DSCP_CMD_DUI_ERASE 命令，
 *  其期望的分片序号被复位为0。Updater 每正确接收到一条 DSCP_CMD_DUI_WRITE_PROGRAM 命令，
 *  其期望的分片序号增1。
 * @param data[length] Byte[]，分片数据，具体长度取决于length参数。
 * @return 执行状态：
 *  - @b status Uint16，执行状态，支持的状态有：
 *      - @ref DSCP_OK 操作成功；
 *      - @ref DUI_WRITE_CHECK_ERROR 写入校验错误，写完后立即检查，发现与写入的数据不同；
 *      - @ref DUI_WRITE_FAILED 写入失败；
 *      - @ref DUI_WRITE_SIZE_ERROR 错误的数据长度；
  *  - @b seq Uint16，期望序号；
 * @note 请确保已先正确调用 @ref DSCP_CMD_DUI_ERASE 命令。
 */
#define DSCP_CMD_DUI_WRITE_PROGRAM                  (DSCP_DUI_CBASE + 0x07)

/**
 * @brief 程序完整性检验。
 * @details 对整个目标升级程序涉及到的所有数据进行检验，只有校验成功才能跳转到应用程序。
 *  校验失败时，升级管理程序应该且只能从头（块擦除）开始尝试再次升级。
 *  如果校验失败，即使强行重启，设备仍将进入 Updater，等待下一次的升级尝试。
 * @param checksum Uint16，期望的 CRC16-CCITT 校验和数值。
 *  0 表示不需要校验，强行认为写入的程序是完整的，目标设备无需校验。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  校验成功；
 *  - @ref DSCP_ERROR 校验失败；
 */
#define DSCP_CMD_DUI_CHECK_INTEGRITY                (DSCP_DUI_CBASE + 0x08)



// *******************************************************************
// 事件

/**
 * @brief 设备 Updater 进入事件。
 * @details 设备进入 Updater 后，将产生该事件，以提示上位机需要特殊处理。
 *  该事件对于设备异常进入 Updater 时，对上位机判别有帮助。
 *  <p>频度：一次
 */
#define DSCP_EVENT_DUI_ENTER_UPDATER                (DSCP_DUI_EBASE + 0x00)


/**
 * @brief 擦除操作结果。
 */
typedef enum
{
    ERASE_RESULT_FINISHED = 0,          ///< 擦除完成
    ERASE_RESULT_CHECK_ERROR = 1,       ///< 擦除校验错误
}EraseResult;

/**
 * @brief 擦除FLASH结果事件。
 * @details 擦除完成后产生该事件。
 * @param result Uint16，擦除结果码（ @ref EraseResult ），定义如下：
 *  - @ref ERASE_RESULT_FINISHED  擦除完成
 *  - @ref ERASE_RESULT_CHECK_ERROR  擦除校验错误
 */
#define DSCP_EVENT_DUI_ERASE_RESULT                 (DSCP_DUI_EBASE + 0x01)

// *******************************************************************
// 状态返回

#define DUI_ERASE_FAILED                (DSCP_DUI_SBASE + 0x01)    ///< 擦除失败
#define DUI_ERASE_CHECK_ERROR           (DSCP_DUI_SBASE + 0x02)    ///< 擦除校验错误
#define DUI_SEQ_ERROR                   (DSCP_DUI_SBASE + 0x03)    ///< 分片序号错误
#define DUI_WRITE_FAILED                (DSCP_DUI_SBASE + 0x04)    ///< 写入操作失败
#define DUI_WRITE_CHECK_ERROR           (DSCP_DUI_SBASE + 0x05)    ///< 写校验错误，结果与期望不一致
#define DUI_ADDR_ERROR                  (DSCP_DUI_SBASE + 0x06)    ///< 地址错误，超出可写范围
#define DUI_ERASE_PARAM_ERROR           (DSCP_DUI_SBASE + 0x07)    ///< 地址或长度参数有误
#define DUI_WRITE_SIZE_ERROR            (DSCP_DUI_SBASE + 0x08)    ///< 报文提供了错误的数据长度
#define DUI_WRITE_SEQ_ERROR             (DSCP_DUI_SBASE + 0x09)    ///< 错误的帧序号
#define DUI_WRITE_BLANK_ERROR           (DSCP_DUI_SBASE + 0x0a)    ///< 写入的数据段非空

#endif // DSCP_DAVICE_UPDATE_INTERFACE_H_

/** @} */
