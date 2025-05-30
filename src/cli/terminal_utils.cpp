#include "cli/terminal_utils.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#include <windows.h>

#else
#include <sys/ioctl.h>
#include <unistd.h>

#endif

#include "core/config.h"

namespace neumann { namespace cli {

TerminalUtils& TerminalUtils::getInstance()
{
    static TerminalUtils instance;
    return instance;
}

TerminalUtils::TerminalUtils() : colorEnabled(false)
{
    // 根据配置设置是否启用彩色输出
    colorEnabled = Config::getInstance().getEnableColorOutput() && detectColorSupport();
}

void TerminalUtils::setColorEnabled(bool enabled)
{
    colorEnabled = enabled;
}

bool TerminalUtils::isColorEnabled() const
{
    return colorEnabled;
}

std::string TerminalUtils::getAnsiCode(Color color, TextStyle style)
{
    if (!colorEnabled) {
        return "";
    }

    std::stringstream ss;
    ss << "\033[";

    if (style != TextStyle::NORMAL) {
        ss << static_cast<int>(style) << ";";
    }

    if (color != Color::RESET) {
        ss << static_cast<int>(color);
    } else {
        ss << "0";
    }

    ss << "m";
    return ss.str();
}

bool TerminalUtils::detectColorSupport()
{
#ifdef _WIN32
    // Windows 10 版本 1607 及以上支持 ANSI 转义序列
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD consoleMode;
    if (!GetConsoleMode(hConsole, &consoleMode)) {
        return false;
    }

    // 尝试启用 ANSI 转义序列
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (SetConsoleMode(hConsole, consoleMode)) {
        return true;
    }

    return false;
#else
    // 对于类Unix系统，检查TERM环境变量
    const char* term = getenv("TERM");
    if (term == nullptr) {
        return false;
    }

    std::string termStr(term);
    return (
        termStr.find("color") != std::string::npos || termStr.find("256") != std::string::npos ||
        termStr.find("xterm") != std::string::npos || termStr.find("screen") != std::string::npos);
#endif
}

std::string TerminalUtils::colorText(const std::string& text, Color color, TextStyle style)
{
    if (!colorEnabled) {
        return text;
    }

    return getAnsiCode(color, style) + text + getAnsiCode(Color::RESET);
}

void TerminalUtils::printColor(const std::string& text, Color color, TextStyle style)
{
    std::cout << colorText(text, color, style);
}

void TerminalUtils::printSuccess(const std::string& message)
{
    printColor("✓ " + message, Color::BRIGHT_GREEN, TextStyle::BOLD);
    std::cout << std::endl;
}

void TerminalUtils::printWarning(const std::string& message)
{
    printColor("⚠ " + message, Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;
}

void TerminalUtils::printError(const std::string& message)
{
    printColor("✗ " + message, Color::BRIGHT_RED, TextStyle::BOLD);
    std::cout << std::endl;
}

void TerminalUtils::printInfo(const std::string& message)
{
    printColor("ℹ " + message, Color::BRIGHT_CYAN, TextStyle::NORMAL);
    std::cout << std::endl;
}

void TerminalUtils::showProgress(int current, int total, const std::string& prefix, int width)
{
    if (total <= 0) return;

    double percentage = static_cast<double>(current) / total;
    int filledWidth = static_cast<int>(percentage * width);

    moveToLineStart();
    clearCurrentLine();

    std::cout << prefix;
    if (!prefix.empty()) {
        std::cout << " ";
    }

    // 进度条边框
    printColor("[", Color::WHITE);

    // 已完成部分
    for (int i = 0; i < filledWidth; ++i) {
        printColor("█", Color::BRIGHT_GREEN);
    }

    // 未完成部分
    for (int i = filledWidth; i < width; ++i) {
        printColor("░", Color::WHITE);
    }

    printColor("]", Color::WHITE);

    // 百分比
    std::cout << " " << std::setw(3) << static_cast<int>(percentage * 100) << "% ";
    std::cout << "(" << current << "/" << total << ")";

    std::cout.flush();
}

void TerminalUtils::showSpinner(const std::string& message, int durationMs)
{
    const std::vector<std::string> spinnerChars = {"⠋", "⠙", "⠹", "⠸", "⠼",
                                                   "⠴", "⠦", "⠧", "⠇", "⠏"};
    const int spinDelay = 100;  // 毫秒
    const int steps = durationMs / spinDelay;

    hideCursor();

    for (int i = 0; i < steps; ++i) {
        moveToLineStart();
        clearCurrentLine();

        printColor(spinnerChars[i % spinnerChars.size()], Color::BRIGHT_CYAN);
        std::cout << " " << message;
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(spinDelay));
    }

    moveToLineStart();
    clearCurrentLine();
    showCursor();
}

void TerminalUtils::clearCurrentLine()
{
    if (colorEnabled) {
        std::cout << "\033[2K";
    } else {
        // 简单的清行方法，打印足够的空格
        std::cout << std::string(80, ' ');
        moveToLineStart();
    }
}

void TerminalUtils::moveToLineStart()
{
    if (colorEnabled) {
        std::cout << "\r";
    }
}

void TerminalUtils::hideCursor()
{
    if (colorEnabled) {
        std::cout << "\033[?25l";
    }
}

void TerminalUtils::showCursor()
{
    if (colorEnabled) {
        std::cout << "\033[?25h";
    }
}

std::string TerminalUtils::createTableSeparator(int width, char separator)
{
    return std::string(width, separator);
}

// 计算字符串的显示宽度（考虑中文字符占用2个字符宽度）
int TerminalUtils::getDisplayWidth(const std::string& str)
{
    int width = 0;
    for (size_t i = 0; i < str.length();) {
        unsigned char c = str[i];
        if (c < 0x80) {
            // ASCII字符，占用1个字符宽度
            width += 1;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // UTF-8 2字节字符
            width += 2;  // 通常是中文字符，占用2个字符宽度
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // UTF-8 3字节字符（大部分中文）
            width += 2;  // 中文字符占用2个字符宽度
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // UTF-8 4字节字符
            width += 2;  // 假设占用2个字符宽度
            i += 4;
        } else {
            // 无效UTF-8序列，跳过
            i += 1;
        }
    }
    return width;
}

std::string TerminalUtils::formatTableRow(const std::vector<std::string>& columns,
                                          const std::vector<int>& widths,
                                          const std::string& alignment)
{
    if (columns.size() != widths.size()) {
        return "";
    }

    std::ostringstream oss;

    for (size_t i = 0; i < columns.size(); ++i) {
        std::string content = columns[i];
        int width = widths[i];
        char align = (i < alignment.size()) ? alignment[i] : 'l';

        // 计算实际显示宽度
        int displayWidth = getDisplayWidth(content);

        // 实现正确的对齐逻辑，基于显示宽度
        if (displayWidth >= width) {
            // 内容显示宽度超过列宽，直接输出
            oss << content;
        } else {
            int padding = width - displayWidth;

            if (align == 'r') {
                // 右对齐：添加左侧空格
                oss << std::string(padding, ' ') << content;
            } else if (align == 'c') {
                // 中心对齐：平均分配左右空格
                int leftPad = padding / 2;
                int rightPad = padding - leftPad;
                oss << std::string(leftPad, ' ') << content << std::string(rightPad, ' ');
            } else {
                // 左对齐：添加右侧空格
                oss << content << std::string(padding, ' ');
            }
        }

        // 添加列间分隔符（除了最后一列）
        if (i < columns.size() - 1) {
            oss << " ";
        }
    }

    return oss.str();
}

int TerminalUtils::getTerminalWidth()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

}}  // namespace neumann::cli