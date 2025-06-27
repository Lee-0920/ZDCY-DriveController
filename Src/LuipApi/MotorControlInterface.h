/**
 * @page page_MotorControlInterface 电机控制接口
 *  电机控制接口提供了电机移动的相关操作。
 *
 *  具体命令见： @ref module_MotorControlInterface
 *
 * @section sec_MCI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2018.1.17)
 *
 */

/**
 * @addtogroup module_MotorControlInterface 电机控制接口
 * @{
 */

/**
 * @file
 * @brief 电机控制接口。
 * @details 定义了一序列电机控制相关的操作。
 * @version 1.0.0
 * @author yongxi
 * @date 2018.1.17
 */

#ifndef DSCP_MOTOR_CONTROL_INTERFACE_H_
#define DSCP_MOTOR_CONTROL_INTERFACE_H_

#define DSCP_MCI_CBASE                  0x0000 + 0x0600     ///< 命令基值
#define DSCP_MCI_EBASE                  0x8000 + 0x0600     ///< 事件基值
#define DSCP_MCI_SBASE                  0x0000 + 0x0600     ///< 状态基值


// *******************************************************************
// 命令和回应


/**
 * @brief 查询系统操作的电机数目。
 * @return 总电机数目， Uint16。
 */
#define DSCP_CMD_MCI_GET_TOTAL_MOTORS           (DSCP_MCI_CBASE + 0x00)

/**
 * @brief 查询指定电机的参数（不使用）。
 */
#define DSCP_CMD_MCI_GET_MOTION_PARAM           (DSCP_MCI_CBASE + 0x01)

/**
 * @brief 设置指定电机的参数（不使用）。
 */
#define DSCP_CMD_MCI_SET_MOTION_PARAM           (DSCP_MCI_CBASE + 0x02)

/**
 * @brief 查询指定电机的工作状态（留样定位泵）。
 * @param index Uint8，要查询的电机索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_IDLE 空闲；
 *  - @ref DSCP_BUSY 忙碌，需要停止后才能做下一个动作；
 */
#define DSCP_CMD_MCI_GET_MOTOR_STATUS           (DSCP_MCI_CBASE + 0x03)

/**
 * @brief 查询指定电机的最大移动度数（留样定位泵）。
 * @param index Uint8，要查询的电机索引。
 * @return 度数， float。
 */
#define DSCP_CMD_MCI_GET_MOTOR_MAX_STEPS        (DSCP_MCI_CBASE + 0x04)

/**
 * @brief 查询指定电机的初始位置度数（留样定位泵）。
 * @param index Uint8，要查询的电机索引。
 * @return 度数， float。
 */
#define DSCP_CMD_MCI_GET_MOTOR_INIT_STEPS       (DSCP_MCI_CBASE + 0x05)

/**
 * @brief 查询指定电机的当前移动度数。
 * @param index Uint8，要查询的电机索引。
 * @return 度数， float。
 */
#define DSCP_CMD_MCI_GET_MOTOR_CURRENT_STEPS    (DSCP_MCI_CBASE + 0x06)

/**
 * @brief 电机移动（留样定位泵）。
 * @details 启动后，不管成功与否，操作结果都将以事件的形式上传给上位机。关联的事件有：
 *   - @ref DSCP_EVENT_MCI_MOTOR_RESULT
 * @param index Uint8，要操作的电机索引。
 * @param degree float，电机移动度数，[0~360)。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败，如电机正在运转，无法启动电机，需要先停止；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 * @note 该命令将立即返回，电机移动完成将以事件的形式上报。
 */
#define DSCP_CMD_MCI_MOTOR_MOVE                 (DSCP_MCI_CBASE + 0x07)

/**
 * @brief 停止电机（留样定位泵）。
 * @param index Uint8，要操作的电机索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_MCI_MOTOR_STOP                 (DSCP_MCI_CBASE + 0x08)

/**
 * @brief 电机复位（留样定位泵）。
 * @details 电机回到0位置，不管成功与否，操作结果都将以事件的形式上传给上位机。关联的事件有：
 *   - @ref DSCP_EVENT_MCI_MOTOR_RESULT
 * @param index Uint8，要操作的电机索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_MCI_MOTOR_RESET                (DSCP_MCI_CBASE + 0x09)

/**
 * @brief 查询指定位置传感器的状态。
 * @param index Uint8，要查询的位置传感器索引。留样定位泵（0~1）,液位传感器(2~3)
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_IDLE 空闲, 传感器未被遮挡；
 *  - @ref DSCP_BUSY 忙碌，传感器被遮挡。
 */
#define DSCP_CMD_MCI_GET_SENSOR_STATUS           (DSCP_MCI_CBASE + 0x0A)

/**
 * @brief 电机驱动配置初始化（留样定位泵）。
 * @details 所有电机驱动配置初始化，不管成功与否，操作结果都将以事件的形式上传给上位机。关联的事件有：
 *   - @ref DSCP_EVENT_MCI_MOTOR_RESULT
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_MCI_DRIVER_REINIT               (DSCP_MCI_CBASE + 0x0B)

/**
 * @brief 检查指定电机驱动的状态（留样定位泵）。
 * @param index Uint8，要查询的位置电机驱动索引。
 * @return gstat Uint8，驱动全局寄存器的值
 */
#define DSCP_CMD_MCI_DRIVER_CHECK           (DSCP_MCI_CBASE + 0x0C)

/**
 * @brief 启动直流电机（试剂泵）。
 * @param index Uint8，要操作的直流电机索引。
 * @param dir   Uint8，方向，与泵控制一致。
 * @param time 	float，时间（秒），最多支持3位小数（即Ms）。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_MCI_DC_MOTOR_SET_STATE               (DSCP_MCI_CBASE + 0x0D)

/**
 * @brief 查询直流电机状态（试剂泵）
 * @param index Uint8，要查询直流电机索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_IDLE 空闲；
 *  - @ref DSCP_BUSY 忙碌还未完成。
 */
#define DSCP_CMD_MCI_DC_MOTOR_GET_STATE               (DSCP_MCI_CBASE + 0x0E)

/**
 * @brief 停止电机（试剂泵）
 * @param index Uint8，要查询直流电机索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_MCI_DC_MOTOR_STOP_STATE               (DSCP_MCI_CBASE + 0x0F)

/**
 * @brief 设置搅拌电机功率
 * @param index Uint8，要查询直流电机索引。
 * @param level float，功率0~1(一位小数)
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_MCI_STIR_MOTOR_SET_LEVEL               (DSCP_MCI_CBASE + 0x10)
// *******************************************************************
// 事件

/**
 * @brief 电机操作结果。
 */
//enum MotorResult
//{
//    MOTOR_RESULT_FINISHED = 0,       ///< 电机操作正常完成。
//    MOTOR_RESULT_FAILED = 1,         ///< 电机操作中途出现故障，未能完成。
//    MOTOR_RESULT_STOPPED = 2         ///< 电机操作被停止。
//};

/**
 * @brief 电机操作结果事件。
 * @details 电机移动操作结束时将产生该事件。
 * @param index Uint8，产生事件的泵索引，0号泵为留样定位泵，1号为试剂泵。
 * @param result Uint8，泵操作结果码（ @ref MotorResult ），定义如下：
 *  - @ref MOTOR_RESULT_FINISHED  电机操作正常完成；
 *  - @ref MOTOR_RESULT_FAILED  电机操作中途出现故障，未能完成。
 *  - @ref MOTOR_RESULT_STOPPED  电机操作被停止。
 */
#define DSCP_EVENT_MCI_MOTOR_RESULT             (DSCP_MCI_EBASE + 0x00)
// *******************************************************************
// 状态返回




#endif // DSCP_MOTOR_CONTROL_INTERFACE_H_

/** @} */
