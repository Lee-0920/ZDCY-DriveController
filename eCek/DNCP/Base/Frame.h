/**
 * @addtogroup module_DNCP
 * @{
 */

/**
 * @file
 * @brief 链路层帧对象，通常作为各层交换数据的载体而使用。
 * @details 提供链路层数据帧的基本操作接口，如帧分配、释放等。
 *  <p>该对象的设计是以链路层作为切入点，但其功用不限于链路层，在 LL
 *  层、网络层……一直到应用层，都使用该对象作为业务封包的载体（容器）。
 *  即是说，该对象与LL层及以上各层都有关联，耦合性很强，违背了设计原则。
 *  这样设计的目的是提高运行效率，减少内存碎片，这在MCU环境是很有必要的。
 *  <p>使用导引：
 *    - 发送流程：在应用层（顶层）创建一个帧对象，以容量具体的业务数据，
 *  然后丢给下层处理，这样一层一层往下传，最终在 LL 层向 Lai 层提交数据
 *  后，由 LL 层协议实体销毁这个作为载体的帧对象；
 *    - 接收流程：当 LL 层收到 Lai 实体提交的数据后，LL 实体动态创建
 *  一个帧对象，作为载体保存Lai接收的数据，然后提交给上层，一层层往上提交，
 *  最终到达应用层（顶层）。应用层用完后，必须把该帧载体对象销毁。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-4
 */

#ifndef DNCP_BASE_FRAME_POOL_H_
#define DNCP_BASE_FRAME_POOL_H_

#include "Common/Types.h"

#ifdef __cplusplus
extern "C" {
#endif


//*******************************************************************
// 基本常数定义

#define FRAME_MAX_SIZE              255     ///< 最大帧长度


//*******************************************************************
// 帧数据结构定义

/**
 * @brief 
 * @details 
 */
typedef struct FrameStruct
{
    Uint8 len;                      ///< 帧长度，单位为字节
    Uint8 packOffset;               ///< 封包数据偏移，单位为字节
    Int8 refCount;                  ///< 引用计数，为0时自动销毁
    Uint8 destAddr;                 ///< 帧目的地址，仅供网络层发送逻辑使用
                                    // 此后为帧的裸数据
}
Frame;


//*******************************************************************
// 帧数据操作

// 帧管理
Frame* Frame_New(Uint8 len);
Frame* Frame_CopyNew(Frame* original);
void Frame_AddRef(Frame* self);
void Frame_Delete(Frame* self);

// 帧使用
void Frame_CopyFrom(Frame* self, Byte* data, Uint16 bytes);
void Frame_CopyTo(Frame* self, Byte* data, Uint16 bytes);
Byte* Frame_GetData(Frame* self);

// 应用封包版本
Frame* Frame_NewPack(Uint8 packOffset, Uint8 packSize);
Byte* Frame_GetPackData(Frame* self);
Uint8 Frame_GetPackSize(Frame* self);

#ifdef __cplusplus
}
#endif

#endif  // DNCP_BASE_FRAME_POOL_H_

/** @} */
