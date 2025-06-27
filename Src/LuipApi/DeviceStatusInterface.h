/**
 * @page page_DeviceStatusInterface 设备状态接口
 *  设备状态接口提供了设备实时状态的查询操作。
 *
 *  具体命令见： @ref module_DeviceStatusInterface
 *
 * @section sec_DSI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.26)
 *      - 设备闪灯指示；
 *      - 设备运行模式；
 *
 */

/**
 * @addtogroup module_DeviceStatusInterface 设备状态接口
 * @{
 */

/**
 * @file
 * @brief 设备状态接口。
 * @details 定义了一序列查询设备状态的操作。
 * @version 1.0.0
 * @author xiejinqiang
 * @date 2016-4-26
 */

#ifndef DSCP_DAVICE_STATUS_INTERFACE_H_
#define DSCP_DAVICE_STATUS_INTERFACE_H_

#define DSCP_DSI_CBASE                  0x0000 + 0x0300     ///< 命令基值
#define DSCP_DSI_EBASE                  0x8000 + 0x0300     ///< 事件基值
#define DSCP_DSI_SBASE                  0x0000 + 0x0300     ///< 状态基值


// *******************************************************************
// 命令和回应


/**
 * @brief 设备运行模式。
 */
typedef enum
{
    DEVICE_MODE_APPLICATION = 0,    ///< 应用程序模式，正常的工作模式。
    DEVICE_MODE_UPDATER = 1,        ///< App 升级模式，该模式将对应用程序进行更新。
    DEVICE_MODE_UPGRADER = 2,		///< Updater 升级模式，该模式将对 Updater 本身进行升级。
	DEVICE_MODE_UNKNOWN = -1		///< 未知模式。
}DeviceRunMode;

/**
 * @brief 查询程序当前的运行模式。
 * @details 本操作通常用于升级管理程序，以确保设备处于期望的运行模式。
 * @return 运行模式，Uint8。请参考 @ref DeviceRunMode 。
 * @note App 模式和 Updater 模式都支持本命令。见： @ref DSCP_CMD_DUI_GET_RUN_MODE ，
 *  注意这两条命令的码值是一致，只是名称不同而已。
 */
#define DSCP_CMD_DSI_GET_RUN_MODE                   (DSCP_DSI_CBASE + 0x00)

/**
 * @brief 闪烁设备指示灯。
 * @details 本命令只是抽象定义了设备的指示操作，而未指定是哪一盏灯，
 *  具体的指示灯（一盏或多盏）由设备实现程序决定。
 *  <p>闪烁方式由参数指定。通过调节参数，可设置LED灯为常亮或常灭：
 *  - onTime 为 0 表示灭灯
 *  - offTime 为 0 表示亮灯
 *  - onTime 和 offTime 都为0 时，不动作
 *  <p> 当持续时间结束之后，灯的状态将返回系统状态，受系统控制。
 * @param duration Uint32，持续时间，单位为毫秒。0 表示不起作用，-1 表示一直持续。
 * @param onTime Uint16，亮灯的时间，单位为毫秒。
 * @param offTime Uint16，灭灯的时间，单位为毫秒。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *  - @ref DSCP_NOT_SUPPORTED 操作不被支持；
 *  
 */
#define DSCP_CMD_DSI_BLINK_DEVICE                   (DSCP_DSI_CBASE + 0x01)


/**
 * @brief 设备电源供给类型。
 */
enum DevicePowerSupplyType
{
    DEVICE_POWER_TYPE_UNKNOWN = 0,         ///< 未知类型，保留使用。
    DEVICE_POWER_TYPE_ONGOING_AC = 1,      ///< 持续交流供电。
    DEVICE_POWER_TYPE_BUS = 2,             ///< 总线供电。
    DEVICE_POWER_TYPE_TERMINAL = 3,        ///< 外部终端供电。
    DEVICE_POWER_TYPE_BATTERY = 4,         ///< 电池供电。
    DEVICE_POWER_TYPE_CHARGER = 5          ///< 充电器供电。
};

/**
 * @brief 查询设备的电源供应类型。
 * @details 这里的电源供给类型是指设备实际的电源类型。
 * @return 电源供应类型，Uint8。请参考 @ref DevicePowerSupplyType 。
 */
#define DSCP_CMD_DSI_GET_POWER_SUPPLY_TYPE          (DSCP_DSI_CBASE + 0x02)

/**
 * @brief 设备初始化。
 * @details 恢复业务功能到初始状态。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 *
 */
#define DSCP_CMD_DSI_INITIALIZE                      (DSCP_DSI_CBASE + 0x03)

/**
 * @brief 出厂恢复设置。
 * @details 恢复出厂数据。
 *
 */
#define DSCP_CMD_DSI_FACTORY_DEFAULT                 (DSCP_DSI_CBASE + 0x04)

/**
 * @brief 设备单片机复位。
 * @details 恢复单片机的初始状态。
 *
 */
#define DSCP_CMD_DSI_RESET                           (DSCP_DSI_CBASE + 0x05)

// *******************************************************************
// 事件


/**
 * @brief 设备复位原因。
 */
typedef enum
{
    DEVICE_RESET_POWER_ON = 0,          ///< 正常上电启动。
    DEVICE_RESET_PLANNED = 1,           ///< 计划内复位，通常是预期的复位操作。
    DEVICE_RESET_UNEXPECTED = 32,       ///< 意外的复位（未知原因），如程序跑飞等
    DEVICE_RESET_EXCEPTION = 33,        ///< 异常中断事件引起的复位。
    DEVICE_RESET_LOW_VOLTAGE = 34       ///< 低电压复位，因为供电不稳定造成复位
}DeviceResetCause;

/**
 * @brief 设备复位事件。
 * @details 设备复位时产生该事件。设备复位后，整个系统的状态都将恢复到初始状态。
 *  如果上位机改变过设备的某些状态，并且依赖于这个状态才能正常工作，上位机应该关注
 *  该事件。正常上电和异常复位都将引起设备复位，产生该事件，具体原因由事件参数给出。
 * @param cause Uint16，复位原因。请参考 @ref DeviceResetCause 定义的复位代码。
 */
#define DSCP_EVENT_DSI_RESET                        (DSCP_DSI_EBASE + 0x01)


// *******************************************************************



#endif // DSCP_DAVICE_STATUS_INTERFACE_H_

/** @} */
