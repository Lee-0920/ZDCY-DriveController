/**
 * @page page_AnalogControlInterface 模拟量控制接口
 *  模拟量控制接口提供了采集模拟量的相关操作。
 *
 *  具体命令见： @ref module_AnalogControlInterface
 *
 * @section sec_ACI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2020.3.3)
 *
 */

/**
 * @addtogroup module_AnalogControlInterface 模拟量控制接口
 * @{
 */

/**
 * @file
 * @brief 模拟量控制接口
 * @details 定义了一系列输入输出模拟量的操作。
 * @version 1.0.0
 * @author xingfan
 * @date 2020-03-03
 */

#ifndef DSCP_ANALOG_CONTROL_INTERFACE_H_
#define DSCP_ANALOG_CONTROL_INTERFACE_H_

#define DSCP_ACI_CBASE                  0x0000 + 0x0800     ///< 命令基值
#define DSCP_ACI_EBASE                  0x8000 + 0x0800     ///< 事件基值
#define DSCP_ACI_SBASE                  0x0000 + 0x0800     ///< 状态基值

/**
 * @brief 查询模拟输入量接口数目。
 * @return 模拟输入量数目，Uint16。
 */
#define DSCP_CMD_ACI_GET_AI_NUM              (DSCP_ACI_CBASE + 0x00)

/**
 * @brief 查询模拟量数值。
 * @param index Uint16，要查询的模拟输入量索引。
 * @return 当前模拟输入量，Uint32。
 */
#define DSCP_CMD_ACI_GET_AI_DATA             (DSCP_ACI_CBASE + 0x01)

/**
 * @brief 获取所有模拟量数据。
 * @return 所有模拟输入量数据
 * - num Uint16，当前模拟量个数
 * - data Uint32*，模拟量AD值数组
 */
#define DSCP_CMD_ACI_ALL_AI_DATA             (DSCP_ACI_CBASE + 0x02)

/**
 * @brief 设置模拟输入量上报间隔
 * @param time Uint16， 上报间隔，单位：秒。（0表示不需要主动上传）
 * @return 状态回应，Uint16，支持的状态有：
 * @retval DSCP_OK  操作成功
 * @retval DSCP_ERROR 操作失败
 */
#define DSCP_CMD_ACI_SET_AI_UPLOAD_PERIOD            (DSCP_ACI_CBASE + 0x03)

// *******************************************************************
// 事件

/**
 * @brief 模拟输入量定时上报事件。
 * @details 系统将根据设置的上报周期，定时上传所有模拟量最新数值。
 *  上报周期可通过命令 @ref DSCP_CMD_ACI_SET_AI_UPLOAD_PERIOD 设定。
 * @param num Uint16，当前模拟量个数。
 * @param data Uint32*，模拟量AD值数组。
 */
#define DSCP_EVENT_ACI_AI_UPLOAD             (DSCP_ACI_EBASE + 0x00)

// *******************************************************************
// 状态返回

#endif /* DSCP_ANALOG_CONTROL_INTERFACE_H_ */

/** @} */
