#pragma once

#include <chrono>
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
    std::string colorText(const std::string& text, Color color,
                          TextStyle style = TextStyle::NORMAL);

    /**
     * @brief 打印彩色文本
     * @param text 文本内容
     * @param color 文本颜色
     * @param style 文本样式
     */
    void printColor(const std::string& text, Color color, TextStyle style = TextStyle::NORMAL);

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
    void showSpinner(const std::string& message, int durationMs = 1000);

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
     * @brief 格式化表格行
     * @param columns 列内容
     * @param widths 列宽度
     * @param alignment 对齐方式（'l'=左对齐, 'r'=右对齐, 'c'=居中）
     * @return 格式化的行字符串
     */
    std::string formatTableRow(const std::vector<std::string>& columns,
                               const std::vector<int>& widths, const std::string& alignment = "l");

    /**
     * @brief 获取终端宽度
     * @return 终端宽度（字符数）
     */
    int getTerminalWidth();

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
};

// 便利宏定义
#define PRINT_SUCCESS(msg) neumann::cli::TerminalUtils::getInstance().printSuccess(msg)
#define PRINT_WARNING(msg) neumann::cli::TerminalUtils::getInstance().printWarning(msg)
#define PRINT_ERROR(msg) neumann::cli::TerminalUtils::getInstance().printError(msg)
#define PRINT_INFO(msg) neumann::cli::TerminalUtils::getInstance().printInfo(msg)

}}  // namespace neumann::cli