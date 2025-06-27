/**
 * @addtogroup module_DNCP
 * @{
 */


/**
 * @file
 * @brief 链路层帧对象，通常作为各层交换数据的载体而使用。
 * @details 提供链路层数据帧的基本操作接口，如帧分配、释放等。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2013-1-4
 */

#include <string.h>
#include "Common/SafeHeap.h"

#include "Frame.h"

// 帧数据头部大小
#define FRAME_HEADER_SIZE           4

/**
 * @brief 安全分配帧数据。
 * @param[in] len 要分配的帧长度，单位为字节。
 * @return 分配好的帧对象。
 */
Frame* Frame_New(Uint8 len)
{
    Frame* self;
    self = (Frame*) SafeHeap_Alloc(len + FRAME_HEADER_SIZE);
    if (self)
    {
        self->len = len;
        self->refCount = 1;
        self->packOffset = 0;
        self->destAddr = 0;
    }
    return self;
}

/**
 * @brief 拷贝构造一个新的数据帧。
 * @param[in] original 原始数据帧。
 * @return 分配好的新帧对象。
 */
Frame* Frame_CopyNew(Frame* original)
{
    Frame* self;
    int rawLen = original->len + FRAME_HEADER_SIZE;
    self = (Frame*) SafeHeap_Alloc(rawLen);
    memcpy(self, original, rawLen);
    self->refCount = 1;
    return self;
}

/**
 * @brief 以应用封包的规格分配帧数据。
 * @param[in] packOffset 封包数据的偏移量，单位为字节。
 * @param[in] packSize 封包数据长度，单位为字节。
 * @return 分配好的帧对象。
 */
Frame* Frame_NewPack(Uint8 packOffset, Uint8 packSize)
{
    Frame* self;
    Uint8 len;

    // 多出2字节作为 CRC 检验域
    len = packOffset + packSize + 2;

    self = (Frame*) SafeHeap_Alloc(len + FRAME_HEADER_SIZE);
    if (self)
    {
        self->len = len;
        self->refCount = 1;
        self->packOffset = packOffset;
        self->destAddr = 0;
    }
    return self;
}

/**
 * @brief 为帧数据增加引用。
 * @param[in] self 要操作的帧对象。
 */
void Frame_AddRef(Frame* self)
{
    ++(self->refCount);
}

/**
 * @brief 安全释放帧数据。
 * @param[in] me 要释放的帧对象指针。
 */
void Frame_Delete(Frame* self)
{
    --(self->refCount);

    // 引用计数为0时，才真正释放内存
    if (self->refCount <= 0)
        SafeHeap_Free(self);
}

/**
 * @brief 从外部拷贝数据到帧对象中。
 * @param[in] self 要操作的帧对象。
 * @param[in] data 外部数据的源地址。
 * @param[in] bytes 要拷贝的字节数。
 */
void Frame_CopyFrom(Frame* self, Byte* data, Uint16 bytes)
{
    memcpy((Byte*)self + FRAME_HEADER_SIZE, data, bytes);
}

/**
 * @brief 拷贝帧数据到到指定地址。
 * @param[in] self 要操作的帧对象。
 * @param[in] data 外部数据的目的地址。
 * @param[in] bytes 要拷贝的字节数。
 */
void Frame_CopyTo(Frame* self, Byte* data, Uint16 bytes)
{
    memcpy(data, (Byte*)self + FRAME_HEADER_SIZE, bytes);
}

/**
 * @brief 获取帧的裸数据指针。
 * @param[in] self 要操作的帧对象。
 * @return 帧裸数据的访问地址。
 */
Byte* Frame_GetData(Frame* self)
{
    return ((Byte*)self + FRAME_HEADER_SIZE);
}

/**
 * @brief 获取封包数据指针。
 * @param[in] self 要操作的帧对象。
 * @return 封包数据的访问地址。
 */
Byte* Frame_GetPackData(Frame* self)
{
    return ((Byte*)self + FRAME_HEADER_SIZE + self->packOffset);
}

/**
 * @brief 获取封包数据长度。
 * @param[in] self 要操作的帧对象。
 * @return 封包数据长度。
 */
Uint8 Frame_GetPackSize(Frame* self)
{
    return (self->len - self->packOffset - 2);
}

/** @} */
