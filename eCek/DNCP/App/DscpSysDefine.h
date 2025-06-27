/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief DSCP 系统定义。
 * @details 对DSCP协议规定的一些常数、码值进行定义，包括 DSCP 系统保留的命令、事件
 *  等码值定义。
 * @version 1.0.0
 * @author xiejinqiang
 * @date 2016-4-26
 */

#ifndef DNCP_APP_DSCP_SYS_DEFINE_H_
#define DNCP_APP_DSCP_SYS_DEFINE_H_

// *******************************************************************
// 系统命令、回应、事件、返回定义

//
// 码值空间分配
//
#define DSCP_SYSC_BASE                  0xFFF0                  ///< 系统命令基值
#define DSCP_SYSE_BASE                  0xFFF0                  ///< 系统事件基值


//
// 系统命令定义
//
/**
 * @brief 系统命令：回声测试。
 * @details 用于基本的通信测试，下发什么数据，回来的就是什么数据。
 * @param echoData 回声测试数据，长度任意（可为0），但不能超过DNCP规定的最大帧长度。
 * @return 与 @p echoData 一致。
 */
#define DSCP_SYSCMD_ECHO                (DSCP_SYSC_BASE + 0)

/**
 * @brief 系统命令：获取DSCP接口版本号。
 * @details DSCP接口版本号表征了应用命令的版本变更，一般情况下，接口命令、回应或状态
 *  有增或删或改时，DSCP应用会变更该版本号。
 * @return 版本号，Byte[4]：
 *  - @b major Uint8，主版本号；
 *  - @b minor Uint8，次版本号；
 *  - @b revision Uint8，修订版本号；
 *  - @b build Uint8，编译版本号；
 */
#define DSCP_SYSCMD_IFVER_GET           (DSCP_SYSC_BASE + 1)

/**
 * @brief 系统命令：获取缓存的事件。
 * @details DscpDevice 在发送事件前，可先把事件缓冲起来，上位机等不到事件时（事件有丢失），
    可以主动调用本命令，让下位机再发一次事件。此时上位机应该及时再次等待事件，并作解析。
 * @return 状态回应，Uint16，支持的状态有：
 *  - @ref DSCP_OK  操作成功，稍候将把缓冲的事件重发；
 *  - @ref DSCP_ERROR 操作失败，缓冲中无事件；
 *  - @ref DSCP_NOT_SUPPORTED 没有实现本命令，上下位机的版本兼容可能出现问题。
 * @note 本机制为可选机制，在需要更可靠的事件时可实现本机制。
 */
#define DSCP_SYSCMD_ACQUIRE_EVENT       (DSCP_SYSC_BASE + 2)
//
// 系统返回定义
//
#define DSCP_OK                         0       ///< 系统状态：正常，操作成功
#define DSCP_IDLE                       1       ///< 系统状态：空闲
#define DSCP_BUSY                       2       ///< 系统状态：忙碌
#define DSCP_ERROR                      -1      ///< 系统状态：出错
#define DSCP_ERROR_PARAM                -2      ///< 系统状态：参数错误，传入的参数有问题
#define DSCP_TIMEOUT                    -3      ///< 系统状态：超时
#define DSCP_NOT_SUPPORTED              -4      ///< 系统状态：该命令不被系统支持
#define DSCP_AUTHENTICATE_FAIL          -50     ///< 系统状态：认证失败



#endif // DNCP_APP_DSCP_SYS_DEFINE_H_

/** @} */
