/**
 * @page page_OpticalAcquireInterface 光学信号采集接口
 *  光学信号采集接口提供了光学信号采集的的相关操作。
 *
 *  具体命令见： @ref module_OpticalAcquireInterface
 *
 * @section sec_OAI_ChangeLog 版本变更历史
 *  接口历史变更记录：
 *  - 1.0.0 基本版本 (2016.4.28)
 *
 */

/**
 * @addtogroup module_OpticalAcquireInterface 光学信号采集接口
 * @{
 */

/**
 * @file
 * @brief 光学信号采集接口(LF82专用)。
 * @details 定义了一序列光学信号采集相关的操作。
 * @version 1.0.0
 * @author yongxi
 * @date 2018.01.17
 */

#ifndef DSCP_OPTICAL_ACQUIRE_INTERFACE_H_
#define DSCP_OPTICAL_ACQUIRE_INTERFACE_H_

#define DSCP_OAI_CBASE                  0x0000 + 0x0C00     ///< 命令基值
#define DSCP_OAI_EBASE                  0x8000 + 0x0C00     ///< 事件基值
#define DSCP_OAI_SBASE                  0x0000 + 0x0C00     ///< 状态基值


// *******************************************************************
// 命令和回应
/**
 * @brief 设置信号AD上报周期。
 * @details 系统将根据设定的周期，定时向上发出信号AD上报事件。
 * @param period Float32，信号AD上报周期，单位为秒。0表示不需要上报，默认为0。
 * @see DSCP_EVENT_OAI_SIGNAL_AD_NOTICE
 * @note 所设置的上报周期将在下一次启动时丢失，默认为0，不上报。而且通信失败时不上报。
 */
#define DSCP_CMD_OAI_SET_SIGNAL_AD_NOTIFY_PERIOD    (DSCP_OAI_CBASE + 0x00)

/**
 * @brief 启动采集过程。
 * @details 采集到的信号数据将以事件 @ref DSCP_EVENT_OAI_AD_ACQUIRED
 *  的形式上传给上位机。
 * @param acquireTime Float32，采样时间，单位为秒，0表示只采1个样本。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_START_ACQUIRER                 (DSCP_OAI_CBASE + 0x01)

/**
 * @brief 停止采集过程。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_STOP_ACQUIRER                  (DSCP_OAI_CBASE + 0x02)

/**
 * @brief 设置运放增益
 * @param gainCoefficient Uint8，运放增益,0-3
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_SET_GAIN                       (DSCP_OAI_CBASE + 0x03)

/**
 * @brief 获取运放增益。
 * @return gain Uint8，运放增益,0-3
 */
#define DSCP_CMD_OAI_GET_GAIN                       (DSCP_OAI_CBASE + 0x04)

/**
 * @brief 开启PD板电源
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_POWER_ON                       (DSCP_OAI_CBASE + 0x05)

/**
 * @brief 关闭PD板电源
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_POWER_OFF                      (DSCP_OAI_CBASE + 0x06)

/**
 * @brief 启动静态AD调节至目标值操作。
 * @details 开启PMT电源，调整电位器输出值，优化目标AD。
 * @param index Uint8，目标PMT通道索引号[pmt_ref=0,pmt_mea=1]。
 * @param targetAD Uint32，目标AD。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_START_STATIC_AD_CONTROL                  (DSCP_OAI_CBASE + 0x07)

/**
 * @brief 停止静态AD调节并关闭PMT电源。
 * @details 停止静态AD调节过程
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_STOP_STATIC_AD_CONTROL                    (DSCP_OAI_CBASE + 0x08)

/**
 * @brief 查询默认静态AD控制参数。
 * @return value Uint16 ，静态AD调节参数。
 * @see DSCP_CMD_OAI_SET_STATIC_AD_CONTROL_PARAM
 */
#define DSCP_CMD_OAI_GET_STATIC_AD_CONTROL_PARAM              (DSCP_OAI_CBASE + 0x09)

/**
 * @brief 设置默认静态AD控制参数。
 * @details 静态AD控制器将根据设置的参数设置器件，并将参数永久保存在FLASH。
 * @details 静态AD调节状态中禁用。
 * @param index Uint8，目标PMT通道索引号。
 * @param value Uint16 ，静态AD调节参数。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功；
 *  - @ref DSCP_ERROR 操作失败；
 */
#define DSCP_CMD_OAI_SET_STATIC_AD_CONTROL_PARAM              (DSCP_OAI_CBASE + 0x0A)

/**
 * @brief 查询静态AD控制功能是否有效
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  功能有效，操作成功；
 *  - @ref DSCP_ERROR 功能无效，操作失败；
 */
#define DSCP_CMD_OAI_IS_STATIC_AD_CONTROL_VALID              (DSCP_OAI_CBASE + 0x0B)
// *******************************************************************
// 事件

/**
 * @brief 采集结果。
 */
typedef enum
{
    ACQUIRED_RESULT_FINISHED = 0,     ///< 采集正常完成。
    ACQUIRED_RESULT_FAILED = 1,       ///< 采集中途出现故障，未能完成。
    ACQUIRED_RESULT_STOPPED = 2,      ///< 采集被停止。
}AcquiredResult;

/**
 * @brief 信号AD采集完成事件。
 * @details 启动采集后，采完数据时将产生该事件。
 * @param reference Uint32，参考端AD。
 * @param measure Uint32，测量端AD。
 * @param result Uint8，采集结果码（ @ref AcquiredResult ），定义如下：
 *  - @ref ACQUIRED_RESULT_FINISHED  采集正常完成；
 *  - @ref ACQUIRED_RESULT_FAILED  采集中途出现故障，未能完成。
 *  - @ref ACQUIRED_RESULT_STOPPED  采集被停止。
 * @see DSCP_CMD_OAI_START_ACQUIRER
 */
#define DSCP_EVENT_OAI_AD_ACQUIRED                  (DSCP_OAI_EBASE + 0x00)

/**
 * @brief 信号AD定时上报事件。
 * @details 系统将根据设置的上报周期，定时向上汇报信号AD。
 *  上报周期可通过命令 @ref DSCP_CMD_TCI_SET_TEMPERATURE_NOTIFY_PERIOD 设定。
 * @param reference Uint32，参考端AD。
 * @param measure Uint32，测量端AD。
 */
#define DSCP_EVENT_OAI_SIGNAL_AD_NOTICE               (DSCP_OAI_EBASE + 0x01)

/**
 * @brief PMT静态AD调节控制结果。
 */
typedef enum
{
    STATIC_AD_CONTROL_RESULT_UNFINISHED= 0,       ///<静态AD调节未完成。
    STATIC_AD_CONTROL_RESULT_FINISHED = 1,     ///<  静态AD调节完成。
}StaticADControlResult;

/**
 * @brief PMT静态AD调节结果事件。
 * @details PMT调节AD至目标AD最优取值时产生该事件。
 * @see DSCP_CMD_OAI_START_STATIC_AD_CONTROL
 */
#define DSCP_EVENT_OAI_STATIC_AD_CONTROL_RESULT                  (DSCP_OAI_EBASE + 0x02)

// *******************************************************************
// 状态返回




#endif // DSCP_OPTICAL_ACQUIRE_INTERFACE_H_

/** @} */
