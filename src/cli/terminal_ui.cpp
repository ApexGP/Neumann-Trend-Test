#include "cli/terminal_ui.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include "cli/terminal_utils.h"
#include "core/batch_processor.h"
#include "core/config.h"
#include "core/data_visualization.h"
#include "core/excel_reader.h"
#include "core/i18n.h"
#include "core/standard_values.h"

// Web服务器头文件放在最后，避免与其他头文件冲突
#include "web/web_server.h"

namespace fs = std::filesystem;

namespace neumann { namespace cli {

// C++17兼容的字符串后缀检查函数
static bool endsWith(const std::string &str, const std::string &suffix)
{
    if (suffix.length() > str.length()) {
        return false;
    }
    return str.substr(str.length() - suffix.length()) == suffix;
}

TerminalUI::TerminalUI() : currentMenuId("main"), running(false), webServer(nullptr)
{
    initializeMenus();
}

TerminalUI::~TerminalUI()
{
    // 确保Web服务器在析构时停止
    if (webServer && webServer->isRunning()) {
        webServer->stop();
    }
}

void TerminalUI::run()
{
    running = true;

    auto &termUtils = TerminalUtils::getInstance();

    // 显示欢迎信息
    clearScreen();

    termUtils.printColor("=====================================", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl;
    termUtils.printColor("  " + _("app.title"), Color::BRIGHT_GREEN, TextStyle::BOLD);
    std::cout << std::endl;
    termUtils.printColor("=====================================", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl << std::endl;

    // 加载标准值
    termUtils.showSpinner(_("status.loading") + " standard values...", 500);
    if (!StandardValues::getInstance().loadFromFile("data/standard_values.json")) {
        termUtils.printWarning(_("error.standard_values_not_found"));
    } else {
        termUtils.printSuccess("Standard values loaded successfully");
    }

    // 主循环
    while (running) {
        displayMenu();
        handleInput();
    }

    // 显示退出信息
    std::cout << std::endl;
    termUtils.printInfo(_("status.goodbye"));
}

void TerminalUI::initializeMenus()
{
    // 主菜单
    Menu mainMenu;
    mainMenu.id = "main";
    mainMenu.title = "menu.main";  // 存储翻译键而不是翻译文本
    mainMenu.items = {
        {"new_test", "menu.new_test", MenuItemType::ACTION, "", [this]() { runNeumannTest(); }},
        {"load_data", "menu.load_data", MenuItemType::ACTION, "", [this]() { loadDataSet(); }},
        {"import_csv", "menu.import_csv", MenuItemType::ACTION, "", [this]() { importFromCSV(); }},
        {"import_excel", "menu.import_excel", MenuItemType::ACTION, "",
         [this]() { importFromExcel(); }},
        {"load_sample", "menu.load_sample", MenuItemType::ACTION, "",
         [this]() { loadSampleData(); }},
        {"batch_process", "menu.batch_process", MenuItemType::ACTION, "",
         [this]() { runBatchProcessing(); }},
        {"start_web", "menu.start_web", MenuItemType::ACTION, "", [this]() { startWebServer(); }},
        {"advanced", "menu.advanced", MenuItemType::SUBMENU, "advanced", nullptr},
        {"settings", "menu.settings", MenuItemType::SUBMENU, "settings", nullptr},
        {"help", "menu.help", MenuItemType::ACTION, "", [this]() { showHelp(); }},
        {"about", "menu.about", MenuItemType::ACTION, "", [this]() { showAbout(); }},
        {"exit", "menu.exit", MenuItemType::EXIT, "", nullptr}};
    menus[mainMenu.id] = mainMenu;

    // 设置菜单
    Menu settingsMenu;
    settingsMenu.id = "settings";
    settingsMenu.title = "menu.settings";  // 存储翻译键而不是翻译文本
    settingsMenu.items = {
        {"language", "menu.language", MenuItemType::ACTION, "", [this]() { showLanguageMenu(); }},
        {"confidence", "menu.confidence", MenuItemType::ACTION, "",
         [this]() { showConfidenceLevelMenu(); }},
        {"back", "menu.back", MenuItemType::BACK, "", nullptr}};
    menus[settingsMenu.id] = settingsMenu;

    // 高级功能菜单
    Menu advancedMenu;
    advancedMenu.id = "advanced";
    advancedMenu.title = "menu.advanced";
    advancedMenu.items = {{"visualization", "menu.visualization", MenuItemType::ACTION, "",
                           [this]() { showDataVisualization(); }},
                          {"statistics", "menu.statistics", MenuItemType::ACTION, "",
                           [this]() { showStatisticsAnalysis(); }},
                          {"back", "menu.back", MenuItemType::BACK, "", nullptr}};
    menus[advancedMenu.id] = advancedMenu;
}

void TerminalUI::displayMenu()
{
    clearScreen();

    // 显示右上角状态栏
    displayStatusBar();

    // 获取当前菜单
    const Menu &menu = menus[currentMenuId];

    // 动态获取菜单标题的翻译文本
    std::cout << "===== " << _(menu.title.c_str()) << " =====" << std::endl;
    std::cout << std::endl;

    // 显示菜单项，动态获取翻译文本
    for (size_t i = 0; i < menu.items.size(); ++i) {
        // 使用固定宽度格式化编号，确保对齐（支持最多99个菜单项）
        std::cout << std::setw(2) << std::right << (i + 1) << ". " << _(menu.items[i].title.c_str())
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-" << menu.items.size() << "]: ";
}

void TerminalUI::handleInput()
{
    int choice;
    std::cin >> choice;

    // 清除输入缓冲区
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const Menu &menu = menus[currentMenuId];

    if (choice < 1 || choice > static_cast<int>(menu.items.size())) {
        std::cout << _("error.invalid_choice") << std::endl;
        std::cout << _("prompt.press_enter") << std::endl;
        std::cin.get();
        return;
    }

    const MenuItem &item = menu.items[choice - 1];

    switch (item.type) {
        case MenuItemType::ACTION:
            if (item.action) {
                item.action();
            }
            break;

        case MenuItemType::SUBMENU:
            menuStack.push_back(currentMenuId);
            currentMenuId = item.subMenuId;
            break;

        case MenuItemType::BACK:
            if (!menuStack.empty()) {
                currentMenuId = menuStack.back();
                menuStack.pop_back();
            }
            break;

        case MenuItemType::EXIT:
            running = false;
            break;
    }
}

void TerminalUI::clearScreen()
{
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

std::vector<double> TerminalUI::promptForData(const std::string &prompt)
{
    std::vector<double> data;
    std::string input;

    std::cout << prompt << " (" << _("prompt.separator_help") << ", " << _("input.exit_hint")
              << "): ";
    std::getline(std::cin, input);

    // 检查是否要退出
    if (input.empty() || input == "q" || input == "Q" || input == "quit" || input == "exit") {
        return data;  // 返回空向量表示退出
    }

    std::stringstream ss(input);
    std::string token;

    // 处理可能的分隔符（空格或逗号）
    while (std::getline(ss, token, ',')) {
        std::stringstream tokenStream(token);
        double value;

        while (tokenStream >> value) {
            data.push_back(value);
        }
    }

    return data;
}

std::vector<double> TerminalUI::promptForTimePoints(const std::string &prompt, size_t count)
{
    std::vector<double> timePoints;

    // 询问用户是否要使用默认时间点
    std::cout << prompt << " [y/n]: ";
    std::string response;
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        // 使用默认时间点 (0, 1, 2, ...)
        for (size_t i = 0; i < count; ++i) {
            timePoints.push_back(static_cast<double>(i));
        }
    } else {
        // 询问自定义时间点
        timePoints = promptForData(_("input.time_points"));

        // 确保时间点数量与数据点一致
        if (timePoints.size() != count) {
            std::cout << _("prompt.timepoint_mismatch_warning") << std::endl;
            timePoints.clear();
            for (size_t i = 0; i < count; ++i) {
                timePoints.push_back(static_cast<double>(i));
            }
        }
    }

    return timePoints;
}

void TerminalUI::loadDataSet()
{
    clearScreen();
    std::cout << "===== " << _("menu.load_data") << " =====" << std::endl;
    std::cout << std::endl;

    // 获取可用数据集
    std::vector<std::string> datasets = DataManager::getInstance().getDataSetNames();

    if (datasets.empty()) {
        std::cout << _("load.no_datasets") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 显示数据集列表
    std::cout << _("load.available_datasets") << std::endl;
    for (size_t i = 0; i < datasets.size(); ++i) {
        std::cout << (i + 1) << ". " << datasets[i] << std::endl;
    }
    std::cout << "0. " << _("menu.back") << " (" << _("input.exit_hint") << ")" << std::endl;

    // 选择数据集
    std::cout << std::endl;
    std::cout << _("load.select_dataset") << " [0-" << datasets.size() << "]: ";
    int choice;
    std::cin >> choice;

    // 清除输入缓冲区
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 检查退出选项
    if (choice == 0) {
        return;
    }

    if (choice < 1 || choice > static_cast<int>(datasets.size())) {
        std::cout << _("error.invalid_choice") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 加载选择的数据集
    std::string datasetName = datasets[choice - 1];
    DataSet dataSet = DataManager::getInstance().loadDataSet(datasetName);

    // 显示数据集信息
    std::cout << std::endl;
    std::cout << _("load.dataset_name") << ": " << dataSet.name << std::endl;
    std::cout << _("load.dataset_description") << ": " << dataSet.description << std::endl;
    std::cout << _("load.dataset_source") << ": " << dataSet.source << std::endl;
    std::cout << _("load.dataset_created") << ": " << dataSet.createdAt << std::endl;
    std::cout << _("load.dataset_count") << ": " << dataSet.dataPoints.size() << std::endl;

    // 询问是否运行测试
    std::cout << std::endl;
    std::cout << _("load.run_test_prompt") << " [y/n]: ";
    std::string response;
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        // 创建计算器并运行测试
        NeumannCalculator calculator;
        NeumannTestResults results = calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

        // 显示测试结果
        displayTestResults(results);
    }

    std::cout << _("prompt.press_enter");
    std::cin.get();
}

void TerminalUI::importFromCSV()
{
    clearScreen();
    std::cout << "===== " << _("menu.import_csv") << " =====" << std::endl;
    std::cout << std::endl;

    // 获取CSV文件路径
    std::cout << _("input.filename") << " (" << _("input.exit_hint") << "): ";
    std::string filePath;
    std::getline(std::cin, filePath);

    // 检查是否要退出
    if (filePath.empty() || filePath == "q" || filePath == "Q" || filePath == "quit" ||
        filePath == "exit") {
        return;
    }

    if (!fs::exists(filePath)) {
        std::cout << _("error.file_not_found") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 询问是否有表头
    std::cout << _("input.has_header") << " [y/n]: ";
    std::string response;
    std::getline(std::cin, response);
    bool hasHeader = !response.empty() && std::tolower(response[0]) == 'y';

    // 导入数据
    DataSet dataSet = DataManager::getInstance().importFromCSV(filePath, hasHeader);

    // 显示导入信息
    std::cout << std::endl;
    std::cout << _("import.success") << ": " << dataSet.name << std::endl;
    std::cout << _("import.data_count") << ": " << dataSet.dataPoints.size() << std::endl;

    // 询问是否保存
    std::cout << std::endl;
    std::cout << _("import.save_prompt") << " [y/n]: ";
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        // 可以修改数据集名称
        std::cout << _("import.dataset_name") << " [" << dataSet.name << "]: ";
        std::string name;
        std::getline(std::cin, name);

        if (!name.empty()) {
            dataSet.name = name;
        }

        // 可以添加描述
        std::cout << _("import.dataset_description") << ": ";
        std::getline(std::cin, dataSet.description);

        // 保存数据集
        if (DataManager::getInstance().saveDataSet(dataSet)) {
            std::cout << _("import.dataset_saved") << std::endl;
        } else {
            std::cout << _("import.dataset_save_failed") << std::endl;
        }
    }

    // 询问是否运行测试
    std::cout << std::endl;
    std::cout << _("import.run_test_prompt") << " [y/n]: ";
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        // 创建计算器并运行测试
        NeumannCalculator calculator;
        NeumannTestResults results = calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

        // 显示测试结果
        displayTestResults(results);
    }

    std::cout << _("prompt.press_enter");
    std::cin.get();
}

void TerminalUI::importFromExcel()
{
    clearScreen();
    std::cout << "===== " << _("menu.import_excel") << " =====" << std::endl;
    std::cout << std::endl;

    // 获取Excel文件路径
    std::cout << _("input.filename") << " (" << _("input.exit_hint") << "): ";
    std::string filePath;
    std::getline(std::cin, filePath);

    // 检查是否要退出
    if (filePath.empty() || filePath == "q" || filePath == "Q" || filePath == "quit" ||
        filePath == "exit") {
        return;
    }

    if (!fs::exists(filePath)) {
        std::cout << _("error.file_not_found") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    auto &termUtils = TerminalUtils::getInstance();

    // 检查文件类型
    if (!ExcelReader::isExcelFile(filePath)) {
        termUtils.printError(_("excel.unsupported_format"));
        termUtils.printInfo("请将Excel文件转换为CSV格式后重试");
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    termUtils.printInfo(_("excel.file_detected"));

    // 创建Excel读取器
    ExcelReader reader;

    try {
        // 如果是真正的Excel文件，获取工作表列表
        std::string lowerPath = filePath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
        if ((lowerPath.length() >= 5 && lowerPath.substr(lowerPath.length() - 5) == ".xlsx") ||
            (lowerPath.length() >= 4 && lowerPath.substr(lowerPath.length() - 4) == ".xls")) {
            termUtils.printError(_("excel.unsupported_format"));
            termUtils.printInfo("请将Excel文件转换为CSV格式后重试");
            std::cout << _("prompt.press_enter");
            std::cin.get();
            return;
        }

        // 对于CSV文件，直接进行处理
        termUtils.showSpinner(_("progress.importing") + "...", 1000);

        // 询问是否有表头
        std::cout << _("input.has_header") << " [y/n]: ";
        std::string response;
        std::getline(std::cin, response);
        bool hasHeader = !response.empty() && std::tolower(response[0]) == 'y';

        // 数据预览
        auto previewData = reader.previewExcelData(filePath, "", 5);
        if (!previewData.empty()) {
            termUtils.printColor(_("input.file_preview"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
            std::cout << std::endl;

            // 显示前几行数据
            for (size_t i = 0; i < std::min(size_t(5), previewData.size()); ++i) {
                for (size_t j = 0; j < previewData[i].size(); ++j) {
                    std::cout << std::setw(12) << previewData[i][j];
                    if (j < previewData[i].size() - 1) {
                        std::cout << " | ";
                    }
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }

        // 导入数据
        DataSet dataSet = reader.importFromExcel(filePath, "", hasHeader);

        // 显示导入信息
        std::cout << std::endl;
        termUtils.printSuccess(_("excel.import_success"));
        std::cout << _("import.data_count") << ": " << dataSet.dataPoints.size() << std::endl;

        // 自动检测的列信息
        if (!previewData.empty()) {
            auto columnTypes = reader.detectColumnTypes(previewData);
            termUtils.printColor(_("excel.column_detection"), Color::BRIGHT_CYAN);
            std::cout << std::endl;
            for (const auto &[colIndex, type] : columnTypes) {
                std::cout << "  " << _("excel.data_column") << " " << (colIndex + 1) << ": " << type
                          << std::endl;
            }
            std::cout << std::endl;
        }

        // 询问是否保存
        std::cout << _("import.save_prompt") << " [y/n]: ";
        std::getline(std::cin, response);

        if (!response.empty() && std::tolower(response[0]) == 'y') {
            // 可以修改数据集名称
            std::cout << _("import.dataset_name") << " [" << dataSet.name << "]: ";
            std::string name;
            std::getline(std::cin, name);

            if (!name.empty()) {
                dataSet.name = name;
            }

            // 可以添加描述
            std::cout << _("import.dataset_description") << ": ";
            std::getline(std::cin, dataSet.description);

            // 保存数据集
            if (DataManager::getInstance().saveDataSet(dataSet)) {
                termUtils.printSuccess(_("import.dataset_saved"));
            } else {
                termUtils.printError(_("import.dataset_save_failed"));
            }
        }

        // 询问是否运行测试
        std::cout << std::endl;
        std::cout << _("import.run_test_prompt") << " [y/n]: ";
        std::getline(std::cin, response);

        if (!response.empty() && std::tolower(response[0]) == 'y') {
            // 创建计算器并运行测试
            NeumannCalculator calculator;
            NeumannTestResults results =
                calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

            // 显示测试结果
            displayTestResults(results);
        }
    }
    catch (const std::exception &e) {
        termUtils.printError("导入失败: " + std::string(e.what()));
    }

    std::cout << _("prompt.press_enter");
    std::cin.get();
}

void TerminalUI::runNeumannTest()
{
    clearScreen();
    std::cout << "===== " << _("menu.new_test") << " =====" << std::endl;
    std::cout << std::endl;

    // 询问数据点
    std::vector<double> dataPoints = promptForData(_("input.data_points"));

    // 检查是否用户选择退出
    if (dataPoints.empty()) {
        return;
    }

    if (dataPoints.size() < 4) {
        std::cout << _("error.insufficient_data") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 询问时间点
    std::vector<double> timePoints =
        promptForTimePoints(_("test.use_default_timepoints"), dataPoints.size());

    // 使用配置中的置信水平
    auto &config = Config::getInstance();
    double confidenceLevel = config.getDefaultConfidenceLevel();

    std::cout << _("test.using_confidence_level") << ": " << std::fixed << std::setprecision(2)
              << (confidenceLevel * 100) << "%" << std::endl;
    std::cout << _("test.change_in_settings") << std::endl;
    std::cout << std::endl;

    // 创建计算器并运行测试
    NeumannCalculator calculator(confidenceLevel);
    NeumannTestResults results = calculator.performTest(dataPoints, timePoints);

    // 显示测试结果
    displayTestResults(results);

    // 询问是否保存数据
    std::cout << std::endl;
    std::cout << _("test.save_dataset_prompt") << " [y/n]: ";
    std::string response;
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        DataSet dataSet;
        dataSet.dataPoints = dataPoints;
        dataSet.timePoints = timePoints;

        // 询问数据集名称
        std::cout << _("test.dataset_name_prompt") << ": ";
        std::getline(std::cin, dataSet.name);

        if (dataSet.name.empty()) {
            dataSet.name = "dataset_" + std::to_string(std::time(nullptr));
        }

        // 询问描述
        std::cout << _("test.dataset_description_prompt") << ": ";
        std::getline(std::cin, dataSet.description);

        // 设置来源
        dataSet.source = _("test.manual_input");

        // 设置创建时间
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
        dataSet.createdAt = ss.str();

        // 保存数据集
        if (DataManager::getInstance().saveDataSet(dataSet)) {
            std::cout << _("test.dataset_saved") << std::endl;
        } else {
            std::cout << _("test.dataset_save_failed") << std::endl;
        }
    }

    std::cout << _("prompt.press_enter");
    std::cin.get();
}

void TerminalUI::displayTestResults(const NeumannTestResults &results)
{
    clearScreen();
    auto &termUtils = TerminalUtils::getInstance();

    // 标题
    termUtils.printColor("===== " + _("result.test_results") + " =====", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl << std::endl;

    if (results.results.empty()) {
        termUtils.printError(_("result.test_failed"));
        return;
    }

    // 表头
    std::vector<std::string> headers = {_("result.data_point"), _("result.time_point"),
                                        _("result.pg_value"), _("result.threshold"),
                                        _("result.trend_judgment")};

    // 动态计算列宽 - 考虑表头文本长度和数据内容
    std::vector<int> columnWidths(headers.size());

    // 基于表头文本长度计算初始列宽
    for (size_t i = 0; i < headers.size(); ++i) {
        columnWidths[i] = termUtils.getDisplayWidth(headers[i]);
    }

    // 考虑数据内容，确保列宽足够显示数据
    for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

        // 格式化数据以计算所需宽度
        std::ostringstream dataPointStr, timePointStr, pgValueStr, thresholdStr;

        dataPointStr << std::fixed << std::setprecision(2) << results.data[dataIndex];
        timePointStr << std::fixed << std::setprecision(2) << results.timePoints[dataIndex];
        pgValueStr << std::fixed << std::setprecision(4) << results.results[i].pgValue;
        thresholdStr << std::fixed << std::setprecision(4) << results.results[i].wpThreshold;

        std::string trendText =
            results.results[i].hasTrend ? _("result.has_trend") : _("result.no_trend");

        // 更新列宽以容纳数据内容
        columnWidths[0] = std::max(columnWidths[0], termUtils.getDisplayWidth(dataPointStr.str()));
        columnWidths[1] = std::max(columnWidths[1], termUtils.getDisplayWidth(timePointStr.str()));
        columnWidths[2] = std::max(columnWidths[2], termUtils.getDisplayWidth(pgValueStr.str()));
        columnWidths[3] = std::max(columnWidths[3], termUtils.getDisplayWidth(thresholdStr.str()));
        columnWidths[4] = std::max(columnWidths[4], termUtils.getDisplayWidth(trendText));
    }

    // 设置最小列宽，并添加一些填充空间
    const std::vector<int> minWidths = {8, 8, 10, 12, 10};  // 最小列宽
    const std::vector<int> padding = {2, 2, 2, 2, 2};       // 每列的填充空间

    for (size_t i = 0; i < columnWidths.size(); ++i) {
        columnWidths[i] = std::max(columnWidths[i] + padding[i], minWidths[i]);
    }

    // 显示表头 - 使用混合对齐：数值列右对齐，文本列左对齐
    termUtils.printColor(termUtils.formatTableRow(headers, columnWidths, "lrrrr"),
                         Color::BRIGHT_WHITE, TextStyle::BOLD);
    std::cout << std::endl;

    // 计算实际表格宽度：列宽总和 + 间隔
    int tableWidth = 0;
    for (int width : columnWidths) {
        tableWidth += width;
    }
    tableWidth += (columnWidths.size() - 1);  // 添加列间隔

    // 分隔线
    termUtils.printColor(termUtils.createTableSeparator(tableWidth, '='), Color::CYAN);
    std::cout << std::endl;

    // 显示数据行 - 改进数据格式化
    for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

        // 格式化数据以适应列宽
        std::ostringstream dataPointStr, timePointStr, pgValueStr, thresholdStr;

        dataPointStr << std::fixed << std::setprecision(2) << results.data[dataIndex];
        timePointStr << std::fixed << std::setprecision(2) << results.timePoints[dataIndex];
        pgValueStr << std::fixed << std::setprecision(4) << results.results[i].pgValue;
        thresholdStr << std::fixed << std::setprecision(4) << results.results[i].wpThreshold;

        std::vector<std::string> row = {
            dataPointStr.str(), timePointStr.str(), pgValueStr.str(), thresholdStr.str(),
            results.results[i].hasTrend ? _("result.has_trend") : _("result.no_trend")};

        // 根据趋势判断使用不同颜色 - 使用混合对齐与表头一致
        Color rowColor = results.results[i].hasTrend ? Color::BRIGHT_RED : Color::BRIGHT_GREEN;
        termUtils.printColor(termUtils.formatTableRow(row, columnWidths, "lrrrr"), rowColor);
        std::cout << std::endl;
    }

    // 分隔线
    termUtils.printColor(termUtils.createTableSeparator(tableWidth, '='), Color::CYAN);
    std::cout << std::endl << std::endl;

    // 添加ASCII图表显示
    std::string asciiChart = DataVisualization::generateASCIIChart(results);
    std::cout << asciiChart << std::endl;

    // 汇总结果 - 完整的闭合边框
    int summaryWidth = 70;  // 增加边框宽度以适应内容

    // 计算有趋势的数据点数量和百分比
    int trendPointsCount = 0;
    for (const auto &result : results.results) {
        if (result.hasTrend) trendPointsCount++;
    }
    double trendPercentage = (100.0 * trendPointsCount) / results.results.size();

    // 顶部边框
    std::string topBorder = "┌─── " + _("result.summary") + " ";
    int topTitleLength = termUtils.getDisplayWidth("┌─── " + _("result.summary") + " ");
    for (int i = topTitleLength; i < summaryWidth - 1 + 5; ++i) {
        topBorder += "─";
    }
    topBorder += "┐";

    termUtils.printColor(topBorder, Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // 整体趋势结果行
    termUtils.printColor("│ ", Color::BRIGHT_CYAN, TextStyle::BOLD);
    termUtils.printColor(_("result.overall_trend") + ": ", Color::BRIGHT_WHITE, TextStyle::BOLD);
    if (results.overallTrend) {
        termUtils.printColor("⚠ " + _("result.has_trend") + " ⚠", Color::BRIGHT_RED,
                             TextStyle::BOLD);
    } else {
        termUtils.printColor("✓ " + _("result.no_trend") + " ✓", Color::BRIGHT_GREEN,
                             TextStyle::BOLD);
    }
    // 计算需要填充的空格数
    std::string overallContent = _("result.overall_trend") + ": " +
                                 (results.overallTrend ? ("⚠ " + _("result.has_trend") + " ⚠")
                                                       : ("✓ " + _("result.no_trend") + " ✓"));
    int overallContentWidth = termUtils.getDisplayWidth(overallContent);
    int overallPadding = summaryWidth - overallContentWidth;
    for (int i = 0; i < overallPadding; ++i) {
        std::cout << " ";
    }
    termUtils.printColor("│", Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // 状态描述行
    termUtils.printColor("│ ", Color::BRIGHT_CYAN, TextStyle::BOLD);
    if (results.overallTrend) {
        termUtils.printColor("📈 " + _("result.trend_detected"), Color::BRIGHT_RED);
    } else {
        termUtils.printColor("📊 " + _("result.data_stable"), Color::BRIGHT_GREEN);
    }
    // 计算填充空格
    std::string statusContent = (results.overallTrend ? "📈 " + _("result.trend_detected")
                                                      : "📊 " + _("result.data_stable"));
    int statusContentWidth = termUtils.getDisplayWidth(statusContent);
    int statusPadding = summaryWidth - statusContentWidth - 2;
    for (int i = 0; i < statusPadding; ++i) {
        std::cout << " ";
    }
    termUtils.printColor("│", Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // 趋势点统计行
    termUtils.printColor("│ ", Color::BRIGHT_CYAN, TextStyle::BOLD);
    termUtils.printColor("🔍 " + _("result.trend_statistics") + ": ", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << trendPointsCount << "/" << results.results.size() << " (";

    Color percentageColor = (trendPercentage > 50)   ? Color::BRIGHT_RED
                            : (trendPercentage > 20) ? Color::YELLOW
                                                     : Color::BRIGHT_GREEN;
    termUtils.printColor(std::to_string((int) trendPercentage) + "%", percentageColor,
                         TextStyle::BOLD);
    std::cout << ")";
    // 计算填充空格
    std::string statsContent = "🔍 " + _("result.trend_statistics") + ": " +
                               std::to_string(trendPointsCount) + "/" +
                               std::to_string(results.results.size()) + " (" +
                               std::to_string((int) trendPercentage) + "%)";
    int statsContentWidth = termUtils.getDisplayWidth(statsContent);
    int statsPadding = summaryWidth - statsContentWidth - 2;
    for (int i = 0; i < statsPadding; ++i) {
        std::cout << " ";
    }
    termUtils.printColor("│", Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // PG值范围行
    termUtils.printColor("│ ", Color::BRIGHT_CYAN, TextStyle::BOLD);
    termUtils.printColor("📊 " + _("result.pg_range") + ": ", Color::BRIGHT_CYAN, TextStyle::BOLD);

    // 最小PG值
    Color minColor = (results.minPG < 1.0) ? Color::BRIGHT_RED : Color::BRIGHT_GREEN;
    termUtils.printColor(std::to_string(results.minPG).substr(0, 6), minColor, TextStyle::BOLD);
    std::cout << " ~ ";

    // 最大PG值
    Color maxColor = (results.maxPG < 1.0) ? Color::BRIGHT_RED : Color::BRIGHT_GREEN;
    termUtils.printColor(std::to_string(results.maxPG).substr(0, 6), maxColor, TextStyle::BOLD);
    // 计算填充空格
    std::string rangeContent = "📊 " + _("result.pg_range") + ": " +
                               std::to_string(results.minPG).substr(0, 6) + " ~ " +
                               std::to_string(results.maxPG).substr(0, 6);
    int rangeContentWidth = termUtils.getDisplayWidth(rangeContent);
    int rangePadding = summaryWidth - rangeContentWidth - 2;
    for (int i = 0; i < rangePadding; ++i) {
        std::cout << " ";
    }
    termUtils.printColor("│", Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // 平均PG值行
    termUtils.printColor("│ ", Color::BRIGHT_CYAN, TextStyle::BOLD);
    termUtils.printColor("📈 " + _("result.avg_pg_label") + ": ", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    Color avgColor = (results.avgPG < 1.0) ? Color::BRIGHT_RED : Color::BRIGHT_GREEN;
    termUtils.printColor(std::to_string(results.avgPG).substr(0, 6), avgColor, TextStyle::BOLD);
    // 计算填充空格
    std::string avgContent =
        "📈 " + _("result.avg_pg_label") + ": " + std::to_string(results.avgPG).substr(0, 6);
    int avgContentWidth = termUtils.getDisplayWidth(avgContent);
    int avgPadding = summaryWidth - avgContentWidth - 2;
    for (int i = 0; i < avgPadding; ++i) {
        std::cout << " ";
    }
    termUtils.printColor("│", Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // PG值解释行
    termUtils.printColor("│ ", Color::BRIGHT_CYAN, TextStyle::BOLD);
    termUtils.printColor("💡 ", Color::BRIGHT_YELLOW);
    std::string interpretationText;
    if (results.overallTrend) {
        termUtils.printColor(_("result.pg_interpretation_trend"), Color::YELLOW);
        interpretationText = "💡 " + _("result.pg_interpretation_trend");
    } else {
        termUtils.printColor(_("result.pg_interpretation_stable"), Color::GREEN);
        interpretationText = "💡 " + _("result.pg_interpretation_stable");
    }
    // 计算填充空格
    int interpretationWidth = termUtils.getDisplayWidth(interpretationText);
    if (results.overallTrend) {
        int interpretationPadding = summaryWidth - interpretationWidth - 2;
        for (int i = 0; i < interpretationPadding; ++i) {
            std::cout << " ";
        }
    } else {
        int interpretationPadding = summaryWidth - interpretationWidth - 2;
        for (int i = 0; i < interpretationPadding; ++i) {
            std::cout << " ";
        }
    }
    termUtils.printColor("│", Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;

    // 底部边框
    std::string bottomBorder = "└";
    for (int i = 0; i < summaryWidth - 1; ++i) {
        bottomBorder += "─";
    }
    bottomBorder += "┘";

    termUtils.printColor(bottomBorder, Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl << std::endl;

    // 结论部分 - 完整的闭合边框设计
    int conclusionWidth = 125;  // 固定列宽

    // 显示结论标题
    if (results.overallTrend) {
        termUtils.printColor("🔴 " + _("result.conclusion"), Color::BRIGHT_RED, TextStyle::BOLD);
    } else {
        termUtils.printColor("🟢 " + _("result.conclusion"), Color::BRIGHT_GREEN, TextStyle::BOLD);
    }
    std::cout << std::endl;

    // 构建顶部边框
    std::string conclusionTopBorder, conclusionBottomBorder;
    Color borderColor = results.overallTrend ? Color::BRIGHT_RED : Color::BRIGHT_GREEN;

    if (results.overallTrend) {
        conclusionTopBorder = "┌─── " + _("result.trend_warning") + " ";
        int titleLength = termUtils.getDisplayWidth("┌─── " + _("result.trend_warning") + " ");
        for (int i = titleLength; i < conclusionWidth + 3; ++i) {
            conclusionTopBorder += "─";
        }
        conclusionTopBorder += "┐";
    } else {
        conclusionTopBorder = "┌─── " + _("result.stability_confirmed") + " ";
        int titleLength =
            termUtils.getDisplayWidth("┌─── " + _("result.stability_confirmed") + " ");
        for (int i = titleLength; i < conclusionWidth + 3; ++i) {
            conclusionTopBorder += "─";
        }
        conclusionTopBorder += "┐";
    }

    // 构建底部边框
    conclusionBottomBorder = "└";
    for (int i = 0; i < conclusionWidth - 2; ++i) {
        conclusionBottomBorder += "─";
    }
    conclusionBottomBorder += "┘";

    // 显示顶部边框
    termUtils.printColor(conclusionTopBorder, borderColor, TextStyle::BOLD);
    std::cout << std::endl;

    // 显示结论内容
    if (results.overallTrend) {
        termUtils.printColor("│ ", Color::BRIGHT_RED, TextStyle::BOLD);
    } else {
        termUtils.printColor("│ ", Color::BRIGHT_GREEN, TextStyle::BOLD);
    }
    std::string conclusionText;
    Color textColor;

    if (results.overallTrend) {
        conclusionText = _("result.conclusion_trend");
        textColor = Color::YELLOW;
    } else {
        conclusionText = _("result.conclusion_no_trend");
        textColor = Color::GREEN;
    }

    termUtils.printColor(conclusionText, textColor);

    // 计算需要填充的空格数以保持边框对齐
    int contentWidth = termUtils.getDisplayWidth(conclusionText);
    int conclusionPadding = conclusionWidth - contentWidth - 3;  // 3 for "│ " and "│"
    for (int i = 0; i < conclusionPadding; ++i) {
        std::cout << " ";
    }
    if (results.overallTrend) {
        termUtils.printColor("│", Color::BRIGHT_RED, TextStyle::BOLD);
    } else {
        termUtils.printColor("│", Color::BRIGHT_GREEN, TextStyle::BOLD);
    }
    std::cout << std::endl;

    // 显示底部边框
    termUtils.printColor(conclusionBottomBorder, borderColor, TextStyle::BOLD);
    std::cout << std::endl;
}

void TerminalUI::showHelp()
{
    clearScreen();
    auto &termUtils = TerminalUtils::getInstance();

    termUtils.printColor("===== " + _("help.title") + " =====", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl << std::endl;

    // 程序描述
    termUtils.printColor(_("help.description"), Color::BRIGHT_WHITE, TextStyle::BOLD);
    std::cout << std::endl;
    std::cout << _("help.usage_areas") << std::endl;
    std::cout << std::endl;

    // 主要功能
    termUtils.printColor(_("help.main_features"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;

    termUtils.printColor("1. " + _("help.feature_basic_test"), Color::BRIGHT_GREEN);
    std::cout << std::endl;
    std::cout << "   • " << _("help.feature_basic_test_desc") << std::endl;

    termUtils.printColor("2. " + _("help.feature_data_import"), Color::BRIGHT_GREEN);
    std::cout << std::endl;
    std::cout << "   • " << _("help.feature_import_csv") << std::endl;
    std::cout << "   • " << _("help.feature_import_excel") << std::endl;
    std::cout << "   • " << _("help.feature_load_dataset") << std::endl;

    termUtils.printColor("3. " + _("help.feature_batch_processing"), Color::BRIGHT_GREEN);
    std::cout << std::endl;
    std::cout << "   • " << _("help.feature_batch_directory") << std::endl;
    std::cout << "   • " << _("help.feature_batch_files") << std::endl;
    std::cout << "   • " << _("help.feature_batch_export") << std::endl;

    termUtils.printColor("4. " + _("help.feature_visualization"), Color::BRIGHT_GREEN);
    std::cout << std::endl;
    std::cout << "   • " << _("help.feature_ascii_charts") << std::endl;
    std::cout << "   • " << _("help.feature_svg_export") << std::endl;

    termUtils.printColor("5. " + _("help.feature_statistics"), Color::BRIGHT_GREEN);
    std::cout << std::endl;
    std::cout << "   • " << _("help.feature_multi_dataset") << std::endl;
    std::cout << "   • " << _("help.feature_trend_analysis") << std::endl;

    termUtils.printColor("6. " + _("help.feature_configuration"), Color::BRIGHT_GREEN);
    std::cout << std::endl;
    std::cout << "   • " << _("help.feature_language") << std::endl;
    std::cout << "   • " << _("help.feature_confidence") << std::endl;
    std::cout << std::endl;

    // 使用指南
    termUtils.printColor(_("help.usage_guide"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;
    std::cout << _("help.guide_1") << std::endl;
    std::cout << _("help.guide_2") << std::endl;
    std::cout << _("help.guide_3") << std::endl;
    std::cout << _("help.guide_4") << std::endl;
    std::cout << _("help.guide_5") << std::endl;
    std::cout << std::endl;

    // 重要提示
    termUtils.printColor(_("help.important_notes"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;
    std::cout << "• " << _("help.note_1") << std::endl;
    std::cout << "• " << _("help.note_2") << std::endl;
    std::cout << "• " << _("help.note_3") << std::endl;
    std::cout << "• " << _("help.note_4") << std::endl;
    std::cout << "• " << _("help.note_5") << std::endl;
    std::cout << std::endl;

    // 快捷操作提示
    termUtils.printColor(_("help.quick_tips"), Color::BRIGHT_CYAN, TextStyle::BOLD);
    std::cout << std::endl;
    std::cout << "• " << _("help.tip_confidence") << std::endl;
    std::cout << "• " << _("help.tip_batch") << std::endl;
    std::cout << "• " << _("help.tip_visualization") << std::endl;
    std::cout << "• " << _("help.tip_export") << std::endl;

    std::cout << std::endl;
    termUtils.printColor(_("prompt.press_enter"), Color::BRIGHT_WHITE);
    std::cin.get();
}

void TerminalUI::showAbout()
{
    clearScreen();
    auto &termUtils = TerminalUtils::getInstance();

    termUtils.printColor("===== " + _("about.title") + " =====", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl << std::endl;

    // 程序信息
    termUtils.printColor(_("app.title"), Color::BRIGHT_GREEN, TextStyle::BOLD);
    std::cout << " v2.2.0" << std::endl;
    std::cout << "Copyright © 2025" << std::endl;
    std::cout << std::endl;

    // 程序描述
    std::cout << _("about.description") << std::endl;
    std::cout << _("about.applications") << std::endl;
    std::cout << std::endl;

    // 功能亮点
    termUtils.printColor(_("about.features"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;
    std::cout << "✓ " << _("about.feature_multilang") << std::endl;
    std::cout << "✓ " << _("about.feature_batch") << std::endl;
    std::cout << "✓ " << _("about.feature_visualization") << std::endl;
    std::cout << "✓ " << _("about.feature_statistics") << std::endl;
    std::cout << "✓ " << _("about.feature_export") << std::endl;
    std::cout << "✓ " << _("about.feature_config") << std::endl;
    std::cout << std::endl;

    // 技术信息
    termUtils.printColor(_("about.technical_info"), Color::BRIGHT_CYAN);
    std::cout << std::endl;
    std::cout << "• " << _("about.tech_cpp17") << std::endl;
    std::cout << "• " << _("about.tech_cross_platform") << std::endl;
    std::cout << "• " << _("about.tech_formats") << std::endl;
    std::cout << "• " << _("about.tech_performance") << std::endl;

    std::cout << std::endl;
    termUtils.printColor(_("prompt.press_enter"), Color::BRIGHT_WHITE);
    std::cin.get();
}

void TerminalUI::showLanguageMenu()
{
    clearScreen();
    std::cout << "===== " << _("menu.language") << " =====" << std::endl;
    std::cout << std::endl;

    auto &i18n = I18n::getInstance();
    std::cout << _("prompt.current_language") << ": "
              << (i18n.getCurrentLanguage() == Language::CHINESE ? "中文" : "English") << std::endl;
    std::cout << std::endl;

    std::cout << "1. 中文 (Chinese)" << std::endl;
    std::cout << "2. English" << std::endl;
    std::cout << "3. " << _("menu.back") << std::endl;
    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-3]: ";

    int choice;
    std::cin >> choice;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
        case 1:
            i18n.setLanguage(Language::CHINESE);
            Config::getInstance().setLanguage(Language::CHINESE);
            if (Config::getInstance().saveToFile(Config::getInstance().getConfigFilePath())) {
                std::cout << "语言已设置为中文" << std::endl;
            } else {
                std::cout << "语言设置保存失败" << std::endl;
            }
            std::cout << "按Enter键继续..." << std::endl;
            std::cin.get();
            // 返回主菜单
            if (!menuStack.empty()) {
                currentMenuId = menuStack.back();
                menuStack.pop_back();
            }
            break;
        case 2:
            i18n.setLanguage(Language::ENGLISH);
            Config::getInstance().setLanguage(Language::ENGLISH);
            if (Config::getInstance().saveToFile(Config::getInstance().getConfigFilePath())) {
                std::cout << "Language set to English" << std::endl;
            } else {
                std::cout << "Failed to save language settings" << std::endl;
            }
            std::cout << "Press Enter to continue..." << std::endl;
            std::cin.get();
            // 返回主菜单
            if (!menuStack.empty()) {
                currentMenuId = menuStack.back();
                menuStack.pop_back();
            }
            break;
        case 3:
            return;
        default:
            std::cout << _("error.invalid_choice") << std::endl;
            std::cout << _("prompt.press_enter") << std::endl;
            std::cin.get();
            break;
    }
}

void TerminalUI::showConfidenceLevelMenu()
{
    clearScreen();
    std::cout << "===== " << _("menu.confidence") << " =====" << std::endl;
    std::cout << std::endl;

    auto &config = Config::getInstance();
    auto &standardValues = StandardValues::getInstance();

    std::cout << _("prompt.current_confidence_level") << ": " << config.getDefaultConfidenceLevel()
              << std::endl;
    std::cout << std::endl;

    // 获取支持的置信度列表
    auto supportedLevels = standardValues.getSupportedConfidenceLevels();

    // 显示置信度选项
    for (size_t i = 0; i < supportedLevels.size(); ++i) {
        double level = supportedLevels[i];
        std::cout << (i + 1) << ". " << std::fixed << std::setprecision(3) << level;

        // 显示百分比
        std::cout << " (" << std::fixed << std::setprecision(1) << (level * 100) << "%)";

        // 标记默认值
        if (std::abs(level - 0.95) < 0.001) {
            std::cout << " - " << _("menu.default");
        }

        std::cout << std::endl;
    }

    std::cout << (supportedLevels.size() + 1) << ". " << _("menu.custom") << std::endl;
    std::cout << (supportedLevels.size() + 2) << ". " << _("menu.manage_custom") << std::endl;
    std::cout << (supportedLevels.size() + 3) << ". " << _("menu.back") << std::endl;
    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-" << (supportedLevels.size() + 3) << "]: ";

    int choice;
    std::cin >> choice;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    double newLevel = 0.95;  // 默认值
    bool levelChanged = false;

    if (choice >= 1 && choice <= static_cast<int>(supportedLevels.size())) {
        // 选择了预设置信度
        newLevel = supportedLevels[choice - 1];
        levelChanged = true;
    } else if (choice == static_cast<int>(supportedLevels.size()) + 1) {
        // 自定义置信度 - 改为选项式交互
        std::cout << std::endl;
        std::cout << "===== " << _("menu.custom_confidence") << " =====" << std::endl;
        std::cout << _("custom.confidence_explanation") << std::endl;
        std::cout << std::endl;

        // 显示自定义置信度选项
        std::cout << _("custom.select_import_method") << std::endl;
        std::cout << "1. " << _("custom.manual_input") << std::endl;
        std::cout << "2. " << _("custom.load_example_file") << std::endl;
        std::cout << "3. " << _("menu.back") << std::endl;
        std::cout << std::endl;
        std::cout << _("prompt.select_option") << " [1-3]: ";

        int methodChoice;
        std::cin >> methodChoice;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        double customLevel = 0.0;
        std::string filePath;
        bool isExampleFile = false;

        switch (methodChoice) {
            case 1: {
                // 选项1：手动输入自定义置信度
                std::cout << std::endl;
                std::cout << "===== " << _("custom.manual_input") << " =====" << std::endl;
                std::cout << _("custom.file_format_info") << std::endl;
                std::cout << std::endl;

                // 输入置信度值
                std::cout << _("prompt.enter_confidence_level") << " (0.0-1.0): ";
                std::string input;
                std::getline(std::cin, input);

                try {
                    customLevel = std::stod(input);
                    if (customLevel <= 0.0 || customLevel >= 1.0) {
                        std::cout << _("error.invalid_confidence_level") << std::endl;
                        std::cout << _("prompt.press_enter") << std::endl;
                        std::cin.get();
                        return;
                    }
                }
                catch (const std::exception &e) {
                    std::cout << _("error.invalid_input") << std::endl;
                    std::cout << _("prompt.press_enter") << std::endl;
                    std::cin.get();
                    return;
                }

                // 输入标准值表文件路径
                std::cout << std::endl;
                std::cout << _("custom.enter_file_path") << " (.json/.csv): ";
                std::getline(std::cin, filePath);

                if (filePath.empty()) {
                    return;
                }
                break;
            }
            case 2: {
                // 选项2：从示例文件加载
                std::cout << std::endl;
                std::cout << "===== " << _("custom.load_example_file") << " =====" << std::endl;
                std::cout << std::endl;

                // 扫描示例文件目录
                std::string exampleDir = "data/sample/ConfidenceLevel";
                std::vector<std::string> exampleFiles;

                if (!fs::exists(exampleDir)) {
                    std::cout << _("custom.example_dir_not_found") << ": " << exampleDir
                              << std::endl;
                    std::cout << _("prompt.press_enter") << std::endl;
                    std::cin.get();
                    return;
                }

                try {
                    for (const auto &entry : fs::directory_iterator(exampleDir)) {
                        if (entry.is_regular_file()) {
                            std::string filename = entry.path().filename().string();
                            std::string extension = entry.path().extension().string();
                            std::transform(extension.begin(), extension.end(), extension.begin(),
                                           ::tolower);

                            if (extension == ".json" || extension == ".csv") {
                                exampleFiles.push_back(entry.path().string());
                            }
                        }
                    }
                }
                catch (const std::exception &e) {
                    std::cout << _("custom.scan_example_error") << ": " << e.what() << std::endl;
                    std::cout << _("prompt.press_enter") << std::endl;
                    std::cin.get();
                    return;
                }

                if (exampleFiles.empty()) {
                    std::cout << _("custom.no_example_files") << std::endl;
                    std::cout << _("prompt.press_enter") << std::endl;
                    std::cin.get();
                    return;
                }

                // 显示可用的示例文件
                std::cout << _("custom.available_example_files") << std::endl;
                for (size_t i = 0; i < exampleFiles.size(); ++i) {
                    fs::path filePath(exampleFiles[i]);
                    std::string filename = filePath.filename().string();
                    std::string extension = filePath.extension().string();

                    std::cout << (i + 1) << ". " << filename << " (" << extension << " "
                              << _("custom.format") << ")";

                    // 尝试从文件名推断置信度
                    if (filename.find("90") != std::string::npos) {
                        std::cout << " - 90% " << _("custom.confidence_level");
                    }
                    std::cout << std::endl;
                }
                std::cout << (exampleFiles.size() + 1) << ". " << _("menu.back") << std::endl;
                std::cout << std::endl;
                std::cout << _("prompt.select_option") << " [1-" << (exampleFiles.size() + 1)
                          << "]: ";

                int fileChoice;
                std::cin >> fileChoice;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (fileChoice == static_cast<int>(exampleFiles.size()) + 1) {
                    return;  // 返回
                }

                if (fileChoice < 1 || fileChoice > static_cast<int>(exampleFiles.size())) {
                    std::cout << _("error.invalid_choice") << std::endl;
                    std::cout << _("prompt.press_enter") << std::endl;
                    std::cin.get();
                    return;
                }

                filePath = exampleFiles[fileChoice - 1];
                isExampleFile = true;

                // 从文件名推断置信度，或让用户输入
                fs::path selectedFile(filePath);
                std::string filename = selectedFile.filename().string();

                if (filename.find("90") != std::string::npos) {
                    customLevel = 0.90;
                    std::cout << std::endl;
                    std::cout << _("custom.detected_confidence_level") << ": 90%" << std::endl;
                } else {
                    std::cout << std::endl;
                    std::cout << _("custom.enter_confidence_for_file") << " " << filename
                              << std::endl;
                    std::cout << _("prompt.enter_confidence_level") << " (0.0-1.0): ";
                    std::string input;
                    std::getline(std::cin, input);

                    try {
                        customLevel = std::stod(input);
                        if (customLevel <= 0.0 || customLevel >= 1.0) {
                            std::cout << _("error.invalid_confidence_level") << std::endl;
                            std::cout << _("prompt.press_enter") << std::endl;
                            std::cin.get();
                            return;
                        }
                    }
                    catch (const std::exception &e) {
                        std::cout << _("error.invalid_input") << std::endl;
                        std::cout << _("prompt.press_enter") << std::endl;
                        std::cin.get();
                        return;
                    }
                }

                std::cout << _("custom.using_example_file") << ": " << filename << std::endl;
                std::cout << _("custom.example_confidence_level") << ": " << std::fixed
                          << std::setprecision(1) << (customLevel * 100) << "%" << std::endl;
                break;
            }
            case 3:
                return;  // 返回
            default:
                std::cout << _("error.invalid_choice") << std::endl;
                std::cout << _("prompt.press_enter") << std::endl;
                std::cin.get();
                return;
        }

        // 现在检查是否已存在该置信度（在确定了最终的customLevel之后）
        auto existingLevels = standardValues.getSupportedConfidenceLevels();
        bool alreadyExists = false;

        for (double level : existingLevels) {
            if (std::abs(level - customLevel) < 0.001) {
                alreadyExists = true;
                break;
            }
        }

        if (alreadyExists) {
            if (isExampleFile) {
                // 示例文件自动覆盖，给出提示
                std::cout << std::endl;
                std::cout << _("custom.example_file_overwrite") << ": " << std::fixed
                          << std::setprecision(1) << (customLevel * 100) << "%" << std::endl;
            } else {
                // 用户文件需要确认
                std::cout << std::endl;
                std::cout << _("warning.confidence_level_exists") << ": " << customLevel
                          << std::endl;
                std::cout << _("custom.overwrite_prompt") << " [y/n]: ";
                std::string response;
                std::getline(std::cin, response);
                if (response.empty() || std::tolower(response[0]) != 'y') {
                    return;
                }
            }
        }

        // 检查文件是否存在
        if (!fs::exists(filePath)) {
            std::cout << _("error.file_not_found") << ": " << filePath << std::endl;
            std::cout << _("prompt.press_enter") << std::endl;
            std::cin.get();
            return;
        }

        // 显示文件格式示例（仅对非示例文件显示）
        if (!isExampleFile) {
            std::cout << std::endl;
            std::cout << _("custom.file_format_example") << std::endl;
            std::cout << "JSON " << _("custom.format") << ":" << std::endl;
            std::cout << "  {" << std::endl;
            std::cout << "    \"4\": 0.7805," << std::endl;
            std::cout << "    \"5\": 0.8204," << std::endl;
            std::cout << "    \"6\": 0.8902," << std::endl;
            std::cout << "    ..." << std::endl;
            std::cout << "  }" << std::endl;
            std::cout << std::endl;
            std::cout << "CSV " << _("custom.format") << ":" << std::endl;
            std::cout << "  n,wp_value" << std::endl;
            std::cout << "  4,0.7805" << std::endl;
            std::cout << "  5,0.8204" << std::endl;
            std::cout << "  6,0.8902" << std::endl;
            std::cout << "  ..." << std::endl;
            std::cout << std::endl;

            std::cout << _("custom.confirm_import") << " [y/n]: ";
            std::string confirmResponse;
            std::getline(std::cin, confirmResponse);

            if (confirmResponse.empty() || std::tolower(confirmResponse[0]) != 'y') {
                return;
            }
        }

        // 尝试导入自定义标准值表
        std::cout << std::endl;
        std::cout << _("custom.importing") << "..." << std::endl;

        if (standardValues.importCustomConfidenceLevel(customLevel, filePath)) {
            newLevel = customLevel;
            levelChanged = true;
            std::cout << _("custom.import_success") << std::endl;
        } else {
            std::cout << _("custom.import_failed") << std::endl;
            std::cout << _("custom.check_file_format") << std::endl;
        }

        std::cout << _("prompt.press_enter") << std::endl;
        std::cin.get();
    } else if (choice == static_cast<int>(supportedLevels.size()) + 2) {
        // 管理自定义置信度
        manageCustomConfidenceLevels();
        return;
    } else if (choice == static_cast<int>(supportedLevels.size()) + 3) {
        // 返回
        return;
    } else {
        std::cout << _("error.invalid_choice") << std::endl;
        std::cout << _("prompt.press_enter") << std::endl;
        std::cin.get();
        return;
    }

    if (levelChanged) {
        config.setDefaultConfidenceLevel(newLevel);

        // 尝试保存配置
        std::string configFile = config.getConfigFilePath();
        if (config.saveToFile(configFile)) {
            std::cout << _("status.confidence_level_saved") << ": " << newLevel << std::endl;
        } else {
            std::cout << _("warning.config_save_failed") << std::endl;
        }

        std::cout << _("prompt.press_enter") << std::endl;
        std::cin.get();
    }
}

void TerminalUI::displayStatusBar()
{
    auto &termUtils = TerminalUtils::getInstance();
    auto &config = Config::getInstance();

    // 获取程序标题和置信度信息
    std::string appTitle = _("app.title");
    double confidenceLevel = config.getDefaultConfidenceLevel();

    // 格式化置信度显示
    std::ostringstream confidenceText;
    confidenceText << _("status.confidence") << std::fixed << std::setprecision(2)
                   << (confidenceLevel * 100) << "%";

    // 获取终端宽度（假设80字符，可以后续改进为动态获取）
    int terminalWidth = 80;
    int appTitleWidth = appTitle.length();
    int confidenceWidth = confidenceText.str().length();

    // 计算中间需要的空格数
    int remainingSpace = terminalWidth - appTitleWidth - confidenceWidth;
    if (remainingSpace < 0) remainingSpace = 1;  // 至少保留1个空格

    // 显示状态栏：左边标题 + 中间空格 + 右边置信度
    termUtils.printColor(appTitle, Color::BRIGHT_GREEN, TextStyle::BOLD);
    std::cout << std::string(remainingSpace, ' ');
    termUtils.printColor(confidenceText.str(), Color::BRIGHT_BLUE, TextStyle::BOLD);
    std::cout << std::endl;

    // 添加分隔线
    termUtils.printColor(std::string(terminalWidth, '-'), Color::CYAN);
    std::cout << std::endl << std::endl;
}

void TerminalUI::runBatchProcessing()
{
    clearScreen();
    std::cout << "===== " << _("menu.batch_process") << " =====" << std::endl;
    std::cout << std::endl;

    // 询问处理模式
    std::cout << _("batch.select_mode") << std::endl;
    std::cout << "1. " << _("batch.process_directory") << std::endl;
    std::cout << "2. " << _("batch.process_files") << std::endl;
    std::cout << "3. " << _("menu.back") << std::endl;
    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-3]: ";

    int choice;
    std::cin >> choice;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::vector<BatchProcessResult> results;
    auto &config = Config::getInstance();
    BatchProcessor processor(config.getDefaultConfidenceLevel());

    switch (choice) {
        case 1: {
            // 处理目录
            std::cout << _("batch.enter_directory") << ": ";
            std::string directory;
            std::getline(std::cin, directory);

            if (directory.empty()) {
                return;
            }

            std::cout << std::endl;
            std::cout << _("batch.processing") << "..." << std::endl;

            // 进度回调
            auto progressCallback = [](int current, int total, const std::string &filename) {
                std::cout << "\r" << _("batch.progress") << ": " << current << "/" << total << " - "
                          << filename << std::flush;
            };

            results = processor.processDirectory(directory, progressCallback);
            std::cout << std::endl;
            break;
        }
        case 2: {
            // 处理指定文件
            std::cout << _("batch.enter_files") << " (" << _("prompt.separator_help") << "): ";
            std::string filesInput;
            std::getline(std::cin, filesInput);

            if (filesInput.empty()) {
                return;
            }

            // 解析文件列表
            std::vector<std::string> files;
            std::stringstream ss(filesInput);
            std::string file;
            while (std::getline(ss, file, ',')) {
                // 去除前后空格
                file.erase(0, file.find_first_not_of(" \t"));
                file.erase(file.find_last_not_of(" \t") + 1);
                if (!file.empty()) {
                    files.push_back(file);
                }
            }

            std::cout << std::endl;
            std::cout << _("batch.processing") << "..." << std::endl;

            auto progressCallback = [](int current, int total, const std::string &filename) {
                std::cout << "\r" << _("batch.progress") << ": " << current << "/" << total << " - "
                          << filename << std::flush;
            };

            results = processor.processFiles(files, progressCallback);
            std::cout << std::endl;
            break;
        }
        case 3:
            return;
        default:
            std::cout << _("error.invalid_choice") << std::endl;
            std::cout << _("prompt.press_enter") << std::endl;
            std::cin.get();
            return;
    }

    // 显示处理结果
    if (results.empty()) {
        std::cout << _("batch.no_files_processed") << std::endl;
    } else {
        BatchProcessStats stats = BatchProcessor::generateStatistics(results);

        std::cout << std::endl;
        std::cout << "===== " << _("batch.results_summary") << " =====" << std::endl;
        std::cout << _("batch.total_files") << ": " << stats.totalFiles << std::endl;
        std::cout << _("batch.successful_files") << ": " << stats.successfulFiles << std::endl;
        std::cout << _("batch.error_files") << ": " << stats.errorFiles << std::endl;
        std::cout << _("batch.files_with_trend") << ": " << stats.filesWithTrend << std::endl;
        std::cout << _("batch.total_processing_time") << ": " << std::fixed << std::setprecision(2)
                  << stats.totalProcessingTime << "s" << std::endl;

        // 询问是否保存结果
        std::cout << std::endl;
        std::cout << _("batch.save_results_prompt") << " [y/n]: ";
        std::string response;
        std::getline(std::cin, response);

        if (!response.empty() && std::tolower(response[0]) == 'y') {
            std::cout << _("batch.select_format") << std::endl;
            std::cout << "1. CSV" << std::endl;
            std::cout << "2. HTML" << std::endl;
            std::cout << _("prompt.select_option") << " [1-2]: ";

            int formatChoice;
            std::cin >> formatChoice;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << _("batch.enter_output_filename") << ": ";
            std::string filename;
            std::getline(std::cin, filename);

            if (!filename.empty()) {
                bool success = false;
                if (formatChoice == 1) {
                    if (!endsWith(filename, ".csv")) {
                        filename += ".csv";
                    }
                    success = BatchProcessor::exportResultsToCSV(results, filename);
                } else if (formatChoice == 2) {
                    if (!endsWith(filename, ".html")) {
                        filename += ".html";
                    }
                    success = BatchProcessor::exportResultsToHTML(results, filename);
                }

                if (success) {
                    std::cout << _("batch.results_saved") << ": " << filename << std::endl;
                } else {
                    std::cout << _("batch.save_failed") << std::endl;
                }
            }
        }
    }

    std::cout << std::endl;
    std::cout << _("prompt.press_enter") << std::endl;
    std::cin.get();
}

void TerminalUI::showDataVisualization()
{
    clearScreen();
    std::cout << "===== " << _("menu.visualization") << " =====" << std::endl;
    std::cout << std::endl;

    // 获取可用数据集
    std::vector<std::string> datasets = DataManager::getInstance().getDataSetNames();

    if (datasets.empty()) {
        std::cout << _("load.no_datasets") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 显示数据集列表
    std::cout << _("visualization.select_dataset") << std::endl;
    for (size_t i = 0; i < datasets.size(); ++i) {
        std::cout << (i + 1) << ". " << datasets[i] << std::endl;
    }
    std::cout << "0. " << _("menu.back") << std::endl;

    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [0-" << datasets.size() << "]: ";
    int choice;
    std::cin >> choice;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 0 || choice < 1 || choice > static_cast<int>(datasets.size())) {
        return;
    }

    // 加载数据集并执行测试
    std::string datasetName = datasets[choice - 1];
    DataSet dataSet = DataManager::getInstance().loadDataSet(datasetName);

    auto &config = Config::getInstance();
    NeumannCalculator calculator(config.getDefaultConfidenceLevel());
    NeumannTestResults results = calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

    // 显示ASCII图表
    std::cout << std::endl;
    std::string asciiChart = DataVisualization::generateASCIIChart(results);
    std::cout << asciiChart << std::endl;

    // 询问是否保存SVG图表
    std::cout << _("visualization.save_svg_prompt") << " [y/n]: ";
    std::string response;
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        std::cout << _("visualization.enter_filename") << " (.svg): ";
        std::string filename;
        std::getline(std::cin, filename);

        if (!filename.empty()) {
            if (!endsWith(filename, ".svg")) {
                filename += ".svg";
            }

            // 确保SVG输出目录存在
            std::string svgDir = "data/svg";
            if (!fs::exists(svgDir)) {
                try {
                    fs::create_directories(svgDir);
                }
                catch (const std::exception &e) {
                    std::cout << _("visualization.save_failed") << ": " << e.what() << std::endl;
                    std::cout << std::endl;
                    std::cout << _("prompt.press_enter") << std::endl;
                    std::cin.get();
                    return;
                }
            }

            // 构建完整的文件路径
            std::string fullPath = svgDir + "/" + filename;

            std::string svgChart = DataVisualization::generateTrendChart(results);
            if (DataVisualization::saveChartToFile(svgChart, fullPath)) {
                std::cout << _("visualization.chart_saved") << ": " << fullPath << std::endl;
            } else {
                std::cout << _("visualization.save_failed") << std::endl;
            }
        }
    }

    std::cout << std::endl;
    std::cout << _("prompt.press_enter") << std::endl;
    std::cin.get();
}

void TerminalUI::showStatisticsAnalysis()
{
    clearScreen();
    std::cout << "===== " << _("menu.statistics") << " =====" << std::endl;
    std::cout << std::endl;

    // 获取可用数据集
    std::vector<std::string> datasets = DataManager::getInstance().getDataSetNames();

    if (datasets.empty()) {
        std::cout << _("load.no_datasets") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    std::cout << _("statistics.analyzing_all_datasets") << std::endl;
    std::cout << std::endl;

    auto &config = Config::getInstance();
    int totalDatasets = 0;
    int datasetsWithTrend = 0;
    double totalDataPoints = 0;
    double totalPGSum = 0;
    double minOverallPG = std::numeric_limits<double>::max();
    double maxOverallPG = std::numeric_limits<double>::min();

    // 分析所有数据集
    for (const auto &datasetName : datasets) {
        try {
            DataSet dataSet = DataManager::getInstance().loadDataSet(datasetName);
            if (dataSet.dataPoints.size() >= 4) {
                NeumannCalculator calculator(config.getDefaultConfidenceLevel());
                NeumannTestResults results =
                    calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

                totalDatasets++;
                if (results.overallTrend) {
                    datasetsWithTrend++;
                }

                totalDataPoints += dataSet.dataPoints.size();
                totalPGSum += results.avgPG;
                minOverallPG = std::min(minOverallPG, results.minPG);
                maxOverallPG = std::max(maxOverallPG, results.maxPG);

                std::cout << "✓ " << datasetName << " (" << dataSet.dataPoints.size()
                          << " points, trend: " << (results.overallTrend ? "YES" : "NO") << ")"
                          << std::endl;
            }
        }
        catch (const std::exception &e) {
            std::cout << "✗ " << datasetName << " (Error: " << e.what() << ")" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "===== " << _("statistics.overall_summary") << " =====" << std::endl;
    std::cout << _("statistics.total_datasets") << ": " << totalDatasets << std::endl;
    std::cout << _("statistics.datasets_with_trend") << ": " << datasetsWithTrend << " ("
              << std::fixed << std::setprecision(1)
              << (totalDatasets > 0 ? (datasetsWithTrend * 100.0 / totalDatasets) : 0) << "%)"
              << std::endl;
    std::cout << _("statistics.avg_data_points") << ": " << std::setprecision(1)
              << (totalDatasets > 0 ? (totalDataPoints / totalDatasets) : 0) << std::endl;
    std::cout << _("statistics.avg_pg_value") << ": " << std::setprecision(4)
              << (totalDatasets > 0 ? (totalPGSum / totalDatasets) : 0) << std::endl;

    if (totalDatasets > 0) {
        std::cout << _("statistics.pg_range") << ": " << std::setprecision(4) << minOverallPG
                  << " - " << maxOverallPG << std::endl;
    }

    std::cout << std::endl;
    std::cout << _("prompt.press_enter") << std::endl;
    std::cin.get();
}

void TerminalUI::loadSampleData()
{
    clearScreen();
    auto &termUtils = TerminalUtils::getInstance();

    termUtils.printColor("===== " + _("menu.load_sample") + " =====", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl;

    // 定义样本文件目录
    std::string sampleDir = "data/sample/TestSuite";

    // 检查样本目录是否存在
    if (!fs::exists(sampleDir)) {
        termUtils.printError(_("sample.directory_not_found"));
        termUtils.printInfo(_("sample.directory_not_exists") + ": " + sampleDir);
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 扫描样本文件
    std::vector<std::string> sampleFiles;
    try {
        for (const auto &entry : fs::directory_iterator(sampleDir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                // 只显示支持的文件格式
                if (extension == ".csv" || extension == ".txt") {
                    sampleFiles.push_back(entry.path().string());
                }
            }
        }
    }
    catch (const std::exception &e) {
        termUtils.printError(_("sample.scan_error") + ": " + std::string(e.what()));
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    if (sampleFiles.empty()) {
        termUtils.printWarning(_("sample.no_files_found"));
        termUtils.printInfo(_("sample.no_supported_files") + " " + sampleDir);
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 显示可用的样本文件
    termUtils.printColor(_("sample.available_files"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;

    for (size_t i = 0; i < sampleFiles.size(); ++i) {
        fs::path filePath(sampleFiles[i]);
        std::string filename = filePath.filename().string();

        // 显示文件信息
        std::cout << (i + 1) << ". " << filename;

        // 显示文件大小（如果可以获取）
        try {
            auto fileSize = fs::file_size(filePath);
            if (fileSize < 1024) {
                std::cout << " (" << fileSize << " bytes)";
            } else if (fileSize < 1024 * 1024) {
                std::cout << " (" << std::fixed << std::setprecision(1) << (fileSize / 1024.0)
                          << " KB)";
            } else {
                std::cout << " (" << std::fixed << std::setprecision(1)
                          << (fileSize / (1024.0 * 1024.0)) << " MB)";
            }
        }
        catch (...) {
            // 忽略文件大小获取错误
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << _("sample.select_file") << " [1-" << sampleFiles.size() << "] ("
              << _("input.exit_hint") << "): ";

    std::string input;
    std::getline(std::cin, input);

    // 检查是否要退出
    if (input.empty() || input == "q" || input == "Q" || input == "quit" || input == "exit") {
        return;
    }

    // 解析用户选择
    int choice;
    try {
        choice = std::stoi(input);
    }
    catch (...) {
        termUtils.printError(_("error.invalid_choice"));
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    if (choice < 1 || choice > static_cast<int>(sampleFiles.size())) {
        termUtils.printError(_("error.invalid_choice"));
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 加载选中的样本文件
    std::string selectedFile = sampleFiles[choice - 1];
    termUtils.printInfo(_("sample.loading_file") + ": " +
                        fs::path(selectedFile).filename().string());

    try {
        // 显示加载进度
        termUtils.showSpinner(_("progress.loading") + "...", 1000);

        // 询问是否有表头
        std::cout << _("input.has_header") << " [y/n]: ";
        std::string response;
        std::getline(std::cin, response);
        bool hasHeader = !response.empty() && std::tolower(response[0]) == 'y';

        // 导入数据
        DataSet dataSet = DataManager::getInstance().importFromCSV(selectedFile, hasHeader);

        if (dataSet.dataPoints.size() < 4) {
            termUtils.printError(_("error.insufficient_data"));
            termUtils.printInfo(_("sample.insufficient_data_info"));
            std::cout << _("prompt.press_enter");
            std::cin.get();
            return;
        }

        // 显示导入信息
        std::cout << std::endl;
        termUtils.printSuccess(_("sample.file_loaded_success"));
        std::cout << _("import.data_count") << ": " << dataSet.dataPoints.size() << std::endl;

        // 显示前几个数据点作为预览
        termUtils.printColor(_("sample.data_preview"), Color::BRIGHT_YELLOW);
        std::cout << std::endl;
        size_t previewCount = std::min(size_t(5), dataSet.dataPoints.size());
        for (size_t i = 0; i < previewCount; ++i) {
            std::cout << "  " << (i + 1) << ". ";
            if (!dataSet.timePoints.empty()) {
                std::cout << _("sample.time_label") << ": " << std::fixed << std::setprecision(2)
                          << dataSet.timePoints[i] << ", ";
            }
            std::cout << _("sample.value_label") << ": " << std::fixed << std::setprecision(4)
                      << dataSet.dataPoints[i] << std::endl;
        }
        if (dataSet.dataPoints.size() > previewCount) {
            std::cout << "  ... (" << _f("sample.total_data_points", dataSet.dataPoints.size())
                      << ")" << std::endl;
        }
        std::cout << std::endl;

        // 询问是否保存为用户数据集
        std::cout << _("sample.save_as_dataset") << " [y/n]: ";
        std::getline(std::cin, response);

        if (!response.empty() && std::tolower(response[0]) == 'y') {
            // 设置数据集名称（基于文件名）
            fs::path filePath(selectedFile);
            dataSet.name = "sample_" + filePath.stem().string();

            // 可以修改数据集名称
            std::cout << _("import.dataset_name") << " [" << dataSet.name << "]: ";
            std::string name;
            std::getline(std::cin, name);

            if (!name.empty()) {
                dataSet.name = name;
            }

            // 设置描述
            dataSet.description =
                _("sample.description_prefix") + ": " + filePath.filename().string();
            dataSet.source = _("sample.source_name");

            // 设置创建时间
            auto now = std::chrono::system_clock::now();
            auto timeT = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
            dataSet.createdAt = ss.str();

            // 保存数据集
            if (DataManager::getInstance().saveDataSet(dataSet)) {
                termUtils.printSuccess(_("import.dataset_saved"));
            } else {
                termUtils.printError(_("import.dataset_save_failed"));
            }
        }

        // 询问是否立即运行测试
        std::cout << std::endl;
        std::cout << _("sample.run_test_prompt") << " [y/n]: ";
        std::getline(std::cin, response);

        if (!response.empty() && std::tolower(response[0]) == 'y') {
            // 创建计算器并运行测试
            NeumannCalculator calculator;
            NeumannTestResults results =
                calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

            // 显示测试结果
            displayTestResults(results);
        }
    }
    catch (const std::exception &e) {
        termUtils.printError(_("sample.load_error") + ": " + std::string(e.what()));
    }

    std::cout << _("prompt.press_enter");
    std::cin.get();
}

void TerminalUI::manageCustomConfidenceLevels()
{
    clearScreen();
    std::cout << "===== " << _("menu.manage_custom") << " =====" << std::endl;
    std::cout << std::endl;

    auto &standardValues = StandardValues::getInstance();
    auto supportedLevels = standardValues.getSupportedConfidenceLevels();

    // 找出自定义置信度（非标准的0.95, 0.99, 0.999）
    std::vector<double> customLevels;
    for (double level : supportedLevels) {
        if (std::abs(level - 0.95) > 0.001 && std::abs(level - 0.99) > 0.001 &&
            std::abs(level - 0.999) > 0.001) {
            customLevels.push_back(level);
        }
    }

    if (customLevels.empty()) {
        std::cout << _("custom.no_custom_levels") << std::endl;
        std::cout << _("custom.add_custom_suggestion") << std::endl;
        std::cout << std::endl;
        std::cout << _("prompt.press_enter") << std::endl;
        std::cin.get();
        return;
    }

    std::cout << _("custom.current_custom_levels") << std::endl;
    for (size_t i = 0; i < customLevels.size(); ++i) {
        double level = customLevels[i];
        std::cout << (i + 1) << ". " << std::fixed << std::setprecision(3) << level;
        std::cout << " (" << std::fixed << std::setprecision(1) << (level * 100) << "%)"
                  << std::endl;
    }

    std::cout << std::endl;
    std::cout << _("custom.select_action") << std::endl;
    std::cout << "1. " << _("custom.view_details") << std::endl;
    std::cout << "2. " << _("custom.delete_level") << std::endl;
    std::cout << "3. " << _("menu.back") << std::endl;
    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-3]: ";

    int action;
    std::cin >> action;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (action) {
        case 1: {
            // 查看详情
            std::cout << std::endl;
            std::cout << _("custom.select_level_to_view") << " [1-" << customLevels.size() << "]: ";
            int levelChoice;
            std::cin >> levelChoice;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (levelChoice >= 1 && levelChoice <= static_cast<int>(customLevels.size())) {
                double selectedLevel = customLevels[levelChoice - 1];
                std::cout << std::endl;
                std::cout << "===== " << _("custom.level_details") << " =====" << std::endl;
                std::cout << _("custom.confidence_level") << ": " << std::fixed
                          << std::setprecision(3) << selectedLevel << " (" << std::setprecision(1)
                          << (selectedLevel * 100) << "%)" << std::endl;
                std::cout << std::endl;

                // 显示部分标准值
                std::cout << _("custom.sample_values") << ":" << std::endl;
                std::vector<int> sampleSizes = {4, 5, 6, 7, 8, 9, 10, 15, 20};
                for (int size : sampleSizes) {
                    double wpValue = standardValues.getWPValue(size, selectedLevel);
                    if (wpValue > 0) {
                        std::cout << "  n=" << size << ": W(P)=" << std::fixed
                                  << std::setprecision(4) << wpValue << std::endl;
                    }
                }
            } else {
                std::cout << _("error.invalid_choice") << std::endl;
            }
            break;
        }
        case 2: {
            // 删除置信度
            std::cout << std::endl;
            std::cout << _("custom.select_level_to_delete") << " [1-" << customLevels.size()
                      << "]: ";
            int levelChoice;
            std::cin >> levelChoice;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (levelChoice >= 1 && levelChoice <= static_cast<int>(customLevels.size())) {
                double selectedLevel = customLevels[levelChoice - 1];
                std::cout << std::endl;
                std::cout << _("custom.confirm_delete") << " " << std::fixed << std::setprecision(3)
                          << selectedLevel << " [y/n]: ";
                std::string response;
                std::getline(std::cin, response);

                if (!response.empty() && std::tolower(response[0]) == 'y') {
                    if (standardValues.removeConfidenceLevel(selectedLevel)) {
                        std::cout << _("custom.delete_success") << std::endl;
                    } else {
                        std::cout << _("custom.delete_failed") << std::endl;
                    }
                } else {
                    std::cout << _("custom.delete_cancelled") << std::endl;
                }
            } else {
                std::cout << _("error.invalid_choice") << std::endl;
            }
            break;
        }
        case 3:
            return;
        default:
            std::cout << _("error.invalid_choice") << std::endl;
            break;
    }

    std::cout << std::endl;
    std::cout << _("prompt.press_enter") << std::endl;
    std::cin.get();
}

void TerminalUI::startWebServer()
{
    clearScreen();
    auto &termUtils = TerminalUtils::getInstance();
    auto &config = Config::getInstance();

    // 标题
    termUtils.printColor("===== " + _("web.start_server") + " =====", Color::BRIGHT_CYAN,
                         TextStyle::BOLD);
    std::cout << std::endl << std::endl;

    // 检查是否已经有Web服务器在运行
    if (webServer && webServer->isRunning()) {
        termUtils.printWarning(_("web.server_already_running"));
        std::cout << _("web.current_url") << ": " << webServer->getUrl() << std::endl;
        std::cout << std::endl;

        // 提供更多选项
        std::cout << _("web.server_running_options") << std::endl;
        std::cout << "1. " << _("web.option_continue_background") << std::endl;
        std::cout << "2. " << _("web.option_stop_server") << std::endl;
        std::cout << "3. " << _("menu.back") << std::endl;
        std::cout << std::endl;
        std::cout << _("prompt.select_option") << " [1-3]: ";

        std::string response;
        std::getline(std::cin, response);

        if (response == "2") {
            termUtils.printInfo(_("web.stopping_server"));
            webServer->stop();
            webServer.reset();
            termUtils.printSuccess(_("web.server_stopped"));
            std::cout << _("prompt.press_enter");
            std::cin.get();
        } else if (response == "1") {
            // 进入Web服务器运行状态
            showWebServerRunningInterface();
        }
        // 选项3或其他输入都返回主菜单
        return;
    }

    // 获取配置参数
    int defaultPort = config.getDefaultWebPort();
    std::string webRootDir = config.getWebRootDirectory();

    // 询问端口设置
    std::cout << _("web.port_prompt") << " (" << _("menu.default") << ": " << defaultPort << "): ";
    std::string portInput;
    std::getline(std::cin, portInput);

    int port = defaultPort;
    if (!portInput.empty()) {
        try {
            port = std::stoi(portInput);
            if (port < 1024 || port > 65535) {
                termUtils.printWarning(_("web.invalid_port_range"));
                port = defaultPort;
            }
        }
        catch (const std::exception &e) {
            termUtils.printWarning(_("web.invalid_port_format"));
            port = defaultPort;
        }
    }

    // 询问Web资源目录
    std::cout << _("web.webroot_prompt") << " (" << _("menu.default") << ": " << webRootDir
              << "): ";
    std::string webRootInput;
    std::getline(std::cin, webRootInput);

    if (!webRootInput.empty()) {
        webRootDir = webRootInput;
    }

    // 检查Web资源目录是否存在
    if (!fs::exists(webRootDir)) {
        termUtils.printWarning(_("web.webroot_not_found") + ": " + webRootDir);
        std::cout << _("web.create_webroot_prompt") << " [y/n]: ";

        std::string createResponse;
        std::getline(std::cin, createResponse);

        if (!createResponse.empty() && std::tolower(createResponse[0]) == 'y') {
            try {
                fs::create_directories(webRootDir);
                termUtils.printSuccess(_("web.webroot_created") + ": " + webRootDir);
            }
            catch (const std::exception &e) {
                termUtils.printError(_("web.webroot_create_failed") + ": " + e.what());
                std::cout << _("prompt.press_enter");
                std::cin.get();
                return;
            }
        } else {
            termUtils.printInfo(_("web.using_default_webroot"));
            webRootDir = "web";  // 使用默认目录
        }
    }

    std::cout << std::endl;
    termUtils.printInfo(_("web.starting_server"));
    std::cout << _("web.server_port") << ": " << port << std::endl;
    std::cout << _("web.server_webroot") << ": " << webRootDir << std::endl;
    std::cout << std::endl;

    try {
        // 创建Web服务器实例
        webServer = std::make_unique<neumann::web::WebServer>(port, webRootDir);

        // 启动服务器（后台模式）
        webServer->start(true);

        // 显示成功信息
        termUtils.printSuccess(_("web.server_started"));
        std::cout << std::endl;

        // 保存端口设置到配置
        if (port != defaultPort) {
            config.setDefaultWebPort(port);
            std::string configFile = config.getConfigFilePath();
            if (config.saveToFile(configFile)) {
                termUtils.printInfo(_("web.port_saved"));
            }
        }

        // 进入Web服务器运行状态
        showWebServerRunningInterface();
    }
    catch (const std::exception &e) {
        termUtils.printError(_("web.server_start_failed") + ": " + e.what());
        webServer.reset();
        std::cout << _("prompt.press_enter");
        std::cin.get();
    }
}

void TerminalUI::showWebServerRunningInterface()
{
    auto &termUtils = TerminalUtils::getInstance();

    while (webServer && webServer->isRunning()) {
        clearScreen();

        // 标题
        termUtils.printColor("===== " + _("web.server_running") + " =====", Color::BRIGHT_CYAN,
                             TextStyle::BOLD);
        std::cout << std::endl << std::endl;

        // 显示服务器信息
        termUtils.printColor("🌐 " + _("web.access_url") + ": ", Color::BRIGHT_CYAN,
                             TextStyle::BOLD);
        termUtils.printColor(webServer->getUrl(), Color::BRIGHT_GREEN, TextStyle::UNDERLINE);
        std::cout << std::endl << std::endl;

        // 显示状态信息
        termUtils.printSuccess(_("web.server_status_running"));
        std::cout << std::endl;

        // 显示操作说明
        termUtils.printInfo(_("web.server_instructions"));
        std::cout << "• " << _("web.instruction_browser") << std::endl;
        std::cout << "• " << _("web.instruction_enter_return") << std::endl;
        std::cout << "• " << _("web.instruction_ctrl_c_stop") << std::endl;
        std::cout << std::endl;

        termUtils.printColor(_("web.waiting_for_input"), Color::YELLOW, TextStyle::BOLD);
        std::cout << std::endl;

        // 等待用户输入
        std::string input;
        std::getline(std::cin, input);

        // 如果用户按Enter，返回主菜单但保持服务器运行
        if (input.empty()) {
            termUtils.printInfo(_("web.returning_to_menu"));
            std::cout << _("web.server_continues_background") << std::endl;
            std::cout << _("prompt.press_enter");
            std::cin.get();
            break;
        }
        // 如果用户输入其他内容，检查是否是停止命令
        else if (input == "stop" || input == "quit" || input == "exit") {
            termUtils.printInfo(_("web.stopping_server"));
            webServer->stop();
            webServer.reset();
            termUtils.printSuccess(_("web.server_stopped"));
            std::cout << _("prompt.press_enter");
            std::cin.get();
            break;
        }
        // 其他输入显示帮助
        else {
            termUtils.printWarning(_("web.invalid_command"));
            std::cout << _("web.valid_commands") << std::endl;
            std::cout << _("prompt.press_enter");
            std::cin.get();
        }
    }
}

}}  // namespace neumann::cli