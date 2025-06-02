#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

// 确保max_align_t定义可用
#ifdef _WIN32
#include <malloc.h>

#include <cstdlib>

#else
#include <cstdlib>
#endif

#include "core/data_manager.h"
#include "core/neumann_calculator.h"

// 前向声明Web服务器类
namespace neumann { namespace web {
class WebServer;
}}  // namespace neumann::web

namespace neumann { namespace cli {

// 菜单项目类型
enum class MenuItemType {
    ACTION,   // 执行动作
    SUBMENU,  // 子菜单
    BACK,     // 返回上一级
    EXIT      // 退出程序
};

// 菜单项目结构
struct MenuItem {
    std::string id;                // 唯一标识符
    std::string title;             // 显示标题
    MenuItemType type;             // 菜单项类型
    std::string subMenuId;         // 子菜单ID（如果type是SUBMENU）
    std::function<void()> action;  // 执行的动作（如果type是ACTION）
};

// 菜单结构
struct Menu {
    std::string id;               // 唯一标识符
    std::string title;            // 菜单标题
    std::vector<MenuItem> items;  // 菜单项目
};

/**
 * @brief 终端UI类
 *
 * 实现交互式命令行界面，提供菜单驱动的用户交互
 */
class TerminalUI
{
public:
    /**
   * @brief 构造函数
   */
    TerminalUI();

    /**
   * @brief 析构函数
   */
    ~TerminalUI();

    /**
   * @brief 启动UI
   */
    void run();

private:
    // 菜单映射
    std::map<std::string, Menu> menus;

    // 当前活动菜单
    std::string currentMenuId;

    // 上一级菜单栈
    std::vector<std::string> menuStack;

    // 程序是否继续运行
    bool running;

    // Web服务器实例
    std::unique_ptr<neumann::web::WebServer> webServer;

    // 初始化菜单
    void initializeMenus();

    // 显示当前菜单
    void displayMenu();

    // 处理用户输入
    void handleInput();

    // 清屏
    void clearScreen();

    // 加载和导入数据集
    void loadDataSet();

    // 从CSV导入数据
    void importFromCSV();

    // 从Excel导入数据
    void importFromExcel();

    // 加载样本数据文件
    void loadSampleData();

    // 运行诺依曼测试
    void runNeumannTest();

    // 启动Web服务器
    void startWebServer();

    // 自动启动Web服务器并打开浏览器
    void autoStartWebServerAndBrowser();

    // 显示Web服务器运行界面
    void showWebServerRunningInterface();

    // 显示帮助信息
    void showHelp();

    // 显示关于信息
    void showAbout();

    // 显示语言选择菜单
    void showLanguageMenu();

    // 显示置信度配置菜单
    void showConfidenceLevelMenu();

    // 显示右上角状态信息
    void displayStatusBar();

    // 批量处理功能
    void runBatchProcessing();

    // 高级功能菜单
    void showDataVisualization();
    void showStatisticsAnalysis();

    // SVG 查看器
    void showSVGViewer();

    // 询问用户输入数据（传统方式）
    std::vector<double> promptForData(const std::string &prompt);

    // 询问用户输入时间点
    std::vector<double> promptForTimePoints(const std::string &prompt, size_t count);

    // 现代化数据输入（Excel风格）
    std::pair<std::vector<double>, std::vector<double>> promptForDataModern(
        const std::string &prompt);

    // 显示测试结果
    void displayTestResults(const NeumannTestResults &results);

    /**
     * @brief 管理自定义置信度水平
     */
    void manageCustomConfidenceLevels();

    /**
     * @brief 使用现代化文件浏览器导入CSV文件
     */
    void importFromCSVModern();

    /**
     * @brief 使用现代化文件浏览器导入Excel文件
     */
    void importFromExcelModern();
};

}}  // namespace neumann::cli