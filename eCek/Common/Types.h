/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 公共类型定义。
 * @details 本接口定义的类型可能会和其它第三方模块冲突。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-5-24
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

/**
 * @brief 布尔基本类型，只能取值为 @ref TRUE 或 @ref FALSE
 */
typedef unsigned char Bool;
#ifndef TRUE
#define TRUE            1       ///< 布尔值：真
#endif
#ifndef FALSE
#define FALSE           0       ///< 布尔值：假
#endif

#ifndef NULL
#define NULL            ((void *)0)     ///< 空指针
#endif

typedef unsigned char Byte;             ///< 字节基本类型
typedef char Char;                      ///< 字符基本类型
typedef short Wchar;                    ///< 宽字符

typedef char Int8;                      ///< 8 位有符号整数
typedef short Int16;                    ///< 16 位有符号整数
typedef long Int32;                     ///< 32 位有符号整数
typedef long long Int64;                ///< 64 位有符号整数
typedef unsigned char Uint8;            ///< 8 位无符号整数
typedef unsigned short Uint16;          ///< 16 位无符号整数
typedef unsigned long Uint32;           ///< 32 位无符号整数
typedef unsigned long long Uint64;      ///< 64 位无符号整数

typedef float Float32;                  ///< 32 位 IEEE 754 浮点数
typedef double Float64;                 ///< 64 位 IEEE 754 浮点数


#endif // COMMON_TYPES_H_

/** @} */
