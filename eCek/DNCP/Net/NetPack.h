/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 网络包格式定义。
 * @details 提供网络层数据包格式的统一定义。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-2-1
 */

#ifndef DNCP_NET_NET_PACK_H_
#define DNCP_NET_NET_PACK_H_

#include "Common/Types.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义



//*******************************************************************
// 包格式定义

/**
 * @brief 网络地址类型。
 * @details 唯一标识垂直树型网络上的一个节点。
 *  <p> 网络地址的格式如下（由低到高排序）：
 *      - 8位：上行地址（包括层级域、广播域、个体域）
 *      - 4位：下行第1级（平板、PC）地址
 *      - 4位：下行第2级（主控）地址
 *      - 8位：下行第3级（单控）地址
 *      - 8位：下行第4级（熔配）地址
 *      在本网络结构体系中，节点的编址是不对等的，真正有效的编址是下行地址，
 *      而上行地址是相对的层级地址，不能唯一标识网络中的节点，仅在具体节点
 *      所有的网络路径中有效。
 *      上行地址主要用于数据包的向上寻址，下行地址主要用于数据包的向下寻址。
 *  <p> 上行地址各域段解释：
 *      - 4位：层级标识域，表征节点所处的网络层次，如平板处于最高层（1层）
 *      - 4位：个体标识域，表征节点处于当前层级中的位置，属于层级标识的子编号
 *      目前上行层级地址的有效取值为（0为无效值，仅表示不是上行地址）：
 *      - 1：表示实际上的顶级控制层，如平板、PC等；
 *      - 2：表示主控层；
 *      - 3：表示单控层；
 *      - 4：表示熔配层；
 *      上行个体标识通常为0，表示未使用。当某节点有多个直接上级时，用于区分
 *      同一层级的多个节点。比如在第1层级，有平板和中控ODMP同时连接到中控的
 *      主控逻辑，对主控来说，需要区别它上面的是那一个角色，所以需要额外编址。
 *      个体标识的有效取值只能是 1、2、4、8 中的一个。路由时，数据包的个体标识
 *      域可以多播或广播（0xF）。DSCP 的事件可以使用该机制进行广播。
 *  
 *  <p> 在下行节点地址中，如果某一级的地址为0，表示应该路由到上一层节点本身。
 *      如果某一级的位值为全1，表示广播地址，应广播到该层的每一个结点。
 *      全 0 无意义，表示该地址仅仅是一个上行地址。
 *  <p> 举例：
 *      - 上行(1.x): 向上路由到PC或平板。
 *      - 上行(1.F): 向上路由到所有控制器（PC和平板）。
 *      - 上行(3.x): 向上路由到单控。
 *      - 下行(1, 1, 4, 0): 向下路由到第1个主控下的第4个单控。
 *      - 下行(1, 1, 0, 0): 向下路由到第1个主控。
 *      - 下行(1, 1, 3, 255): 向下路由到第1个主控第3个单控下的所有熔配。
 *  
 */
typedef Uint32 NetAddress;

// 获取网络地址中的相应域段
#define NET_ADDRESS_UPLINK_GET(addr)        ((addr) & 0x000000FF)
#define NET_ADDRESS_DOWNLINK_GET(addr)      ((addr) & 0xFFFFFF00)
#define NET_ADDRESS_DOWNLINK_GET_D1(addr)   ((Uint8)(((addr) & 0x00000F00) >> 8))
#define NET_ADDRESS_DOWNLINK_GET_D2(addr)   ((Uint8)(((addr) & 0x0000F000) >> 12))
#define NET_ADDRESS_DOWNLINK_GET_D3(addr)   ((Uint8)(((addr) & 0x00FF0000) >> 16))
#define NET_ADDRESS_DOWNLINK_GET_D4(addr)   ((Uint8)(((addr) & 0xFF000000) >> 24))

/**
 * @brief 判断目标地址是否为上行地址。
 * @param addr 要查询的路由地址。
 */
#define NET_ADDRESS_IS_UPLINK(addr)     ((addr) & 0x000000FF)

/**
 * @brief 生成一个上行层级地址，下行域无效。
 * @param la 上行层级地址。
 */
#define NET_ADDRESS_UPLINK_MAKE(la)     (la)

/**
 * @brief 生成一个下行地址，上行域无效。
 * @param d1 下行第1层地址，范围为 0 - 15。
 * @param d2 下行第2层地址，范围为 0 - 15。
 * @param d3 下行第3层地址，范围为 0 - 255。
 * @param d4 下行第4层地址，范围为 0 - 255。
 */
#define NET_ADDRESS_DOWNLINK_MAKE(d1, d2, d3, d4)       \
    (((d4)<<24)|((d3)<<16)|((d2)<<12)|((d1)<<8))

/**
 * @brief 生成一个路由地址，同时指定上行和下行。
 * @param la 上行地址，范围为 0 - 255。
 * @param d1 下行第1层地址，范围为 0 - 15。
 * @param d2 下行第2层地址，范围为 0 - 15。
 * @param d3 下行第3层地址，范围为 0 - 255。
 * @param d4 下行第4层地址，范围为 0 - 255。
 */
#define NET_ADDRESS_MAKE(la, d1, d2, d3, d4)            \
    (((d4)<<24)|((d3)<<16)|((d2)<<12)|((d1)<<8)|(la))

/**
 * @brief 生成一个上行路由地址，下行域被设置为无效。
 * @param layer 层级标识，有效取值为 0 - 15 。
 * @param obj 个体标识，有效取值为 0 - 15 。
 */
#define UPLINK_ADDR_MAKE(layer, obj)                    \
    ((((obj)<<4)|(layer)) & 0x000000FF)

/**
 * @brief 生成一个上行路由掩码，下行域被设置为无效。
 * @param layer 层级标识，有效取值为 0 - 15 。
 * @param obj 个体标识，有效取值为 0 - 15 。
 */
#define UPLINK_MASK_MAKE(layer, obj)                    \
    ((((obj)<<4)|(layer)) | 0xFFFFFF00)

/**
 * @brief 获取下行地址段中的层级标识。
 */
#define UPLINK_ADDR_GET_LAYER(addr)                    \
    ((addr) & 0x0000000F)

/**
 * @brief 获取下行地址段中的个体标识。
 */
#define UPLINK_ADDR_GET_OBJ(addr)                       \
    ((addr) & 0x000000F0)

/**
 * @brief 生成一个下行路由地址，上行域被设置为无效。
 * @param d1 下行第1层地址，范围为 0 - 15。
 * @param d2 下行第2层地址，范围为 0 - 15。
 * @param d3 下行第3层地址，范围为 0 - 255。
 * @param d4 下行第4层地址，范围为 0 - 255。
 */
#define DOWNLINK_ADDR_MAKE(d1, d2, d3, d4)              \
    (((d4)<<24)|((d3)<<16)|((d2)<<12)|((d1)<<8))

/**
 * @brief 生成一个下行路由掩码，上行域被设置为无效。
 * @param d1 下行第1层地址，范围为 0 - 15。
 * @param d2 下行第2层地址，范围为 0 - 15。
 * @param d3 下行第3层地址，范围为 0 - 255。
 * @param d4 下行第4层地址，范围为 0 - 255。
 */
#define DOWNLINK_MASK_MAKE(d1, d2, d3, d4)              \
    (((d4)<<24)|((d3)<<16)|((d2)<<12)|((d1)<<8)|0x000000FF)


extern Uint8 NetAddress_GetDownlinkField(NetAddress addr, Uint8 layer);
extern void NetAddress_SetDownlinkField(NetAddress* addr, Uint8 layer, Uint8 layerAddr);

#ifdef __cplusplus
}
#endif

#endif  // DNCP_NET_NET_PACK_H_

/** @} */
