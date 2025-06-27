/**
 * @page page_DataTransmitInterface Modbus串口数据传输接口
 *  Modbus串口数据传输接口提供了Modbus协议命令的相关操作。
 *
 *  具体命令见： @ref module_DataTransmitInterface
 *
 * @section sec_OAI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2020.4.12)
 *
 */

/**
 * @addtogroup DataTransmitInterface Modbus串口数据传输接口
 * @{
 */

/**
 * @file
 * @brief Modbus串口数据传输接口。
 * @details 定义了一系列Modbus协议命令的相关操作。
 * @version 1.1.0
 * @author ludijun
 * @date 2020.4.12
 */

#ifndef DSCP_DATA_TRANSMIT_INTERFACE_H_
#define DSCP_DATA_TRANSMIT_INTERFACE_H_

#define DSCP_DTI_CBASE                  0x0000 + 0x0B00     ///< 命令基值
#define DSCP_DTI_EBASE                  0x8000 + 0x0B00     ///< 事件基值
#define DSCP_DTI_SBASE                  0x0000 + 0x0B00     ///< 状态基值

/**
 * @brief 停止位
 */
typedef enum
{
    StopBits_1 = 0,
    StopBits_1_5 = 1,
    StopBits_2 = 2,
}StopBitsType;

/**
 * @brief 校验位
 */
typedef enum
{
    Parity_No = 0,
    Parity_Even = 1,
    Parity_Odd = 2,
}ParityType;

/**
 * @brief 串口配置参数
 */
typedef struct
{
    Uint32 baud;
    Uint8 wordLen;
    StopBitsType stopBits;
    ParityType parity;
}SerialPortParam;

// *******************************************************************
// 命令和回应

/**
 * @brief 查询系统支持的串口总数。
 * @return  num Uint16，串口数。
 */
#define DSCP_CMD_DTI_GET_PORT_NUM                           (DSCP_DTI_CBASE + 0x00)

/**
 * @brief 发送数据到指定的串口。
 * @details 发送Modbus协议命令数据。
 * @param index Uint16，串口编号。
 * @param data Uint8*，需要发送的数据。
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */

#define DSCP_CMD_DTI_SEND_DATA                              (DSCP_DTI_CBASE + 0x01)

/**
 * @brief 查询指定串口的配置参数。
 * @param index Uint16，串口索引。
 * @return 串口配置参数
 * @see SerialPortParam
 *  - baud Uint32，波特率
 *  - wordLen Uint8，数据位
 *  - stopBits Uint8，停止位
 *  - parity Uint8，校验位
 */
#define DSCP_CMD_DTI_GET_PORT_PARAM                         (DSCP_DTI_CBASE + 0x02)

/**
 * @brief 设置指定串口的配置参数，参数永久保存到flash。
 * @param index Uint16，串口索引。
 * @param param SerialPortParam，串口配置参数
 * @see SerialPortParam
 *  - baud Uint32，波特率
 *  - wordLen Uint8，数据位
 *  - stopBits Uint8，停止位
 *  - parity Uint8，校验位
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
#define DSCP_CMD_DTI_SET_PORT_PARAM                         (DSCP_DTI_CBASE + 0x03)


// *******************************************************************
// 事件
///**
// * @brief Modbus数据
// */


/**
 * @brief 数据转发上传事件。
 * @details 系统将收到的串口Modbus数据,返回对应串口的数据包。
 * @return 返回对应串口的数据包
 * - index Uint16，串口索引
 * - len Uint16，数据长度
 * - data Uint8*，上传的数据
 */
#define DSCP_EVENT_DTI_UPLOAD_DATA                          (DSCP_DTI_EBASE + 0x00)


/**
 * @brief 下位机Log上传事件
 * @details 系统将收到的Log储存记录。
 * @return 返回对应串口的数据包
 * - len Uint16，数据长度
 * - data Uint8*，上传的数据
 */
#define DSCP_EVENT_DTI_UPLOAD_LOG                       (DSCP_DTI_EBASE + 0x01)


// *******************************************************************
// 状态返回

#endif // DSCP_OPTICAL_ACQUIRE_INTERFACE_H_

/** @} */

