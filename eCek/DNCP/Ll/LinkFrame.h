/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 逻辑链路层帧格式定义和实现。
 * @details 提供逻辑链路层帧格式相关资源的统一定义。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-12-21
 */

#ifndef DNCP_LL_LINK_FRAME_H_
#define DNCP_LL_LINK_FRAME_H_

#include "Common/Types.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义



//*******************************************************************
// 帧格式定义

/**
 * @brief 链路地址类型。
 */
typedef Uint8 LinkAddress;

typedef union FrameCtrlWordUnion
{
    // 链路控制字：裸数据。
    Uint8 data;

    // 链路控制字：信息帧。
    struct  
    {
        unsigned char flag  : 1;        ///< 管理帧标志位，值为0
        unsigned char uap   : 2;        ///< 上层访问点，表征数据包所属的上层协议的类型。
        unsigned char seq   : 2;        ///< 发送帧序号
        unsigned char pf    : 1;        ///< 查询结束标志位
        unsigned char next  : 2;        ///< 稍带确认帧序号，暂时不用
    } info;

    // 链路控制字：管理帧。
    struct  
    {
        unsigned char flag  : 1;        ///< 管理帧标志位，值为1
        unsigned char type  : 2;        ///< 管理帧类型
        unsigned char reserved  : 2;    ///< 保留
        unsigned char pf    : 1;        ///< 查询结束标志位
        unsigned char next  : 2;        ///< 确认帧序号，期望的下一帧序号
    } man;
}
FrameCtrlWord;


// 管理帧类型定义
#define FRAME_TYPE_NULL         0   ///< 空帧，通常与pf域一起使用，作为Lai层的轮询令牌帧
#define FRAME_TYPE_RR           1   ///< Receive Ready 接收就绪，确认帧，期望下一帧
#define FRAME_TYPE_REJ          2   ///< Reject 拒绝，否定确认帧，期望重传
#define FRAME_TYPE_RNR          3   ///< Receive Not Ready 接收暂未就绪

/**
 * @brief 链路层校验和类型。
 */
typedef Uint16 LinkCheckSum;


#ifdef __cplusplus
}
#endif

#endif  // DNCP_LL_LINK_FRAME_H_

/** @} */
