#include "cli/terminal_ui.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "core/config.h"
#include "core/i18n.h"
#include "core/standard_values.h"

namespace fs = std::filesystem;

namespace neumann { namespace cli {

TerminalUI::TerminalUI() : running(false), currentMenuId("main")
{
    initializeMenus();
}

void TerminalUI::run()
{
    running = true;

    // 显示欢迎信息
    clearScreen();
    std::cout << "=====================================" << std::endl;
    std::cout << "  " << _("app.title") << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    // 加载标准值
    if (!StandardValues::getInstance().loadFromFile("data/standard_values.json")) {
        std::cout << _("error.standard_values_not_found") << std::endl;
    }

    // 主循环
    while (running) {
        displayMenu();
        handleInput();
    }

    // 显示退出信息
    std::cout << std::endl;
    std::cout << _("status.goodbye") << std::endl;
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
        {"back", "menu.back", MenuItemType::BACK, "", nullptr}};
    menus[settingsMenu.id] = settingsMenu;
}

void TerminalUI::displayMenu()
{
    clearScreen();

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

    std::cout << prompt << " (" << _("prompt.separator_help") << "): ";
    std::getline(std::cin, input);

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

    // 选择数据集
    std::cout << std::endl;
    std::cout << _("load.select_dataset") << " [1-" << datasets.size() << "]: ";
    int choice;
    std::cin >> choice;

    // 清除输入缓冲区
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
    std::cout << "===== 从CSV导入数据 =====" << std::endl;
    std::cout << std::endl;

    // 获取CSV文件路径
    std::cout << "请输入CSV文件路径: ";
    std::string filePath;
    std::getline(std::cin, filePath);

    if (filePath.empty() || !fs::exists(filePath)) {
        std::cout << "文件不存在或路径无效。" << std::endl;
        std::cout << "按Enter键返回主菜单...";
        std::cin.get();
        return;
    }

    // 询问是否有表头
    std::cout << "CSV文件是否有表头? [y/n]: ";
    std::string response;
    std::getline(std::cin, response);
    bool hasHeader = !response.empty() && std::tolower(response[0]) == 'y';

    // 导入数据
    DataSet dataSet = DataManager::getInstance().importFromCSV(filePath, hasHeader);

    // 显示导入信息
    std::cout << std::endl;
    std::cout << "成功导入数据集: " << dataSet.name << std::endl;
    std::cout << "数据点数量: " << dataSet.dataPoints.size() << std::endl;

    // 询问是否保存
    std::cout << std::endl;
    std::cout << "是否保存该数据集? [y/n]: ";
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        // 可以修改数据集名称
        std::cout << "请输入数据集名称 [" << dataSet.name << "]: ";
        std::string name;
        std::getline(std::cin, name);

        if (!name.empty()) {
            dataSet.name = name;
        }

        // 可以添加描述
        std::cout << "请输入数据集描述: ";
        std::getline(std::cin, dataSet.description);

        // 保存数据集
        if (DataManager::getInstance().saveDataSet(dataSet)) {
            std::cout << "数据集已保存。" << std::endl;
        } else {
            std::cout << "保存数据集失败。" << std::endl;
        }
    }

    // 询问是否运行测试
    std::cout << std::endl;
    std::cout << "是否对该数据集运行诺依曼趋势测试? [y/n]: ";
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        // 创建计算器并运行测试
        NeumannCalculator calculator;
        NeumannTestResults results = calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

        // 显示测试结果
        displayTestResults(results);
    }

    std::cout << "按Enter键返回主菜单...";
    std::cin.get();
}

void TerminalUI::runNeumannTest()
{
    clearScreen();
    std::cout << "===== " << _("menu.new_test") << " =====" << std::endl;
    std::cout << std::endl;

    // 询问数据点
    std::vector<double> dataPoints = promptForData(_("input.data_points"));

    if (dataPoints.size() < 4) {
        std::cout << _("error.insufficient_data") << std::endl;
        std::cout << _("prompt.press_enter");
        std::cin.get();
        return;
    }

    // 询问时间点
    std::vector<double> timePoints =
        promptForTimePoints(_("test.use_default_timepoints"), dataPoints.size());

    // 设置置信水平
    double confidenceLevel = 0.95;  // 默认
    std::cout << _("test.confidence_level_prompt") << " [0.95]: ";
    std::string input;
    std::getline(std::cin, input);

    if (!input.empty()) {
        try {
            double level = std::stod(input);
            if (level > 0.0 && level < 1.0) {
                confidenceLevel = level;
            }
        }
        catch (const std::exception &e) {
            // 使用默认值
        }
    }

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
    std::cout << "===== " << _("result.test_results") << " =====" << std::endl;
    std::cout << std::endl;

    if (results.results.empty()) {
        std::cout << _("result.test_failed") << std::endl;
        return;
    }

    // 输出表头
    std::cout << std::left << std::setw(15) << _("result.data_point") << std::setw(15)
              << _("result.time_point") << std::setw(15) << _("result.pg_value") << std::setw(15)
              << _("result.threshold") << std::setw(15) << _("result.trend_judgment") << std::endl;

    std::cout << std::string(75, '-') << std::endl;

    // 输出测试结果
    for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

        std::cout << std::left << std::setw(15) << results.data[dataIndex] << std::setw(15)
                  << results.timePoints[dataIndex] << std::setw(15) << std::fixed
                  << std::setprecision(4) << results.results[i].pgValue << std::setw(15)
                  << results.results[i].wpThreshold << std::setw(15)
                  << (results.results[i].hasTrend ? _("result.has_trend") : _("result.no_trend"))
                  << std::endl;
    }

    std::cout << std::string(75, '-') << std::endl;

    // 输出汇总结果
    std::cout << std::endl;
    std::cout << _("result.summary") << std::endl;
    std::cout << _("result.overall_trend") << ": "
              << (results.overallTrend ? _("result.has_trend") : _("result.no_trend")) << std::endl;
    std::cout << _("result.min_pg") << ": " << std::fixed << std::setprecision(4) << results.minPG
              << std::endl;
    std::cout << _("result.max_pg") << ": " << results.maxPG << std::endl;
    std::cout << _("result.avg_pg") << ": " << results.avgPG << std::endl;

    std::cout << std::endl;
    std::cout << _("result.conclusion") << std::endl;
    if (results.overallTrend) {
        std::cout << _("result.conclusion_trend") << std::endl;
    } else {
        std::cout << _("result.conclusion_no_trend") << std::endl;
    }
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
            Config::getInstance().saveToFile("config.json");
            std::cout << "语言已设置为中文" << std::endl;
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
            Config::getInstance().saveToFile("config.json");
            std::cout << "Language set to English" << std::endl;
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

}}  // namespace neumann::cli