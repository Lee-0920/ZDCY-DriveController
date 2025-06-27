/**
 * @page page_DigitalControlInterface 数字量控制接口
 *  数字量控制接口提供了控制继电器和采集开关量的相关操作。
 *
 *  具体命令见： @ref module_DigitalControlInterface
 *
 * @section sec_DCI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2020.3.3)
 *
 */

/**
 * @addtogroup module_DigitalControlInterface 数字量控制接口
 * @{
 */

/**
 * @file
 * @brief 数字量控制接口
 * @details 定义了一序列读写数字量的操作。
 * @version 1.0.0
 * @author xingfan
 * @date 2020-03-03
 */

#ifndef DSCP_DIGITAL_CONTROL_INTERFACE_H_
#define DSCP_DIGITAL_CONTROL_INTERFACE_H_

#define DSCP_DCI_CBASE                  0x0000 + 0x0900     ///< 命令基值
#define DSCP_DCI_EBASE                  0x8000 + 0x0900     ///< 事件基值
#define DSCP_DCI_SBASE                  0x0000 + 0x0900     ///< 状态基值

// *******************************************************************
// 命令和回应
/**
 * @brief 查询系统支持的输入开关量数目。
 * @return 输入开关量数目， Uint16。
 */
#define DSCP_CMD_DCI_GET_DI_NUM         (DSCP_DCI_CBASE + 0x00)

/**
 * @brief 查询输入开关量的状态。
 * @param index Uint16，要查询的开关量索引。
 * @return 开关量的状态，Uint8，FALSE 0 = 断开，TRUE 1 = 闭合。
 */
#define DSCP_CMD_DCI_GET_DI_STATUS   (DSCP_DCI_CBASE + 0x01)

/**
 * @brief 查询所有输入开关量的状态映射图。
 * @return 状态映射图，Uint32，按位表示开关量状态：位0为关，1为开。
 */
#define DSCP_CMD_DCI_GET_DI_MAP      (DSCP_DCI_CBASE + 0x02)

/**
 * @brief 查询系统支持的输出开关量数目。
 * @return 开关量数目， Uint16。
 */
#define DSCP_CMD_DCI_GET_DO_NUM   (DSCP_DCI_CBASE + 0x03)

/**
 * @brief 查询输出开关量的工作状态。
 * @param index Uint16，要查询的输出开光量索引。
 * @return 开关量的状态，Uint8，FALSE 0 = 断开，TRUE 1 = 闭合。
 */
#define DSCP_CMD_DCI_GET_DO_STATUS   (DSCP_DCI_CBASE + 0x04)

/**
 * @brief 设置输出开关量状态
 * @details 断开或闭合继电器
 * @param index Uint16， 输出开关量索引。
 * @param state Uint8， FALSE 0 = 断开，TRUE 1 = 闭合。
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
#define DSCP_CMD_DCI_SET_DO_STATUS     (DSCP_DCI_CBASE + 0x05)

/**
 * @brief 查询所有输入开关量的状态映射图。
 * @return 状态映射图，Uint32，按位表示开关量状态：位0为关，1为开。
 */
#define DSCP_CMD_DCI_GET_DO_MAP      (DSCP_DCI_CBASE + 0x06)

/**
 * @brief 设置所有输入开关量的状态映射。
 * @param map Uint32，状态映射图，位0为关，1为开。
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
#define DSCP_CMD_DCI_SET_DO_MAP      (DSCP_DCI_CBASE + 0x07)

/**
 * @brief 查询门禁状态。
 * @return 门禁状态，Uint8，FALSE 0 = 开门，TRUE 1 = 关门。
 */
#define DSCP_CMD_DCI_GET_GATE_STATUS   (DSCP_DCI_CBASE + 0x08)

/**
 * @brief 开关电子锁
 * @param state Uint8， FALSE 0 = 未上锁，TRUE 1 = 上锁。
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
#define DSCP_CMD_DCI_SET_LOCK_STATE      (DSCP_DCI_CBASE + 0x09)

/**
 * @brief 查询电子锁状态。
 * @return 电子锁状态，Uint8，FALSE 0 = 未上锁，TRUE 1 = 上锁。
 */
#define DSCP_CMD_DCI_GET_LOCK_STATUS     (DSCP_DCI_CBASE + 0x0A)

/**
 * @brief 累积流量控制开关
 * @param Uint8 FALSE 0 = 停止，TRUE 1 = 开始。
 */
#define DSCP_CMD_DCI_SET_FLOW_METER     (DSCP_DCI_CBASE + 0x0B)

// *******************************************************************
// 事件

/**
 * @brief 数字输入量上报事件。
 * @details 系统检测到输入量发生变化后，上传所有数字输入量最新值。
 * @param map Uint32，状态映射图，位0为关，1为开。
 */
#define DSCP_EVENT_DCI_DI_UPLOAD            (DSCP_DCI_EBASE + 0x00)

/**
 * @brief 门禁状态变化事件。
 * @details 系统检测到门禁状态变化后，上传当前门禁状态。
 * @param state Uint8，门禁状态，FALSE 0 = 开门，TRUE 1 = 关门。
 */
#define DSCP_EVENT_DCI_GATE_STATUS_UPLOAD     (DSCP_DCI_EBASE + 0x01)

// *******************************************************************
// 状态返回

#endif /* DSCP_DIGITAL_CONTROL_INTERFACE_H_ */

/** @} */
