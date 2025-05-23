#include "cli/terminal_ui.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "cli/terminal_utils.h"
#include "core/config.h"
#include "core/excel_reader.h"
#include "core/i18n.h"
#include "core/standard_values.h"

namespace fs = std::filesystem;

namespace neumann { namespace cli {

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

    // 计算表格列宽
    std::vector<int> columnWidths = {15, 15, 15, 15, 18};

    // 表头
    std::vector<std::string> headers = {_("result.data_point"), _("result.time_point"),
                                        _("result.pg_value"), _("result.threshold"),
                                        _("result.trend_judgment")};

    // 显示表头
    termUtils.printColor(termUtils.formatTableRow(headers, columnWidths, "lllll"),
                         Color::BRIGHT_WHITE, TextStyle::BOLD);
    std::cout << std::endl;

    // 分隔线
    termUtils.printColor(termUtils.createTableSeparator(78, '='), Color::CYAN);
    std::cout << std::endl;

    // 显示数据行
    for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

        std::vector<std::string> row = {
            std::to_string(results.data[dataIndex]), std::to_string(results.timePoints[dataIndex]),
            std::to_string(results.results[i].pgValue),
            std::to_string(results.results[i].wpThreshold),
            results.results[i].hasTrend ? _("result.has_trend") : _("result.no_trend")};

        // 根据趋势判断使用不同颜色
        Color rowColor = results.results[i].hasTrend ? Color::BRIGHT_RED : Color::BRIGHT_GREEN;
        termUtils.printColor(termUtils.formatTableRow(row, columnWidths, "rrrrr"), rowColor);
        std::cout << std::endl;
    }

    // 分隔线
    termUtils.printColor(termUtils.createTableSeparator(78, '='), Color::CYAN);
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
}

void TerminalUI::showHelp()
{
    clearScreen();
    std::cout << "===== " << _("help.title") << " =====" << std::endl;
    std::cout << std::endl;

    std::cout << _("help.description") << std::endl;
    std::cout << _("help.usage_areas") << std::endl;
    std::cout << std::endl;

    std::cout << _("help.usage_guide") << std::endl;
    std::cout << "1. " << _("help.guide_1") << std::endl;
    std::cout << "2. " << _("help.guide_2") << std::endl;
    std::cout << "3. " << _("help.guide_3") << std::endl;
    std::cout << std::endl;

    std::cout << _("help.notes") << std::endl;
    std::cout << "- " << _("help.note_1") << std::endl;
    std::cout << "- " << _("help.note_2") << std::endl;
    std::cout << "- " << _("help.note_3") << std::endl;
    std::cout << "- " << _("help.note_4") << std::endl;

    std::cout << std::endl;
    std::cout << _("prompt.press_enter") << std::endl;
    std::cin.get();
}

void TerminalUI::showAbout()
{
    clearScreen();
    std::cout << "===== " << _("about.title") << " =====" << std::endl;
    std::cout << std::endl;

    std::cout << _("app.title") << " v1.2.0" << std::endl;
    std::cout << "Copyright © 2025" << std::endl;
    std::cout << std::endl;

    std::cout << _("about.description") << std::endl;
    std::cout << _("about.applications") << std::endl;

    std::cout << std::endl;
    std::cout << _("prompt.press_enter") << std::endl;
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
    std::cout << _("prompt.current_confidence_level") << ": " << config.getDefaultConfidenceLevel()
              << std::endl;
    std::cout << std::endl;

    std::cout << "1. 0.90 (90%)" << std::endl;
    std::cout << "2. 0.95 (95%) - " << _("menu.default") << std::endl;
    std::cout << "3. 0.99 (99%)" << std::endl;
    std::cout << "4. " << _("menu.custom") << std::endl;
    std::cout << "5. " << _("menu.back") << std::endl;
    std::cout << std::endl;
    std::cout << _("prompt.select_option") << " [1-5]: ";

    int choice;
    std::cin >> choice;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    double newLevel = 0.95;  // 默认值
    bool levelChanged = false;

    switch (choice) {
        case 1:
            newLevel = 0.90;
            levelChanged = true;
            break;
        case 2:
            newLevel = 0.95;
            levelChanged = true;
            break;
        case 3:
            newLevel = 0.99;
            levelChanged = true;
            break;
        case 4: {
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
        } break;
        case 5:
            return;
        default:
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

    // 获取当前置信度水平
    double confidenceLevel = config.getDefaultConfidenceLevel();

    // 格式化置信度显示
    std::ostringstream statusText;
    statusText << "Confidence: " << std::fixed << std::setprecision(2) << (confidenceLevel * 100)
               << "%";

    // 获取终端宽度（假设80字符，可以后续改进为动态获取）
    int terminalWidth = 80;
    int statusWidth = statusText.str().length();
    int padding = terminalWidth - statusWidth;

    // 显示状态栏（右对齐）
    if (padding > 0) {
        std::cout << std::string(padding, ' ');
    }
    termUtils.printColor(statusText.str(), Color::BRIGHT_BLUE, TextStyle::BOLD);
    std::cout << std::endl;

    // 添加分隔线
    termUtils.printColor(std::string(terminalWidth, '-'), Color::CYAN);
    std::cout << std::endl << std::endl;
}

}}  // namespace neumann::cli