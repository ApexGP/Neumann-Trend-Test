#pragma once

#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace neumann { namespace cli {

/**
 * @brief 颜色代码枚举
 */
enum class Color {
    RESET = 0,
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,
    BRIGHT_BLACK = 90,
    BRIGHT_RED = 91,
    BRIGHT_GREEN = 92,
    BRIGHT_YELLOW = 93,
    BRIGHT_BLUE = 94,
    BRIGHT_MAGENTA = 95,
    BRIGHT_CYAN = 96,
    BRIGHT_WHITE = 97
};

/**
 * @brief 文本样式枚举
 */
enum class TextStyle {
    NORMAL = 0,
    BOLD = 1,
    DIM = 2,
    ITALIC = 3,
    UNDERLINE = 4,
    BLINK = 5,
    REVERSE = 7,
    STRIKETHROUGH = 9
};

/**
 * @brief 终端工具类
 * 
 * 提供彩色输出、进度指示器、表格格式化等功能
 */
class TerminalUtils
{
public:
    /**
     * @brief 获取TerminalUtils单例实例
     * @return TerminalUtils的共享实例
     */
    static TerminalUtils& getInstance();

    /**
     * @brief 设置是否启用彩色输出
     * @param enabled 是否启用彩色输出
     */
    void setColorEnabled(bool enabled);

    /**
     * @brief 检查是否启用了彩色输出
     * @return 是否启用彩色输出
     */
    bool isColorEnabled() const;

    /**
     * @brief 获取彩色文本
     * @param text 文本内容
     * @param color 文本颜色
     * @param style 文本样式
     * @return 格式化后的文本
     */
    std::string colorText(const std::string& text, Color color = Color::WHITE,
                          TextStyle style = TextStyle::NORMAL);

    /**
     * @brief 打印彩色文本
     * @param text 文本内容
     * @param color 文本颜色
     * @param style 文本样式
     */
    void printColor(const std::string& text, Color color = Color::WHITE,
                    TextStyle style = TextStyle::NORMAL);

    /**
     * @brief 打印成功消息
     * @param message 消息内容
     */
    void printSuccess(const std::string& message);

    /**
     * @brief 打印警告消息
     * @param message 消息内容
     */
    void printWarning(const std::string& message);

    /**
     * @brief 打印错误消息
     * @param message 消息内容
     */
    void printError(const std::string& message);

    /**
     * @brief 打印信息消息
     * @param message 消息内容
     */
    void printInfo(const std::string& message);

    /**
     * @brief 显示进度条
     * @param current 当前进度
     * @param total 总进度
     * @param prefix 前缀文本
     * @param width 进度条宽度
     */
    void showProgress(int current, int total, const std::string& prefix = "", int width = 50);

    /**
     * @brief 显示旋转加载指示器
     * @param message 加载消息
     * @param durationMs 持续时间（毫秒）
     */
    void showSpinner(const std::string& message, int durationMs = 2000);

    /**
     * @brief 清空当前行
     */
    void clearCurrentLine();

    /**
     * @brief 移动光标到行首
     */
    void moveToLineStart();

    /**
     * @brief 隐藏光标
     */
    void hideCursor();

    /**
     * @brief 显示光标
     */
    void showCursor();

    /**
     * @brief 创建表格分隔线
     * @param width 表格宽度
     * @param char 分隔符字符
     * @return 分隔线字符串
     */
    std::string createTableSeparator(int width, char separator = '-');

    /**
     * @brief 计算字符串的显示宽度（考虑中文字符占用2个字符宽度）
     * @param str 输入字符串
     * @return 显示宽度
     */
    int getDisplayWidth(const std::string& str);

    /**
     * @brief 格式化表格行
     * @param columns 列内容
     * @param widths 列宽度
     * @param alignment 对齐方式（'l'=左对齐, 'r'=右对齐, 'c'=居中）
     * @return 格式化的行字符串
     */
    std::string formatTableRow(const std::vector<std::string>& columns,
                               const std::vector<int>& widths,
                               const std::string& alignment = "llllll");

    /**
     * @brief 获取终端宽度
     * @return 终端宽度（字符数）
     */
    int getTerminalWidth();

    // 文件路径输入（支持tab补全）
    std::string promptForFilePath(const std::string& prompt, bool directories_only = false);
    std::string promptForDirectory(const std::string& prompt);

    // 带真正tab补全的文件路径输入
    std::string promptForFilePathWithTabCompletion(const std::string& prompt,
                                                   bool directories_only = false);

private:
    // 私有构造函数
    TerminalUtils();

    // 禁用拷贝和赋值
    TerminalUtils(const TerminalUtils&) = delete;
    TerminalUtils& operator=(const TerminalUtils&) = delete;

    // 是否启用彩色输出
    bool colorEnabled;

    // 获取ANSI控制序列
    std::string getAnsiCode(Color color, TextStyle style = TextStyle::NORMAL);

    // 检测终端是否支持彩色输出
    bool detectColorSupport();

    // tab补全相关
    std::vector<std::string> getFileCompletions(const std::string& partial_path,
                                                bool directories_only = false);
    std::string findCommonPrefix(const std::vector<std::string>& completions);
    void showCompletions(const std::vector<std::string>& completions, const std::string& base_path);

    // 简化版本的辅助方法
    void showCurrentDirectoryContents(bool directories_only = false);
    void showDirectoryContents(const std::string& dirPath, bool directories_only = false);
    void showPathInputHelp();

    // 平台特定的输入处理
#ifdef _WIN32
    std::string handleWindowsInput(const std::string& prompt, bool directories_only);
#else
    std::string handleUnixInput(const std::string& prompt, bool directories_only);
#endif
};

// 便利宏定义
#define PRINT_SUCCESS(msg) neumann::cli::TerminalUtils::getInstance().printSuccess(msg)
#define PRINT_WARNING(msg) neumann::cli::TerminalUtils::getInstance().printWarning(msg)
#define PRINT_ERROR(msg) neumann::cli::TerminalUtils::getInstance().printError(msg)
#define PRINT_INFO(msg) neumann::cli::TerminalUtils::getInstance().printInfo(msg)

}}  // namespace neumann::cli