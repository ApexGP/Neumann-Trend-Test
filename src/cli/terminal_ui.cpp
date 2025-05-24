#include "cli/terminal_ui.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "cli/terminal_utils.h"
#include "core/batch_processor.h"
#include "core/config.h"
#include "core/data_visualization.h"
#include "core/excel_reader.h"
#include "core/i18n.h"
#include "core/standard_values.h"

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

TerminalUI::TerminalUI() : currentMenuId("main"), running(false)
{
    initializeMenus();
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
        std::cout << (i + 1) << ". " << _(menu.items[i].title.c_str()) << std::endl;
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

    // 计算表格列宽 - 根据中文字符实际显示宽度重新设计
    // "数据点"=6, "时间点"=6, "PG值"=6, "W(P)阈值"=8, "趋势判断"=8
    // 考虑数据内容：数值通常8-12位，中文判断8位，留些余量
    std::vector<int> columnWidths = {12, 10, 8, 10, 10};

    // 表头
    std::vector<std::string> headers = {_("result.data_point"), _("result.time_point"),
                                        _("result.pg_value"), _("result.threshold"),
                                        _("result.trend_judgment")};

    // 显示表头 - 使用混合对齐：数值列右对齐，文本列左对齐
    termUtils.printColor(termUtils.formatTableRow(headers, columnWidths, "rrrrr"),
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
        termUtils.printColor(termUtils.formatTableRow(row, columnWidths, "rrrrr"), rowColor);
        std::cout << std::endl;
    }

    // 分隔线
    termUtils.printColor(termUtils.createTableSeparator(tableWidth, '='), Color::CYAN);
    std::cout << std::endl << std::endl;

    // 汇总结果
    termUtils.printColor(_("result.summary"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;

    std::cout << _("result.overall_trend") << ": ";
    if (results.overallTrend) {
        termUtils.printColor(_("result.has_trend"), Color::BRIGHT_RED, TextStyle::BOLD);
    } else {
        termUtils.printColor(_("result.no_trend"), Color::BRIGHT_GREEN, TextStyle::BOLD);
    }
    std::cout << std::endl;

    std::cout << _("result.min_pg") << ": " << std::fixed << std::setprecision(4) << results.minPG
              << std::endl;
    std::cout << _("result.max_pg") << ": " << results.maxPG << std::endl;
    std::cout << _("result.avg_pg") << ": " << results.avgPG << std::endl;

    std::cout << std::endl;
    termUtils.printColor(_("result.conclusion"), Color::BRIGHT_YELLOW, TextStyle::BOLD);
    std::cout << std::endl;

    if (results.overallTrend) {
        termUtils.printColor(_("result.conclusion_trend"), Color::YELLOW);
    } else {
        termUtils.printColor(_("result.conclusion_no_trend"), Color::GREEN);
    }
    std::cout << std::endl;

    // 添加ASCII图表显示
    std::cout << std::endl;
    std::string asciiChart = DataVisualization::generateASCIIChart(results);
    std::cout << asciiChart << std::endl;
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
    std::cout << " v2.0.0" << std::endl;
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
    std::cout << (supportedLevels.size() + 2) << ". " << _("menu.back") << std::endl;
    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-" << (supportedLevels.size() + 2) << "]: ";

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
        // 自定义置信度
        std::cout << _("prompt.enter_confidence_level") << " (0.0-1.0): ";
        std::string input;
        std::getline(std::cin, input);
        try {
            double level = std::stod(input);
            if (level > 0.0 && level < 1.0) {
                newLevel = level;
                levelChanged = true;
            } else {
                std::cout << _("error.invalid_confidence_level") << std::endl;
            }
        }
        catch (const std::exception &e) {
            std::cout << _("error.invalid_input") << std::endl;
        }
    } else if (choice == static_cast<int>(supportedLevels.size()) + 2) {
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
    confidenceText << "Confidence: " << std::fixed << std::setprecision(2)
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

            std::string svgChart = DataVisualization::generateTrendChart(results);
            if (DataVisualization::saveChartToFile(svgChart, filename)) {
                std::cout << _("visualization.chart_saved") << ": " << filename << std::endl;
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
    std::string sampleDir = "data/sample";

    // 检查样本目录是否存在
    if (!fs::exists(sampleDir)) {
        termUtils.printError(_("sample.directory_not_found"));
        termUtils.printInfo("样本文件目录不存在: " + sampleDir);
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
        termUtils.printError("扫描样本文件时出错: " + std::string(e.what()));
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    if (sampleFiles.empty()) {
        termUtils.printWarning(_("sample.no_files_found"));
        termUtils.printInfo("在 " + sampleDir + " 目录下没有找到支持的样本文件(.csv, .txt)");
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
    termUtils.printInfo("正在加载样本文件: " + fs::path(selectedFile).filename().string());

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
            termUtils.printInfo("至少需要4个数据点才能进行诺依曼趋势测试");
            std::cout << _("prompt.press_enter");
            std::cin.get();
            return;
        }

        // 显示导入信息
        std::cout << std::endl;
        termUtils.printSuccess("样本文件加载成功!");
        std::cout << _("import.data_count") << ": " << dataSet.dataPoints.size() << std::endl;

        // 显示前几个数据点作为预览
        termUtils.printColor("数据预览:", Color::BRIGHT_YELLOW);
        std::cout << std::endl;
        size_t previewCount = std::min(size_t(5), dataSet.dataPoints.size());
        for (size_t i = 0; i < previewCount; ++i) {
            std::cout << "  " << (i + 1) << ". ";
            if (!dataSet.timePoints.empty()) {
                std::cout << "时间: " << std::fixed << std::setprecision(2) << dataSet.timePoints[i]
                          << ", ";
            }
            std::cout << "数值: " << std::fixed << std::setprecision(4) << dataSet.dataPoints[i]
                      << std::endl;
        }
        if (dataSet.dataPoints.size() > previewCount) {
            std::cout << "  ... (共 " << dataSet.dataPoints.size() << " 个数据点)" << std::endl;
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
            dataSet.description = "从样本文件加载: " + filePath.filename().string();
            dataSet.source = "样本文件";

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
        termUtils.printError("加载样本文件时出错: " + std::string(e.what()));
    }

    std::cout << _("prompt.press_enter");
    std::cin.get();
}

}}  // namespace neumann::cli