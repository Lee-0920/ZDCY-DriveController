/**
 * @addtogroup module_EmConsole
 * @{
 */

/**
 * @file
 * @brief 控制台服务程序。
 * @details 实现控制台服务的常用特性和功能，支持常见的PC控制台客户端。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-5-21
 */

#include <stdarg.h>
#include <string.h>

#include "Port/Driver/ConsoleUart.h"
#include "ConsoleConfig.h"
#include "ConsoleScheduler.h"
#include "IConsoleDriver.h"
#include "Console.h"

// 新字符处理函数
static char Console_NewCharHandle(char data);
const IfNewCharHandle g_kIfNewCharHandle = Console_NewCharHandle;

// 十六进制的格式化打印
static const char * const s_kHexCharTable = "0123456789abcdef";


//************************************
// 命令行解析状态
#define CONSOLE_EVENT_NONE              0   // 暂无命令
#define CONSOLE_EVENT_NEW_CMD           1   // 新命令
#define CONSOLE_EVENT_TERMINATE         2   // 终止命令，结束运行当前命令
static volatile unsigned char s_consoleEvent = CONSOLE_EVENT_NONE;

// 命令行解析结果
#define CMDLINE_BAD_CMD         (-1)    // 系统不支持的命令（未定义）
#define CMDLINE_TOO_MANY_ARGS   (-2)    // 输入的命令行参数太多
#define CMDLINE_NO_CMD          (-3)    // 无命令

// 命令行解析并执行
int Console_ParseAndExecute(char * cmdLine);

// 上一个字符
static volatile char s_lastChar = 0;

// ESC 转换符状态机
#define ESC_STATUS_INIT                 0
#define ESC_STATUS_ESCAPING             1
#define ESC_STATUS_ESCAPING2            2

volatile unsigned char s_escStatus = ESC_STATUS_INIT;

//************************************
// 命令行接收缓冲和写索引
static char s_bufRx[CONSOLE_RX_BUF_SIZE];
static volatile int s_idxWriteRx = 0;

//************************************
// 历史命令行缓冲队列，只写不读的环形缓冲，行缓冲中命令行结束符为0
static char s_bufLines[CONSOLE_CMD_HISTORY_NUM][CONSOLE_LINE_BUF_SIZE];
// 当前命令行缓冲，命令解析工作在本缓冲中进行
// 命令解析和命令执行时会更改缓冲内容，所以不能和历史命令行缓冲队列共用
static char s_bufLine[CONSOLE_LINE_BUF_SIZE];
// 当前命令行写索引，接到新命令会往前挪
static volatile unsigned char s_idxCurLine = 0;
// 当前翻看索引，用于上下方向键翻看历史命令
static volatile unsigned char s_idxViewLine = 0;

// 历史命令队列初始化
#define HISTORY_LINE_INIT()                                                 \
{                                                                           \
    for (s_idxCurLine = 0; s_idxCurLine < CONSOLE_CMD_HISTORY_NUM; s_idxCurLine++)  \
        s_bufLines[s_idxCurLine][0] = 0;                                    \
    s_idxCurLine = 0;                                                       \
}

// 历史命令向后退
#define HISTORY_LINE_BACK()                                                                 \
{                                                                                           \
    (s_idxCurLine = (s_idxCurLine + CONSOLE_CMD_HISTORY_NUM - 1) % CONSOLE_CMD_HISTORY_NUM);\
}

// 历史命令向前进
#define HISTORY_LINE_AHEAD()                                                \
{                                                                           \
    (s_idxCurLine = (s_idxCurLine + 1) % CONSOLE_CMD_HISTORY_NUM);          \
}

static void Console_SaveCmdLine(char * bufLine);


/**
 * @brief 控制台初始化。
 */
void Console_Init(void)
{
    // 驱动初始化
    ConsoleUart_Init();

    ConsoleDriver_Init();
    Console_Welcome();

    s_consoleEvent = CONSOLE_EVENT_NONE;
    s_escStatus = ESC_STATUS_INIT;

    s_idxWriteRx  = 0;
    HISTORY_LINE_INIT();

    s_idxViewLine = s_idxCurLine;

    // 系统调用初始化
    ConsoleScheduler_Init();
}

/**
 * @brief 新字符处理函数。
 * @details 进行回显处理，并保存到接收缓冲中。
 */
char Console_NewCharHandle(char data)
{
    // 返回：是否需要触发响应事件
    char ret = 0;

    switch(data)
    {
    case '\b':
    case '\x7F':
        // 回退符
        // 在用户终端中擦除前一个字符
        if (s_idxWriteRx > 0)
        {
            s_idxWriteRx--;
            ConsoleDriver_WriteData("\b \b", 3);
        }
        break;
    case 0x03:
    case 0x04:
        // 终止符（Ctrl+C 或 Ctrl + D）
        s_idxWriteRx = 0;
        // 通知后台任务处理该事件
        s_consoleEvent = CONSOLE_EVENT_TERMINATE;
        ret = 1;
        break;
    case '\r':
    case '\n':
        // 遇到“\r\n” 只当作一个 “\n”处理
        if (!((s_lastChar == '\r') && (data == '\n')))
        {
            // 解析到新行
            // 从接收缓冲中拷出数据到命令行缓冲，并清空接收缓冲
            memcpy(s_bufLine, s_bufRx, s_idxWriteRx);
            s_bufLine[s_idxWriteRx] = 0;
            s_idxWriteRx = 0;

            // 通知后台任务处理该事件
            s_consoleEvent = CONSOLE_EVENT_NEW_CMD;
            ret = 1;

            ConsoleDriver_WriteData("\n", 1);
        }
        break;
    case 0x1b:
    case '[':
    case 'A':
    case 'B':
        // ESC 转换符处理：ESC 转换状态机
        switch (s_escStatus)
        {
        case ESC_STATUS_INIT:
            if (data == 0x1b)   // ESC
            {
                s_escStatus = ESC_STATUS_ESCAPING;
            }
            else
            {
                // 保存到接收缓冲并回显
                if (s_idxWriteRx < CONSOLE_RX_BUF_SIZE - 1)
                {
                    s_bufRx[s_idxWriteRx++] = data;
                    ConsoleDriver_WriteData(&data, 1);
                }
            }
            break;

        case ESC_STATUS_ESCAPING:
            if (data == '[')
            {
                s_escStatus = ESC_STATUS_ESCAPING2;
            }
            else
            {
                s_escStatus = ESC_STATUS_INIT;
            }
            break;

        case ESC_STATUS_ESCAPING2:
            // 切换历史记录中的当前命令行缓冲
            if (data == 'A')    // 上箭头
            {
                s_idxViewLine = (s_idxViewLine + CONSOLE_CMD_HISTORY_NUM - 1) % CONSOLE_CMD_HISTORY_NUM;
            }
            else if (data == 'B')   // 下箭头
            {
                s_idxViewLine = (s_idxViewLine + 1) % CONSOLE_CMD_HISTORY_NUM;
            }

            // 处理上下方向键为调出历史命令
            if (data == 'A' || data == 'B')
            {
                // 先清空终端窗口（接收缓冲）中的临时字符串
                while (s_idxWriteRx--)
                {
                    ConsoleDriver_WriteData("\b \b", 3);
                }

                // 取出当前翻看处的命令行到终端窗口和接收缓冲
                s_idxWriteRx = strlen(s_bufLines[s_idxViewLine]);
                ConsoleDriver_WriteData(s_bufLines[s_idxViewLine], s_idxWriteRx);

                // 写到当前接收缓冲中
                memcpy(s_bufRx, s_bufLines[s_idxViewLine], s_idxWriteRx);
            }

            s_escStatus = ESC_STATUS_INIT;
            break;
        }
        break;

    default:
        // 保存到接收缓冲并回显
        if (s_idxWriteRx < CONSOLE_RX_BUF_SIZE - 1)
        {
            s_bufRx[s_idxWriteRx++] = data;
            ConsoleDriver_WriteData(&data, 1);
        }
        break;
    }

    // 记下当前字符
    s_lastChar = data;

    return ret;
}

/**
 * @brief Console 例行处理程序。
 * @details 请把该函数加入后台任务中。
 * @note 该函数需要在后台中定时执行，或者有命令时执行（CPU从睡眠到激活）
 * @return 处理状态。
 * @retval CCRC_OK 执行正常；
 * @retval CCRC_CMD_TERMINATE 命令终止，用户可能按了“Ctrl+C或Ctrl+D”
 */
int Console_RoutineHandle(void)
{
    int status;

    // 检查是否有新命令行
    if (s_consoleEvent != CONSOLE_EVENT_NONE)
    {
        if (s_consoleEvent == CONSOLE_EVENT_NEW_CMD)
        {
            // 重置
            s_consoleEvent = CONSOLE_EVENT_NONE;

            // 保存到历史缓冲中
            Console_SaveCmdLine(s_bufLine);

            // 解析命令并执行，返回命令执行结果
            status = Console_ParseAndExecute(s_bufLine);

            //
            // 无命令
            //
            if(status == CMDLINE_NO_CMD)
            {
                Console_Out(CONSOLE_PROMPT_STRING);
                return CCRC_OK;
            }

            //
            // 无效的命令
            //
            else if(status == CMDLINE_BAD_CMD)
            {
                status = 0;
                // 跳过空白字符
                while (0 == s_bufLine[status])
                    status++;
                Console_Out("Invalid command: %s\n", &(s_bufLine[status]));
            }

            //
            // 参数太多
            //
            else if(status == CMDLINE_TOO_MANY_ARGS)
            {
                Console_Out("Too many arguments for command processor!\n");
            }

            //
            // 忽略提示符
            //
            else if(status == CCRC_OMIT_PROMPT)
            {
                // 直接返回，不输出命令提示符
                return CCRC_OK;
            }

            //
            // 终止程序
            //
            else if(status == CCRC_CMD_TERMINATE)
            {
                // 提示上层应该终止程序
                return CCRC_CMD_TERMINATE;
            }

            // 命令已执行，如果返回不成功（非0），则打印相应的错误码
            else if(status != 0)
            {
                Console_Out("Command returned error code: %d\n", status);
            }

            // 命令执行完后，输出新行和提示符
            Console_Out("\n");
            Console_Out(CONSOLE_PROMPT_STRING);
        }
        // 命令终止符（用户在终端按了Ctrl+C或Ctrl+D）
        else if (s_consoleEvent == CONSOLE_EVENT_TERMINATE)
        {
            // 重置
            s_consoleEvent = CONSOLE_EVENT_NONE;

            return CCRC_CMD_TERMINATE;
        }
    }

    return CCRC_OK;
}

// 保存有效的命令行到历史缓冲队列中
void Console_SaveCmdLine(char * bufLine)
{
    unsigned int i = 0;

    // 去掉空白字符
    while (bufLine[i])
    {
        if (bufLine[i] == ' ' || bufLine[i] == '\t')
            i++;
        else
            break;
    }
    bufLine += i;

    if (*bufLine)
    {
        i = 0;
        while((i < CONSOLE_LINE_BUF_SIZE) && bufLine[i])
        {
            s_bufLines[s_idxCurLine][i] = bufLine[i];
            i++;
        }

        if (i < CONSOLE_LINE_BUF_SIZE)
            s_bufLines[s_idxCurLine][i] = 0;
        else
            s_bufLines[s_idxCurLine][i-1] = 0;

        HISTORY_LINE_AHEAD();
        s_idxViewLine = s_idxCurLine;
    }
}

/**
 * @brief 解析命令行文本并执行命令。
 * @param cmdLine 命令行文本字符串。
 * @return 解析或执行状态。
 */
int Console_ParseAndExecute(char * cmdLine)
{
    static char *argv[CONSOLE_MAX_ARGUMENTS + 1];
    char * pos;
    int argc;
    int bFindArg = 1;
    CmdLineEntry *cmdEntry;

    // 初始化各参数为空，方便应用命令解析
    for (argc = 0; argc <= CONSOLE_MAX_ARGUMENTS; argc++)
        argv[argc] = 0;

    // 开始解析命令行
    argc = 0;
    pos = cmdLine;

    // 解析整行文本直到遇到空字符，表示行文本结束
    while(*pos)
    {
        // 空白字符是一个参数的 “前导符”
        if(*pos == ' ' || *pos == '\t')
        {
            *pos = 0;
            bFindArg = 1;
        }
        else
        {
            // 有非空白字符，一个参数开始
            // 上一次有找到过参数
            if(bFindArg)
            {
                if(argc < CONSOLE_MAX_ARGUMENTS)
                {
                    // 记录该参数
                    argv[argc] = pos;
                    argc++;
                    bFindArg = 0;
                }
                else
                {
                    // 参数过多，停止解析
                    return(CMDLINE_TOO_MANY_ARGS);
                }
            }
        }

        pos++;
    }

    // 有解析到参数；第一个参数即是命令关键字
    if(argc)
    {
        // 遍历整个命令表，匹配关键字
        cmdEntry = (CmdLineEntry *) &g_kConsoleCmdTable[0];

        while(cmdEntry->cmdKeyword)
        {
            // 匹配命令关键字，调用相应的命令处理函数
            if(!strcmp(argv[0], cmdEntry->cmdKeyword))
            {
                return(cmdEntry->cmdHandle(argc, argv));
            }

            cmdEntry++;
        }
    }
    else
    {
        // 无命令
        return (CMDLINE_NO_CMD);
    }

    // 没有找到命令，无法识别该命令
    return(CMDLINE_BAD_CMD);
}

/**
 * @brief 显示命令输入提示符。
 * @details 提示符定义为： @ref CONSOLE_PROMPT_STRING 。
 */
void Console_Prompt(void)
{
    Console_Out(CONSOLE_PROMPT_STRING);
}

/**
 * @brief 控制台格式化输出函数。
 * @param strFormat 格式化字符串。
 * @details 类似printf的另一个简单实现。
 * @note 本格式化输出函数暂时仅支持以下格式化控制符：
 *  - %c ：输出一个字符
 *  - %s ：输出一个字符串
 *  - %d ：输出一个整数
 *  - %u ：输出一个无符号整数
 *  - %p ：输出一个地址（十六进制）
 *  - %x 和 %X ：以十六进制格式输出一个整数
 */
void Console_Out(const char *strFormat, ...)
{
    unsigned int ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg;
    char *pcStr, pcBuf[16], cFill;
    double fValue;
    double decValue;
    unsigned int len;
    va_list vaArgP;

    //
    // Start the varargs processing.
    //
    va_start(vaArgP, strFormat);

    //
    // Loop while there are more characters in the string.
    //
    while(*strFormat)
    {
        //
        // Find the first non-% character, or the end of the string.
        //
        for(ulIdx = 0; (strFormat[ulIdx] != '%') && (strFormat[ulIdx] != '\0');
            ulIdx++)
        {
        }

        //
        // Write this portion of the string.
        //
        ConsoleDriver_WriteData(strFormat, ulIdx);

        //
        // Skip the portion of the string that was written.
        //
        strFormat += ulIdx;

        //
        // See if the next character is a %.
        //
        if(*strFormat == '%')
        {
            //
            // Skip the %.
            //
            strFormat++;

            //
            // Set the digit count to zero, and the fill character to space
            // (i.e. to the defaults).
            //
            ulCount = 0;
            cFill = ' ';

            //
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
            //
again:

            //
            // Determine how to handle the next character.
            //
            switch(*strFormat++)
            {
                //
                // Handle the digit characters.
                //
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    //
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    //
                    if((strFormat[-1] == '0') && (ulCount == 0))
                    {
                        cFill = '0';
                    }

                    //
                    // Update the digit count.
                    //
                    ulCount *= 10;
                    ulCount += strFormat[-1] - '0';

                    //
                    // Get the next character.
                    //
                    goto again;
                }

                //
                // Handle the %c command.
                //
                case 'c':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(vaArgP, unsigned int);

                    //
                    // Print out the character.
                    //
                    ConsoleDriver_WriteData((char *)&ulValue, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %d command.
                //
                case 'd':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(vaArgP, unsigned int);

                    //
                    // Reset the buffer position.
                    //
                    ulPos = 0;

                    //
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    //
                    if((int)ulValue < 0)
                    {
                        //
                        // Make the value positive.
                        //
                        ulValue = -(int)ulValue;

                        //
                        // Indicate that the value is negative.
                        //
                        ulNeg = 1;
                    }
                    else
                    {
                        //
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        //
                        ulNeg = 0;
                    }

                    //
                    // Set the base to 10.
                    //
                    ulBase = 10;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                case 'f':
                {
                    // Get the value from the varargs.
                    fValue = va_arg(vaArgP, double);  ///取到浮点数

                    if(fValue < 0)
                    {
                        ulNeg = 1;
                        fValue = - fValue;
                    }
                    else
                    {
                        ulNeg = 0;
                    }

                    ulValue = (int)fValue;

                    decValue = fValue - ulValue;

                    ulBase = 10;

                    for(ulIdx = 1, len = 1;(((ulIdx * ulBase) <= ulValue) &&(((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, len++)
                    {
                    }

                    ulPos = 0;

                    if(ulNeg == 1)
                    {
                        pcBuf[ulPos++] = '-';
                    }

                    for(; ulIdx; ulIdx /= ulBase)
                    {
                        pcBuf[ulPos++] = s_kHexCharTable[(ulValue / ulIdx) % ulBase];
                    }

                    if(decValue >= 0)
                    {
                        pcBuf[ulPos++] = '.';
                    }

                    for(; len <= 6; len++)
                    {
                        decValue *= ulBase;
                        pcBuf[ulPos++] = s_kHexCharTable[(int)decValue % 10];
                    }

                    // Write the string.
                    ConsoleDriver_WriteData(pcBuf, ulPos);

                    break;
                }

                //
                // Handle the %s command.
                //
                case 's':
                {
                    //
                    // Get the string pointer from the varargs.
                    //
                    pcStr = va_arg(vaArgP, char *);

                    //
                    // Determine the length of the string.
                    //
                    for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
                    {
                    }

                    //
                    // Write the string.
                    //
                    ConsoleDriver_WriteData(pcStr, ulIdx);

                    //
                    // Write any required padding spaces
                    //
                    if(ulCount > ulIdx)
                    {
                        ulCount -= ulIdx;
                        while(ulCount--)
                        {
                            ConsoleDriver_WriteData(" ", 1);
                        }
                    }
                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %u command.
                //
                case 'u':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(vaArgP, unsigned int);

                    //
                    // Reset the buffer position.
                    //
                    ulPos = 0;

                    //
                    // Set the base to 10.
                    //
                    ulBase = 10;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ulNeg = 0;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %x and %X commands.  Note that they are treated
                // identically; i.e. %X will use lower case letters for a-f
                // instead of the upper case letters is should use.  We also
                // alias %p to %x.
                //
                case 'x':
                case 'X':
                case 'p':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(vaArgP, unsigned int);

                    //
                    // Reset the buffer position.
                    //
                    ulPos = 0;

                    //
                    // Set the base to 16.
                    //
                    ulBase = 16;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ulNeg = 0;

                    //
                    // Determine the number of digits in the string version of
                    // the value.
                    //
convert:
                    for(ulIdx = 1;
                        (((ulIdx * ulBase) <= ulValue) &&
                         (((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, ulCount--)
                    {
                    }

                    //
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    //
                    if(ulNeg)
                    {
                        ulCount--;
                    }

                    //
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    //
                    if(ulNeg && (cFill == '0'))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ulPos++] = '-';

                        //
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        //
                        ulNeg = 0;
                    }

                    //
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    //
                    if((ulCount > 1) && (ulCount < 16))
                    {
                        for(ulCount--; ulCount; ulCount--)
                        {
                            pcBuf[ulPos++] = cFill;
                        }
                    }

                    //
                    // If the value is negative, then place the minus sign
                    // before the number.
                    //
                    if(ulNeg)
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ulPos++] = '-';
                    }

                    //
                    // Convert the value into a string.
                    //
                    for(; ulIdx; ulIdx /= ulBase)
                    {
                        pcBuf[ulPos++] = s_kHexCharTable[(ulValue / ulIdx) % ulBase];
                    }

                    //
                    // Write the string.
                    //
                    ConsoleDriver_WriteData(pcBuf, ulPos);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %% command.
                //
                case '%':
                {
                    //
                    // Simply write a single %.
                    //
                    ConsoleDriver_WriteData(strFormat - 1, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle all other commands.
                //
                default:
                {
                    //
                    // Indicate an error.
                    //
                    ConsoleDriver_WriteData("ERROR", 5);

                    //
                    // This command has been handled.
                    //
                    break;
                }
            }
        }
    }

    //
    // End the varargs processing.
    //
    va_end(vaArgP);
}

/**
 * @brief 输出一个字符串到控制台。
 * @param str 要输出的字符串。
 */
void Console_OutString(const char * str)
{
    ConsoleDriver_WriteData(str, strlen(str));
}


/**
 * @brief 显示欢迎信息。
 */
void Console_Welcome(void)
{
#if (CONSOLE_NEED_WELCOME_MSG == 1)

    // 打印欢迎信息
    Console_Out("\n\n----------------\n");
    Console_Out("Welcome to use %s Console\n", CONSOLE_WELCOME_PRODUCT);

    // 打印版本号
#if (CONSOLE_WELCOME_NEED_VERSION == 1)
    Console_Out("Version: %u.%u.%u.%u\n",
            g_kCmdLineVersion.major,
            g_kCmdLineVersion.minor,
            g_kCmdLineVersion.revision,
            g_kCmdLineVersion.build
            );
#endif // CONSOLE_WELCOME_NEED_VERSION

#ifdef CONSOLE_WELCOME_COPYRIGHT
    Console_Out(CONSOLE_WELCOME_COPYRIGHT);
#endif // CONSOLE_WELCOME_COPYRIGHT

#ifdef CONSOLE_WELCOME_OTHER
    Console_Out("\n%s\n", CONSOLE_WELCOME_OTHER);
#endif // CONSOLE_WELCOME_OTHER

    Console_Out("\n\n");
    Console_Prompt();

#endif // CONSOLE_NEED_WELCOME_MSG
}

/** @} */
