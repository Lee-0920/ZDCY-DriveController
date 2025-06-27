/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief Lia 链路令牌管理接口定义。
 * @details 主要管理主从式共享链路的通信过程，把主从链路转化为对等链路。
 *  <p> 仅共享总线型的主机需要实现。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-21
 */

#ifndef DNCP_LAI_TOKEN_MANAGER_H_
#define DNCP_LAI_TOKEN_MANAGER_H_

#include "DNCP/Ll/LinkFrame.h"
#include "Lai.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 开启令牌管理服务程序。
 * @details 启动后，主机自动轮询从机。
 */
extern void LaiTokenManager_Start(void);


/**
 * @brief 停止令牌管理服务。
 * @details 停止后从机的信息不可能传给主机。
 */
extern void LaiTokenManager_Stop(void);


/**
 * @brief 添加优先节点。
 * @details 优先节点在令牌分发时，有优先权，单位时间内得到的令牌数会较多。
 *  <p> 本方法主要用于遂道节点的令牌优先管理。隧道连接有变动时应该告知令牌管理程序，
 *      以优先对待已经建立隧道的从机。
 * @param addr 待操作的优先节点的链路地址。
 * @see LaiTokenManager_RemovePriorNode
 */
extern void LaiTokenManager_AddPriorNode(LinkAddress addr);


/**
 * @brief 去除优先节点。
 * @details 去除优先节点后，该节点无总线优先权。
 * @param addr 待操作的优先节点的链路地址。
 * @see LaiTokenManager_AddPriorNode
 */
extern void LaiTokenManager_RemovePriorNode(LinkAddress addr);


#ifdef __cplusplus
}
#endif

#endif  // DNCP_LAI_TOKEN_MANAGER_H_

/** @} */
