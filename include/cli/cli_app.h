#pragma once

#include <string>

#include "cli/terminal_ui.h"

namespace neumann { namespace cli {

/**
 * @brief CLI应用程序类
 *
 * 命令行应用程序入口点
 */
class CLIApp
{
public:
    /**
   * @brief 构造函数
   */
    CLIApp();

    /**
   * @brief 运行应用程序
   * @param argc 命令行参数数量
   * @param argv 命令行参数数组
   * @return 退出代码
   */
    int run(int argc, char **argv);

private:
    // 终端UI实例
    TerminalUI ui;

    // 处理命令行参数
    bool processArgs(int argc, char **argv);

    // 显示帮助信息
    void showHelp();

    // 显示版本信息
    void showVersion();

    // 直接运行数据处理
    void runWithData(const std::string &dataFile);
};

}}  // namespace neumann::cli