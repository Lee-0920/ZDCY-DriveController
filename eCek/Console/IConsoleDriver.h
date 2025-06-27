/**
 * @addtogroup module_EmConsole
 * @{
 */

/**
 * @file
 * @brief Console 驱动程序接口定义。
 * @details 提供Console与Driver之间的接口函数；定义控制台驱动程序所需要实现的接口。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-5-20
 */

#ifndef I_CONSOLE_DRIVER_H_
#define I_CONSOLE_DRIVER_H_

#define CONSOLE_MODE_NONBLOCK           0   ///< 非阻塞（中断）模式（系统默认）
#define CONSOLE_MODE_BLOCK              1   ///< 阻塞模式

/**
 * @brief 控制台驱动初始化。
 */
extern void ConsoleDriver_Init();

/**
 * @brief 切换模式。
 * @details 支持的模式为：
 *  - @ref CONSOLE_MODE_NONBLOCK
 *  - @ref CONSOLE_MODE_BLOCK
 */
extern void ConsoleDriver_SwitchMode(unsigned char mode);

/**
 * @brief 写入数据到控制台（非阻塞）。
 * @param data 要写入的数据缓冲。
 * @param len 要写入的字节数。
 * @return 真正写入的字节数。
 * @note 实现时，需要将数据中的单个“\\n” 转换成“\\r\\n”。
 */
extern int ConsoleDriver_WriteData(const char* data, int len);

//**
// * @brief 写入一个字符到控制台。
// * @param data 要写入的字符。
// * @note 实现时，需要将单个“\\n” 转换成“\\r\\n”。
// */
//extern void ConsoleDriver_WriteChar(char data);
//
//**
// * @brief 写入字符串到控制台。
// * @param data 要写入的字符串。
// * @note 实现时，需要将字符串中的单个“\\n” 转换成“\\r\\n”。
// */
//extern void ConsoleDriver_WriteString(const char* data);

/**
 * @brief 接口函数原型定义：新字符处理函数。
 * @param data 要处理的新字符。
 * @details 由 上层 Console 实现。
 * @return 是否需要触发响应事件（如新命令）。
 * @retval 0 不需要触发。
 * @retval 1 需要触发。
 */
typedef char (* IfNewCharHandle)(char data);

/**
 * @brief 上层 Console 提供的新字符处理的接口函数。
 * @details 由 上层 Console 定义具体的新字符处理函数。
 */
extern const IfNewCharHandle g_kIfNewCharHandle;


#endif // I_CONSOLE_DRIVER_H_

/** @} */
