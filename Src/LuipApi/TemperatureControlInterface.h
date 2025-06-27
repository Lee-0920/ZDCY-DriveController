/**
 * @page page_TemperatureControlInterface 温度控制接口
 *  温度控制接口提供了控制温度的相关操作。
 *
 *  具体命令见： @ref module_TemperatureControlInterface
 *
 * @section sec_TCI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.26)
 *
 */

/**
 * @addtogroup module_TemperatureControlInterface 温度控制接口
 * @{
 */

/**
 * @file
 * @brief 温度控制接口。
 * @details 定义了一序列温度控制相关的操作。
 * @version 1.0.0
 * @author xiejinqiang
 * @date 2016.4.26
 */

#ifndef DSCP_TEMPERATURE_CONTROL_INTERFACE_H_
#define DSCP_TEMPERATURE_CONTROL_INTERFACE_H_

#define DSCP_TCI_CBASE                  0x0000 + 0x0A00     ///< 命令基值
#define DSCP_TCI_EBASE                  0x8000 + 0x0A00     ///< 事件基值
#define DSCP_TCI_SBASE                  0x0000 + 0x0A00     ///< 状态基值


// *******************************************************************
// 命令和回应

/**
 * @brief 查询温度传感器的校准系数。
 * @param index Uint8，要查询的恒温器索引。
 * @return 负输入分压，Float32。
 * @return 参考电压 ，Float32。
 * @return 校准电压 Float32。
 * @see DSCP_CMD_TCI_SET_CALIBRATE_FACTOR
 */
#define DSCP_CMD_TCI_GET_CALIBRATE_FACTOR           (DSCP_TCI_CBASE + 0x00)

/**
 * @brief 设置温度传感器的校准系数。
 * @details 因为个体温度传感器有差异，出厂或使用时需要校准。该参数将永久保存。
 *   @param index Uint8，要设置的恒温器索引。
 *   @param negativeInput Float32，负输入分压 。
 *   @param vref Float32，参考电压。
 *   @param vcal Float32，校准电压。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_TCI_SET_CALIBRATE_FACTOR           (DSCP_TCI_CBASE + 0x01)

/**
 * @brief 查询当前恒温器温度。
 * @param index Uint8，要查询的恒温器索引。
 * @return 当前恒温室温度，Float32。
 */
#define DSCP_CMD_TCI_GET_TEMPERATURE                (DSCP_TCI_CBASE + 0x02)

/**
 * @brief 查询恒温控制参数。
 * @param index Uint8，要查询的恒温器索引。
 * @return 恒温控制参数，格式如下：
 *  - proportion Float32，PID的比例系数。
 *  - integration Float32，PID的积分系数。
 *  - differential Float32，PID的微分系数。
 * @see DSCP_CMD_TCI_SET_THERMOSTAT_PARAM
 */
#define DSCP_CMD_TCI_GET_THERMOSTAT_PARAM           (DSCP_TCI_CBASE + 0x03)

/**
 * @brief 设置恒温控制参数。
 * @details 该参数将永久保存。系统启动时同步到当前恒温控制参数上。
 * @param index Uint8，要设置的恒温器索引。
 * @param proportion Float32，PID的比例系数。
 * @param integration Float32，PID的积分系数。
 * @param differential Float32，PID的微分系数。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_TCI_SET_THERMOSTAT_PARAM           (DSCP_TCI_CBASE + 0x04)

/**
 * @brief 查询恒温器的工作状态。
 * @param index Uint8，要查询的恒温器索引。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_IDLE 空闲；
 *  - @ref DSCP_BUSY 忙碌，需要停止后才能做下一个动作；
 */
#define DSCP_CMD_TCI_GET_THERMOSTAT_STATUS          (DSCP_TCI_CBASE + 0x05)

/**
 * @brief 恒温模式。
 */
typedef enum
{
    THERMOSTAT_MODE_AUTO = 0,           ///< 自动模式，根据需要及硬件能力综合使用加热器和制冷器。
    THERMOSTAT_MODE_HEATER = 1,         ///< 纯加热模式，不使用制冷器。
    THERMOSTAT_MODE_REFRIGERATE = 2,    ///< 纯制冷模式，不使用加热器。
    THERMOSTAT_MODE_NATURAL = 3         ///< 自然模式，加热器和制冷器都不参与，靠环境传递热量，自然升温或冷却。
}ThermostatMode;

/**
 * @brief 开始恒温。
 * @details 恒温开始后，系统将根据设定的恒温控制参数进行自动温度控制，尝试达到指定温度。
 *  不管成功与否，操作结果都将以事件的形式上传给上位机，但恒温器将继续工作，
 *  直到接收到DSCP_CMD_TCI_STOP_THERMOSTAT才停止。关联的事件有：
 *   - @ref DSCP_EVENT_TCI_THERMOSTAT_RESULT
 * @param index Uint8，要操作的恒温器索引。
 * @param mode Uint8，恒温模式，支持的模式见： @ref ThermostatMode 。
 * @param targetTemp Float32，恒温目标温度。
 * @param toleranceTemp Float32，容差温度，与目标温度的差值在该参数范围内即认为到达目标温度。
 * @param timeout Float32，超时时间，单位为秒。超时仍未达到目标温度，也将返回结果事件。
 * @param isExpectEvent bool,是否等待事件。
 * @return 状态回应，Uint16，支持的状态有：
 *   - @ref DSCP_OK  操作成功；
 *   - @ref DSCP_BUSY 操作失败，如恒温已经工作，需要先停止；
 *   - @ref DSCP_ERROR_PARAM 恒温参数错误；
 *   - @ref DSCP_ERROR 温度传感器异常；
 * @note 该命令将立即返回，恒温完成将以事件的形式上报。
 */
#define DSCP_CMD_TCI_START_THERMOSTAT               (DSCP_TCI_CBASE + 0x06)

/**
 * @brief 停止恒温控制。
 * @details 停止后，加热器和冷却器将不工作。
 * @param index Uint8，要操作的恒温器索引。
 * @param isExpectEvent bool,是否等待事件。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 * @note 该命令将立即返回。
 */
#define DSCP_CMD_TCI_STOP_THERMOSTAT                (DSCP_TCI_CBASE + 0x07)

/**
 * @brief 设置温度上报周期。
 * @details 系统将根据设定的周期，定时向上发出温度上报事件。
 * @param period Float32，温度上报周期，单位为秒。0表示不需要上报，默认为0。
 * @see DSCP_EVENT_TCI_TEMPERATURE_NOTICE
 * @note 所设置的上报周期将在下一次启动时丢失，默认为0，不上报。
 */
#define DSCP_CMD_TCI_SET_TEMPERATURE_NOTIFY_PERIOD  (DSCP_TCI_CBASE + 0x08)

/**
 * @brief 设置机箱风扇。
 * @details 根据设定的占空比，调节风扇速率
 * @param level float ,风扇速率，对应高电平占空比。默认为0，机箱风扇关闭。
 * @see
 * @note
 */
#define DSCP_CMD_TCI_TURN_BOXFAN                    (DSCP_TCI_CBASE + 0x09)

/** * @brief 设置恒温器风扇。
 * @details 根据设定的占空比，调节风扇速率
 * @param thermostatIndex Uint8，要设置的恒温器索引。
   @param fanIndex Uint8，要设置的恒温器中的风扇索引。
 * @param level float ,风扇速率，对应高电平占空比。默认为0，恒温器风扇关闭。
 * @see
 * @note
 */
#define DSCP_CMD_TCI_TURN_THERMOSTAT_FAN             (DSCP_TCI_CBASE + 0x0A)

/**
 * @brief 获取恒温器加热丝输出的最大占空比。
 * @param index Uint8，要获取的恒温器索引。
 * @param maxDutyCycle float ,加热丝输出的最大占空比
 * @see DSCP_CMD_TCI_SET_HEATER_MAX_DUTY_CYCLE
 * @note
 */
#define DSCP_CMD_TCI_GET_HEATER_MAX_DUTY_CYCLE      (DSCP_TCI_CBASE + 0x0B)

/**
 * @brief 设置恒温器加热丝输出的最大占空比。
 * @param index Uint8，要设置的恒温器索引。
 * @param maxDutyCycle float，加热丝输出的最大占空比
 * @see DSCP_CMD_TCI_GET_HEATER_MAX_DUTY_CYCLE
 */
#define DSCP_CMD_TCI_SET_HEATER_MAX_DUTY_CYCLE      (DSCP_TCI_CBASE + 0x0C)

/**
 * @brief 查询恒温器当前恒温控制参数。
 * @param index Uint8，要查询的恒温器索引。
 * @return 恒温控制参数，格式如下：
 *  - proportion Float32，PID的比例系数。
 *  - integration Float32，PID的积分系数。
 *  - differential Float32，PID的微分系数。
 * @see DSCP_CMD_TCI_SET_CURRENT_THERMOSTAT_PARAM
 */
#define DSCP_CMD_TCI_GET_CURRENT_THERMOSTAT_PARAM   (DSCP_TCI_CBASE + 0x0D)

/**
 * @brief 设置当前恒温控制参数。
 * @details 恒温系统将根据设置的参数进行温度调节。此参数上电时获取FLASH的PID。
 * @param index Uint8，要设置的恒温器索引。
 * @param proportion Float32，PID的比例系数。
 * @param integration Float32，PID的积分系数。
 * @param differential Float32，PID的微分系数。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_TCI_SET_CURRENT_THERMOSTAT_PARAM   (DSCP_TCI_CBASE + 0x0E)

/**
 * @brief 查询系统支持的恒温器数目。
 * @return 总数目， Uint16。
 */
#define DSCP_CMD_TCI_GET_TOTAL_THERMOSTAT               (DSCP_TCI_CBASE + 0x0F)

/**
 * @brief 查询当前环境温度。
 * @return 当前恒温室温度，Float32。
 */
#define DSCP_CMD_TCI_GET_ENV_TEMPERATURE                (DSCP_TCI_CBASE + 0x10)

/** * @brief 设置独立风扇。
 * @details 根据设定的占空比，调节风扇速率
   @param fanIndex Uint8，要设置风扇索引。
 * @param level float ,风扇速率，对应高电平占空比。默认为0，风扇关闭。
 * @see
 * @note
 */
#define DSCP_CMD_TCI_TURN_FAN                           (DSCP_TCI_CBASE + 0x11)

/** * @brief 温控器初始化
 * @details 温控器各功能模块初始化
 * @see
 * @note
 */
#define DSCP_CMD_TCI_INIT                           (DSCP_TCI_CBASE + 0x12)

/**
 * @brief PT1000温度自动校准
 * @details 输入目标传感器的真实温度，下位机自动校准出参数
 * @param Uint8 index , PT1000温度传感器索引。
 * @param float temp , 真实温度。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_TCI_TEMP_CALIBRATE                  (DSCP_TCI_CBASE + 0x13)
// *******************************************************************
// 事件

/**
 * @brief 恒温操作结果。
 */
typedef enum
{
    THERMOSTAT_RESULT_REACHED = 0,          ///< 恒温目标达成，目标温度在规定时间内达成，后续将继续保持恒温，直到用户停止。
    THERMOSTAT_RESULT_FAILED = 1,           ///< 温度传感器硬件异常。
    THERMOSTAT_RESULT_STOPPED = 2,          ///< 恒温被停止。
    THERMOSTAT_RESULT_TIMEOUT = 3           ///< 恒温超时，指定时间内仍未达到目标温度。
}ThermostatResult;

/**
 * @brief 恒温结果事件。
 * @details 恒温操作结束时将产生该事件。
 * @param[in] timeout 恒温操作事件等待超时时间。
 * @returen 恒温结果（ @ref Thermostat ），定义如下：
 *  - @ref index Uint8，产生事件的恒温器索引。
 *  - @ref result Uint8，恒温结果码（ @ref ThermostatResult ）。
 *  - @ref temp float，当前温度。
 */

#define DSCP_EVENT_TCI_THERMOSTAT_RESULT                (DSCP_TCI_EBASE + 0x00)

/**
 * @brief 温度定时上报事件。
 * @details 系统将根据设置的上报周期，定时向上汇报温度。
 *  上报周期可通过命令 @ref DSCP_CMD_TCI_SET_TEMPERATURE_NOTIFY_PERIOD 设定。
 * @reurn environmentTemp Float32，环境温度，单位为摄氏度。
 * @reurn number Uint8，恒温室个数，决定了后面的变长参数长度。
 * @reurn thermostatTemp1 Float32，恒温室温度，单位为摄氏度。
 * @reurn thermostatTemp2 Float32，恒温室温度，单位为摄氏度。
 *
 */
#define DSCP_EVENT_TCI_TEMPERATURE_NOTICE               (DSCP_TCI_EBASE + 0x01)


// *******************************************************************
// 状态返回



#endif // DSCP_TEMPERATURE_CONTROL_INTERFACE_H_

/** @} */
