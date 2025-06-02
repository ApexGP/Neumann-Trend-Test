#include "cli/terminal_utils.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <conio.h>
#include <io.h>
#include <windows.h>

#else
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#endif

#include "cli/file_browser.h"  // 引入新的文件浏览器模块
#include "core/config.h"
#include "core/i18n.h"

namespace fs = std::filesystem;

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

std::string TerminalUtils::promptForFilePath(const std::string& prompt, bool directories_only)
{
    // 使用新的FileBrowser模块
    FileBrowser browser;
    return browser.selectFile(prompt, directories_only);
}

std::string TerminalUtils::promptForFilePathWithTabCompletion(const std::string& prompt,
                                                              bool directories_only)
{
    std::cout << prompt << std::endl;
    printInfo(_("input.enter_for_list"));
#ifdef _WIN32
    printInfo(_("input.windows_tab_help"));
#else
    printInfo(_("input.tab_completion_instruction"));
#endif

    std::string currentInput;

    while (true) {
        std::cout << "> ";

#ifdef _WIN32
        // Windows版本：使用简化的输入处理
        std::string line;
        std::getline(std::cin, line);

        // 检查是否是特殊命令
        if (line.empty()) {
            // 显示当前目录内容
            showCurrentDirectoryContents(directories_only);
            continue;
        } else if (line == "q" || line == "quit" || line == "exit") {
            return "";
        } else if (line == "?") {
            showPathInputHelp();
            continue;
        } else {
            return line;
        }
#else
        // Unix版本的简化处理
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) {
            showCurrentDirectoryContents(directories_only);
            continue;
        } else if (line == "q" || line == "quit" || line == "exit") {
            return "";
        } else if (line == "?") {
            showPathInputHelp();
            continue;
        } else {
            return line;
        }
#endif
    }

    return "";
}

std::string TerminalUtils::promptForDirectory(const std::string& prompt)
{
    // 使用新的FileBrowser模块
    FileBrowser browser;
    return browser.selectDirectory(prompt);
}

std::vector<std::string> TerminalUtils::getFileCompletions(const std::string& partial_path,
                                                           bool directories_only)
{
    std::vector<std::string> completions;

    try {
        fs::path searchPath;
        std::string prefix;
        std::string basePath;

        if (partial_path.empty()) {
            searchPath = fs::current_path();
            basePath = "";
        } else {
            fs::path inputPath(partial_path);

            if (partial_path.back() == '/' || partial_path.back() == '\\') {
                // 用户输入的是目录路径
                searchPath = inputPath;
                basePath = partial_path;
                prefix = "";
            } else {
                // 用户可能输入了部分文件名
                if (fs::exists(inputPath) && fs::is_directory(inputPath)) {
                    searchPath = inputPath;
                    basePath = partial_path + "/";
                    prefix = "";
                } else {
                    searchPath = inputPath.parent_path();
                    prefix = inputPath.filename().string();
                    basePath = searchPath.string();
                    if (!basePath.empty() && basePath.back() != '/' && basePath.back() != '\\') {
                        basePath += "/";
                    }
                }
            }
        }

        if (searchPath.empty()) {
            searchPath = fs::current_path();
        }

        if (fs::exists(searchPath) && fs::is_directory(searchPath)) {
            for (const auto& entry : fs::directory_iterator(searchPath)) {
                std::string filename = entry.path().filename().string();

                // 跳过隐藏文件
                if (filename.front() == '.') {
                    continue;
                }

                // 检查前缀匹配
                if (prefix.empty() ||
                    filename.substr(0, std::min(prefix.length(), filename.length())) == prefix) {
                    if (directories_only && !entry.is_directory()) {
                        continue;
                    }

                    std::string fullPath = basePath + filename;
                    if (entry.is_directory()) {
                        fullPath += "/";
                    }
                    completions.push_back(fullPath);
                }
            }
        }
    }
    catch (const std::exception&) {
        // 忽略错误
    }

    std::sort(completions.begin(), completions.end());
    return completions;
}

void TerminalUtils::showCurrentDirectoryContents(bool directories_only)
{
    try {
        fs::path currentPath = fs::current_path();
        printColor(_("directory.current") + ": " + currentPath.string(), Color::BRIGHT_CYAN,
                   TextStyle::BOLD);
        std::cout << std::endl;

        showDirectoryContents(currentPath.string(), directories_only);
    }
    catch (const std::exception& e) {
        printError(_("error.cannot_read_directory") + ": " + std::string(e.what()));
    }
}

void TerminalUtils::showDirectoryContents(const std::string& dirPath, bool directories_only)
{
    try {
        fs::path dir(dirPath);
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            printError(_("error.directory_not_exist") + ": " + dirPath);
            return;
        }

        std::vector<std::string> dirs, files;

        for (const auto& entry : fs::directory_iterator(dir)) {
            std::string name = entry.path().filename().string();

            // 跳过隐藏文件
            if (name.front() == '.') {
                continue;
            }

            if (entry.is_directory()) {
                dirs.push_back(name + "/");
            } else if (!directories_only) {
                files.push_back(name);
            }
        }

        // 显示目录
        if (!dirs.empty()) {
            printColor(_("directory.folders"), Color::BRIGHT_BLUE, TextStyle::BOLD);
            std::cout << std::endl;
            for (const auto& d : dirs) {
                printColor("  " + d, Color::BRIGHT_BLUE);
                std::cout << std::endl;
            }
        }

        // 显示文件
        if (!files.empty() && !directories_only) {
            printColor(_("directory.files"), Color::BRIGHT_GREEN, TextStyle::BOLD);
            std::cout << std::endl;
            for (const auto& f : files) {
                std::cout << "  " << f << std::endl;
            }
        }

        std::cout << std::endl;
    }
    catch (const std::exception& e) {
        printError(_("error.cannot_read_dir") + ": " + std::string(e.what()));
    }
}

void TerminalUtils::showPathInputHelp()
{
    printColor(_("help.path_input_title"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;
    std::cout << "  - " << _("help.path_input_enter") << std::endl;
    std::cout << "  - " << _("help.path_input_use_path") << std::endl;
    std::cout << "  - " << _("help.path_input_browse_dir") << std::endl;
    std::cout << "  - " << _("help.path_input_quit") << std::endl;
    std::cout << "  - " << _("help.path_input_help") << std::endl;
    std::cout << std::endl;
}

void TerminalUtils::showCompletions(const std::vector<std::string>& completions,
                                    const std::string& base_path)
{
    // Not used in simplified implementation
}

#ifdef _WIN32
std::string TerminalUtils::handleWindowsInput(const std::string& prompt, bool directories_only)
{
    return "";
}
#else
std::string TerminalUtils::handleUnixInput(const std::string& prompt, bool directories_only)
{
    return "";
}
#endif

// 实现findCommonPrefix函数
std::string TerminalUtils::findCommonPrefix(const std::vector<std::string>& completions)
{
    if (completions.empty()) {
        return "";
    }

    if (completions.size() == 1) {
        return completions[0];
    }

    std::string prefix = completions[0];
    for (size_t i = 1; i < completions.size(); ++i) {
        size_t j = 0;
        while (j < prefix.length() && j < completions[i].length() &&
               prefix[j] == completions[i][j]) {
            ++j;
        }
        prefix = prefix.substr(0, j);
        if (prefix.empty()) {
            break;
        }
    }

    return prefix;
}

}}  // namespace neumann::cli