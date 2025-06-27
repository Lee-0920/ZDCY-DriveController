/**
 * @if _EM_CONSOLE_
 * @mainpage EmConsole
 *
 * EmConsole 是一个简洁的控制台服务程序，适合移植和部署到嵌入式设备。
 *
 * 更详细的说明，请参考 @ref page_EmConsole 。
 *
 * @endif
 */

/**
 * @page page_EmConsole 嵌入式控制台
 *
 * @section sec_EmConsole_introduce 简介
 * EmConsole 是一个简洁的控制台服务程序，适合移植和部署到嵌入式设备。上位机（PC）通过成熟的终端程序连接到设备，
 * 设备便有了强大的输入输出设备，用户可以通过PC的键盘和屏幕控制目标设备，尤其适合嵌入式程序的调试、测试。
 *
 * EmConsole 兼容目前流行的终端客户端程序，如 Microsoft 的超级终端、SecureCRT、minicom 等。
 *
 * EmConsole 支持的功能有：
 * - 回显；
 * - 参数解析；
 * - 格式化输出函数；
 * - 上下方向键调出历史命令；
 * - Ctrl + C / D 终止当前命令；
 *
 * <br> <br>
 * @section sec_EmConsole_guide 使用说明
 * 请参考：
 * - 主题: @ref sec_EmConsole_Resource_Cost
 * - 主题: @ref sec_EmConsole_Architecture
 * - 主题: @ref sec_EmConsole_Transplant
 * - 主题: @ref sec_EmConsole_CmdLine_Coding
 * - 主题: @ref sec_EmConsole_CmdLine_Format_Output
 *
 * <br> <br>
 * @section sec_EmConsole_program_guide 编程手册
 *
 * @subsection sec_EmConsole_Resource_Cost 资源消耗评估
 * EmConsole 代码大小：默认配置编译后约为 2KB
 *
 * EmConsole 内存占用：主要是各个缓冲区占用了大量的内存，但这是可以配置的，
 * 请参考： @ref ConsoleConfig.h 。EmConsole 内存消耗列表如下：
 * - 发送缓冲： @ref CONSOLE_TX_BUF_SIZE
 * - 接收缓冲： @ref CONSOLE_RX_BUF_SIZE
 * - 命令行缓冲： @ref CONSOLE_LINE_BUF_SIZE
 * - 历史命令缓冲： @ref CONSOLE_LINE_BUF_SIZE * @ref CONSOLE_CMD_HISTORY_NUM
 * - 其它全局变量： 约 12 Bytes
 *
 * @subsection sec_EmConsole_Architecture 架构
 * EmConsole 分为三层，分别是：
 * - 驱动层：提供 EmConsole 所依赖的驱动程序，相关代码为：
 *   - 驱动接口定义：@ref IConsoleDriver.h
 *   - UART 驱动程序： @ref UartConsoleDriver.c
 *   - UART 适配器接口定义： @ref UartConsoleAdapter.h
 *
 * - 功能层：EmConsole 主要的功能实现，相关代码为：
 *   - EmConsole 主要实现： @ref Console.c
 *   - EmConsole 与应用层的接口： @ref Console.h
 *
 * - 应用层：所有应用相关的命令都将在这里实现，相关代码为：
 *   - 命令行： @ref CmdLine.c
 *
 * EmConsole 是可配置的，即用户可以在编译时配置控制台的特性和资源，以适合资源有限的嵌入式设备。
 * EmConsole 的详细配置请参考： @ref ConsoleConfig.h
 *
 * @subsection sec_EmConsole_Transplant 移植向导
 * 在移植 EmConsole 时，主要参考 @ref IConsoleDriver.h ，实现底层的通信代码。
 *
 * EmConsole 驱动实现向导：
 * - 实现 @ref ConsoleDriver_WriteData() ，把数据发送到用户控制台中：
 * - 从用户控制台中获取用户输入的命令，交由上层Console处理。
 *   - 获取用户输入的字符；
 *   - 每得到一个字符，即调用 @ref g_kIfNewCharHandle 通知上层；如果CPU睡眠了，还要根据其返回值激活CPU，让后台处理命令或事件。
 *   - 如需使用缓冲，请优先使用 @ref ConsoleConfig.h 中定义的 @ref CONSOLE_TX_BUF_SIZE 和 @ref CONSOLE_RX_BUF_SIZE ，否则需要单独说明；
 *   - 一般说明，不需要实现接收缓冲。Console 模块已经实现了接收缓冲逻辑。
 *
 * 如果通信口是 UART （串口），可参考本程序自带的 @ref UartConsoleDriver.c 。一般来说，
 * 要改动的代码很少，UART 通信控制可以根据实际的CPU ，更改适配器接口
 * @ref UartConsoleAdapter.h 的寄存器定义即可。
 *
 * 最后在后台任务调度中调用 @ref Console_RoutineHandle() 即可。
 *
 * @subsection sec_EmConsole_CmdLine_Coding 命令行实现
 *
 * 在 @ref CmdLine.c 中添加一条应用命令的步骤如下：
 * - 在命令处理函数声明列表处，声明自己的命令处理函数，如：
 * @code
static int Cmd_mycmd(int argc, char *argv[]);
 * @endcode
 *
 * - 在命令行命令表中添加一条命令节点，如：
 * @code
const CmdLineEntry g_kConsoleCmdTable[] =
{
    // ...
    { "mycmd",  Cmd_mycmd,  "\t: Command description" },
    // ...
};
 * @endcode
 *
 * - 实现命令处理函数，如：
 * @code
int Cmd_mycmd(int argc, char *argv[])
{
    if (0 == strcmp(argv[1], "subcmd1"))
    {
        if (0 == strcmp(argv[2], "param"))
        {
            // 调用相关功能函数
        }
    }
    else if (0 == strcmp(argv[1], "subcmd2"))
    {
        // ...
    }
    else if (0 == argv[1] || 0 == strcmp(argv[1], "help") || 0 == strcmp(argv[1], "?"))
    {
        ConsoleOut("====== Sub commands ======\n");
        ConsoleOut(" mycmd subcmd1 param : Sub command description\n");
    }
    else
    {
        ConsoleOut("Invalid operation: %s\n", argv[1]);
    }

    // 成功返回 0
    return (0);
}
 * @endcode
 *
 * @subsection sec_EmConsole_CmdLine_Format_Output 格式化输出
 * 在应用程序中，可以使用以下格式化输出函数（宏）进行信息的打印：
 * - @ref Console_Out()
 * - @ref ConsoleOut
 * - @ref EM_TRACE
 *
 * 格式化输出的格式为：
 * @copydoc Console_Out
 */



/**
* @defgroup module_EmConsole 嵌入式控制台
*/
