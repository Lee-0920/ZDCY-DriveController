/**
 * @page page_SolenoidValveInterface 电磁阀控制接口
 *  电磁阀控制接口提供了控制电磁阀开闭的相关操作。
 *
 *  具体命令见： @ref module_SolenoidValveInterface
 *
 * @section sec_SVI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.26)
 *
 */

/**
 * @addtogroup module_SolenoidValveInterface 电磁阀控制接口
 * @{
 */

/**
 * @file
 * @brief 电磁阀控制接口。
 * @details 定义了一序列查询控制电磁阀的操作。
 * @version 1.0.0
 * @author xingfan
 * @date 2016-05-27
 */
#ifndef SRC_DNCPSTACK_SOLENOIDVALVEINTERFACE_H_
#define SRC_DNCPSTACK_SOLENOIDVALVEINTERFACE_H_

#define DSCP_SVI_CBASE                  0x0000 + 0x0500     ///< 命令基值
#define DSCP_SVI_EBASE                  0x8000 + 0x0500     ///< 事件基值
#define DSCP_SVI_SBASE                  0x0000 + 0x0500     ///< 状态基值


// *******************************************************************
// 命令和回应

/**
 * @brief 查询系统支持的总电磁阀数目。
 * @return 总电磁阀数目， Uint16。
 */
#define DSCP_CMD_SVI_GET_TOTAL_VALVES   (DSCP_SVI_CBASE + 0x00)

/**
 * @brief 查询当前开启的阀门映射图。
 * @return 通道映射图，Uint32，每位表示一个通道的开关状态，1为打开，0为闭合，低位开始。
 */
#define DSCP_CMD_SVI_GET_VALVE_MAP                  (DSCP_SVI_CBASE + 0x01)

/**
 * @brief 设置阀门映射图。
 * @param map Uint32，通道映射图，每位表示一个通道的开关状态，1为打开，0为闭合，低位开始。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_SVI_SET_VALVE_MAP                  (DSCP_SVI_CBASE + 0x02)

/**
 * @brief 独立打开阀门映射图中的阀。
 * @param map Uint32，通道映射图，每位表示一个通道的开关状态，1为打开，0为不操作，低位开始。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_SVI_TURN_ON_VALVE_MAP              (DSCP_SVI_CBASE + 0x03)


/**
 * @brief  独立关闭阀门映射图中的阀。
 * @param map Uint32，通道映射图，每位表示一个通道的开关状态，1为关闭，0为不操作，低位开始。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_SVI_TURN_OFF_VALVE_MAP             (DSCP_SVI_CBASE + 0x04)



// *******************************************************************
// 事件


// *******************************************************************
// 状态返回




#endif // DSCP_SOLENOID_VALVE_INTERFACE_H_

/** @} */
