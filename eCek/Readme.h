/**
 * @if _EM_ECEK_
 * @mainpage eCek
 *
 * eCek 全称为 Embedded Common Extension Kit，嵌入式常用扩展包，为丰富嵌入式 MCU 应用项目
 * 而开发或移植的有用的源码包。
 *
 * 更详细的说明，请参考 @ref page_eCek 。
 *
 * @endif
 */

/**
 * @page page_eCek 嵌入式常用扩展包
 *
 * @section sec_eCek_introduce 简介
 * eCek 全称为 Embedded Common Extension Kit，嵌入式常用扩展包，为丰富嵌入式 MCU 应用项目
 * 而开发或移植的有用的源码包。
 *
 * eCek 基于 FreeRTOS 进行开发，但并不局限于这个 RTOS，您可以很方便快捷地移植到其它系统或裸机。
 *
 * eCek 项目的目标：
 * - 可移植性：可方便移植到不同的硬件平台，且可方便移植到不同的 RTOS/裸机；
 * - 可定制性：除必要的核心基础包外，其余各应用包可自行去除；
 * - 平台性：通过整合各种常用工具，打造一个功能强大的嵌入式MCU软件平台；
 *
 * 目前 eCek 项目包括：
 * - 嵌入式控制台 @ref page_EmConsole
 * - Trace 多级别跟踪工具
 * - DNCP 设备简单控制协议/栈
 * - eLua 嵌入脚本解析引擎
 *
 * eCek 中的包有几类：
 * - 逻辑包/库：不需要移植，可直接使用；
 * - 底层包：依赖于底层硬件，需要移植相关驱动，见 port 目录；
 * - 上层包：依赖于上层OS调度，需要移植操作系统相关调度，见 port 目录；
 * - 中间件：不仅依赖于底层硬件，还依赖于上层OS调度，上下都需要移植，见 port 目录；
 *
 */



/**
* @defgroup module_eCek 嵌入式常用扩展包
*/
