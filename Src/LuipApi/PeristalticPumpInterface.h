/**
 * @page page_PeristalticPumpInterface 蠕动泵控制接口
 *  蠕动泵控制接口提供了控制蠕动泵运转的相关操作。
 *
 *  具体命令见： @ref module_PeristalticPumpInterface
 *
 * @section sec_PPI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.26)
 *
 */

/**
 * @addtogroup module_PeristalticPumpInterface 蠕动泵控制接口
 * @{
 */

/**
 * @file
 * @brief 蠕动泵控制接口。
 * @details 定义了一序列蠕动泵控制相关的操作。
 * @version 1.0.0
 * @author xiejinqiang
 * @date 2016.4.26
 */

#ifndef DSCP_PERISTALTIC_PUMP_INTERFACE_H_
#define DSCP_PERISTALTIC_PUMP_INTERFACE_H_

#define DSCP_PPI_CBASE                  0x0000 + 0x0700     ///< 命令基值
#define DSCP_PPI_EBASE                  0x8000 + 0x0700     ///< 事件基值
#define DSCP_PPI_SBASE                  0x0000 + 0x0700     ///< 状态基值


// *******************************************************************
// 命令和回应
/**
 * @brief 查询系统支持的总泵数目。
 * @return 总泵数目， Uint16。
 */
#define DSCP_CMD_PPI_GET_TOTAL_PUMPS                (DSCP_PPI_CBASE + 0x00)

/**
 * @brief 查询指定泵的校准系数。
 * @param index Uint8，要查询的泵索引，0号泵为光学定量泵。
 * @return 校准系数， Float32，每步泵出的体积，单位为 ml/度。
 * @see DSCP_CMD_PPI_SET_PUMP_FACTOR
 */
#define DSCP_CMD_PPI_GET_PUMP_FACTOR                (DSCP_PPI_CBASE + 0x01)

/**
 * @brief 设置指定泵的校准系数。
 * @details 因为蠕动泵的生产工艺及工作时长，每个泵转一步对应体积都不同，出厂或使用时需要校准。
 *  该参数将永久保存。
 * @param index Uint8，要设置的泵索引，0号泵为光学定量泵。
 * @param factor Float32，要设置的校准系数， 每步泵出的体积，单位为 ml/步。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_PPI_SET_PUMP_FACTOR                (DSCP_PPI_CBASE + 0x02)

/**
 * @brief 查询指定泵的运动参数。
 * @details 系统将根据设定的运动参数进行运动控制和规划。
 * @param index Uint8，要查询的泵索引，0号泵为光学定量泵。
 * @return 运动参数， 数据格式为：
 *  - acceleration Float32，加速度，单位为 ml/平方秒。
 *  - speed Float32，最大速度，单位为 ml/秒。
 * @see DSCP_CMD_PPI_SET_MOTION_PARAM
 */
#define DSCP_CMD_PPI_GET_MOTION_PARAM               (DSCP_PPI_CBASE + 0x03)

/**
 * @brief 设置指定泵的运动参数。
 * @details 系统将根据设定的运动参数进行运动控制和规划。运动参数将永久保存。
 * @param index Uint8，要设置的泵索引，0号泵为光学定量泵。
 * @param acceleration Float32，加速度，单位为 ml/平方秒。
 * @param speed Float32，最大速度，单位为 ml/秒。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 * @see DSCP_CMD_PPI_GET_MOTION_PARAM
 */
#define DSCP_CMD_PPI_SET_MOTION_PARAM               (DSCP_PPI_CBASE + 0x04)

/**
 * @brief 查询指定泵的工作状态。
 * @param index Uint8，要设置的泵索引，0号泵为光学定量泵。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_IDLE 空闲；
 *  - @ref DSCP_BUSY 忙碌，需要停止后才能做下一个动作；
 */
#define DSCP_CMD_PPI_GET_PUMP_STATUS                (DSCP_PPI_CBASE + 0x05)

/**
 * @brief 启动泵。
 * @details 启动后，不管成功与否，操作结果都将以事件的形式上传给上位机。关联的事件有：
 *   - @ref DSCP_EVENT_PPI_PUMP_RESULT
 * @param index Uint8，要操作的泵索引，0号泵为光学定量泵。
 * @param dir Uint8，泵转动方向，0为正向转动（抽取），1为反向转动（排空）。
 * @param volume Float32，泵取/排空体积，单位为 ml。
 * @param speed Float32，临时最大速度，单位为 ml/秒。速度为0时，使用 @ref DSCP_CMD_PPI_SET_MOTION_PARAM 设置的速度。
  * @param isExpect Bool，是否等事件，FALSE不等事件，TRUE等事件（默认等事件）。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败，如泵正在工作，无法启动泵，需要先停止；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 * @note 该命令将立即返回，泵转动完成将以事件的形式上报。
 */
#define DSCP_CMD_PPI_START_PUMP                     (DSCP_PPI_CBASE + 0x06)

/**
 * @brief 停止泵。
 * @param index Uint8，要操作的泵索引，0号泵为光学定量泵。
 * @param isExpect Bool，是否等事件，FALSE不等事件，TRUE等事件（默认等事件）。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
 */
#define DSCP_CMD_PPI_STOP_PUMP                      (DSCP_PPI_CBASE + 0x07)

/**
 * @brief 查询泵出的体积。
 * @details 启动泵到停止泵的过程中，泵的转动体积（步数）。
 * @param index Uint8，要查询的泵索引，0号泵为光学定量泵。
 * @return 泵出的体积，Float32，单位为 ml。
 * @see DSCP_CMD_PPI_START_PUMP
 */
#define DSCP_CMD_PPI_GET_PUMP_VOLUME                (DSCP_PPI_CBASE + 0x08)

/**
  * @brief 开关采水泵/排水泵
  * @param index Uint8， 0 = 采水泵，1 = 排水泵。
  * @param state Uint8， FALSE 0 = 停止，TRUE 1 = 启动。
  * @return 状态回应，Uint16，支持的状态有：
  *  - @ref DSCP_OK  操作成功；
  *  - @ref DSCP_ERROR 操作失败；
  *  - @ref DSCP_ERROR_PARAM 参数错误，传入的参数有问题
  */
#define DSCP_CMD_PPI_WATER_PUMP_SET_STATE               (DSCP_PPI_CBASE + 0x09)

 /**
  * @brief 查询采水泵/排水泵状态。
  * @param index Uint8， 0 = 采水泵，1 = 排水泵。
  * @return 状态回应，Uint16，支持的状态有：
  *  - @ref FALSE 0 = 空闲；
  *  - @ref TRUE 1 =  忙碌；
  */
#define DSCP_CMD_PPI_WATER_PUMP_GET_STATE                (DSCP_PPI_CBASE + 0x0A)

/**
 * @brief 检测点状态
 */
typedef enum
{
   NotCoverd   = 0,   //未遮挡
   Coverd   = 1,      //遮挡
   MaxCheckPointStatus =2,
}CheckPointStatus;

/**
 * @brief 泵至检测点。
 * @param[in] index Uint8，要操作的泵索引，0号泵为光学定量泵。
 * @param[in] dir Uint8，泵转动方向，0为正向转动（抽取），1为反向转动（排空）。
 * @param[in] destStatus CheckPointStatus, 目标检测点需要达到的状态。
 * @param[in] speed float，速度，单位为 ml/s。
 * @param[in] maxVolume float，最大限制体积，单位为 ml。
 * @return 返回状态，操作是否成功。
 * @note 该命令将立即返回，泵转动完成将以事件的形式上报。
 */
#define DSCP_CMD_PPI_START_PUMP_TO_POINT                      (DSCP_PPI_CBASE + 0x0B)

// *******************************************************************
// 事件

/**
 * @brief 泵操作结果。
 */
enum PumpResult
{
    PUMP_RESULT_FINISHED = 0,       ///< 泵操作正常完成。
    PUMP_RESULT_FAILED = 1,         ///< 泵操作中途出现故障，未能完成。
    PUMP_RESULT_STOPPED = 2         ///< 泵操作被停止。
};

/**
 * @brief 泵操作结果事件。(0号泵，上位机使用双流程控制，泵事件分开发)
 * @details 启动泵转动操作结束时将产生该事件。
 * @param index Uint8，产生事件的泵索引，0号泵为光学定量泵。
 * @param result Uint8，泵操作结果码（ @ref PumpResult ），定义如下：
 *  - @ref PUMP_RESULT_FINISHED  泵操作正常完成；
 *  - @ref PUMP_RESULT_FAILED  泵操作中途出现故障，未能完成。
 *  - @ref PUMP_RESULT_STOPPED  泵操作被停止。
 */
#define DSCP_EVENT_PPI_PUMP_0_RESULT                  (DSCP_PPI_EBASE + 0x00)

/**
 * @brief 泵操作结果事件。(1号泵，上位机使用双流程控制，泵事件分开发)
 * @details 启动泵转动操作结束时将产生该事件。
 * @param index Uint8，产生事件的泵索引，0号泵为光学定量泵。
 * @param result Uint8，泵操作结果码（ @ref PumpResult ），定义如下：
 *  - @ref PUMP_RESULT_FINISHED  泵操作正常完成；
 *  - @ref PUMP_RESULT_FAILED  泵操作中途出现故障，未能完成。
 *  - @ref PUMP_RESULT_STOPPED  泵操作被停止。
 */
#define DSCP_EVENT_PPI_PUMP_1_RESULT                  (DSCP_PPI_EBASE + 0x01)


// *******************************************************************
// 状态返回





#endif // DSCP_PERISTALTIC_PUMP_INTERFACE_H_

/** @} */
