#include "cli/terminal_ui.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

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
    std::cout << "  诺依曼趋势测试工具 (Neumann Trend Test)" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    // 加载标准值
    if (!StandardValues::getInstance().loadFromFile("data/standard_values.json")) {
        std::cout << "警告: 无法加载标准值文件，将使用内置默认值。" << std::endl;
    }

    // 主循环
    while (running) {
        displayMenu();
        handleInput();
    }

    // 显示退出信息
    std::cout << std::endl;
    std::cout << "感谢使用诺依曼趋势测试工具，再见！" << std::endl;
}

void TerminalUI::initializeMenus()
{
    // 主菜单
    Menu mainMenu;
    mainMenu.id = "main";
    mainMenu.title = "主菜单";
    mainMenu.items = {
        {"new_test", "运行新的诺依曼趋势测试", MenuItemType::ACTION, "",
         [this]() { runNeumannTest(); }},
        {"load_data", "加载数据集", MenuItemType::ACTION, "", [this]() { loadDataSet(); }},
        {"import_csv", "从CSV导入数据", MenuItemType::ACTION, "", [this]() { importFromCSV(); }},
        {"help", "帮助", MenuItemType::ACTION, "", [this]() { showHelp(); }},
        {"about", "关于", MenuItemType::ACTION, "", [this]() { showAbout(); }},
        {"exit", "退出", MenuItemType::EXIT, "", nullptr}};
    menus[mainMenu.id] = mainMenu;
}

void TerminalUI::displayMenu()
{
    clearScreen();

    // 获取当前菜单
    const Menu &menu = menus[currentMenuId];

    std::cout << "===== " << menu.title << " =====" << std::endl;
    std::cout << std::endl;

    // 显示菜单项
    for (size_t i = 0; i < menu.items.size(); ++i) {
        std::cout << (i + 1) << ". " << menu.items[i].title << std::endl;
    }

    std::cout << std::endl;
    std::cout << "请选择操作 [1-" << menu.items.size() << "]: ";
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
        std::cout << "无效选择，请重试。" << std::endl;
        std::cout << "按Enter键继续...";
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

    std::cout << prompt << " (用空格或逗号分隔): ";
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
        timePoints = promptForData("请输入时间点数据");

        // 确保时间点数量与数据点一致
        if (timePoints.size() != count) {
            std::cout << "警告: 时间点数量与数据点数量不匹配，将使用默认时间点。" << std::endl;
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
    std::cout << "===== 加载数据集 =====" << std::endl;
    std::cout << std::endl;

    // 获取可用数据集
    std::vector<std::string> datasets = DataManager::getInstance().getDataSetNames();

    if (datasets.empty()) {
        std::cout << "没有可用的数据集。" << std::endl;
        std::cout << "按Enter键返回主菜单...";
        std::cin.get();
        return;
    }

    // 显示数据集列表
    std::cout << "可用数据集:" << std::endl;
    for (size_t i = 0; i < datasets.size(); ++i) {
        std::cout << (i + 1) << ". " << datasets[i] << std::endl;
    }

    // 选择数据集
    std::cout << std::endl;
    std::cout << "请选择数据集 [1-" << datasets.size() << "]: ";
    int choice;
    std::cin >> choice;

    // 清除输入缓冲区
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice < 1 || choice > static_cast<int>(datasets.size())) {
        std::cout << "无效选择。" << std::endl;
        std::cout << "按Enter键返回主菜单...";
        std::cin.get();
        return;
    }

    // 加载选择的数据集
    std::string datasetName = datasets[choice - 1];
    DataSet dataSet = DataManager::getInstance().loadDataSet(datasetName);

    // 显示数据集信息
    std::cout << std::endl;
    std::cout << "数据集: " << dataSet.name << std::endl;
    std::cout << "描述: " << dataSet.description << std::endl;
    std::cout << "来源: " << dataSet.source << std::endl;
    std::cout << "创建时间: " << dataSet.createdAt << std::endl;
    std::cout << "数据点数量: " << dataSet.dataPoints.size() << std::endl;

    // 询问是否运行测试
    std::cout << std::endl;
    std::cout << "是否对该数据集运行诺依曼趋势测试? [y/n]: ";
    std::string response;
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
    std::cout << "===== 运行诺依曼趋势测试 =====" << std::endl;
    std::cout << std::endl;

    // 询问数据点
    std::vector<double> dataPoints = promptForData("请输入数据点");

    if (dataPoints.size() < 4) {
        std::cout << "错误: 需要至少4个数据点才能进行诺依曼趋势测试。" << std::endl;
        std::cout << "按Enter键返回主菜单...";
        std::cin.get();
        return;
    }

    // 询问时间点
    std::vector<double> timePoints =
        promptForTimePoints("是否使用默认时间点 (0, 1, 2, ...)?", dataPoints.size());

    // 设置置信水平
    double confidenceLevel = 0.95;  // 默认
    std::cout << "请输入置信水平 [0.95]: ";
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
    std::cout << "是否保存该数据集? [y/n]: ";
    std::string response;
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        DataSet dataSet;
        dataSet.dataPoints = dataPoints;
        dataSet.timePoints = timePoints;

        // 询问数据集名称
        std::cout << "请输入数据集名称: ";
        std::getline(std::cin, dataSet.name);

        if (dataSet.name.empty()) {
            dataSet.name = "dataset_" + std::to_string(std::time(nullptr));
        }

        // 询问描述
        std::cout << "请输入数据集描述: ";
        std::getline(std::cin, dataSet.description);

        // 设置来源
        dataSet.source = "手动输入";

        // 设置创建时间
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
        dataSet.createdAt = ss.str();

        // 保存数据集
        if (DataManager::getInstance().saveDataSet(dataSet)) {
            std::cout << "数据集已保存。" << std::endl;
        } else {
            std::cout << "保存数据集失败。" << std::endl;
        }
    }

    std::cout << "按Enter键返回主菜单...";
    std::cin.get();
}

void TerminalUI::displayTestResults(const NeumannTestResults &results)
{
    clearScreen();
    std::cout << "===== 诺依曼趋势测试结果 =====" << std::endl;
    std::cout << std::endl;

    if (results.results.empty()) {
        std::cout << "测试未能成功完成，可能是因为数据点不足。" << std::endl;
        return;
    }

    // 输出表头
    std::cout << std::left << std::setw(15) << "数据点" << std::setw(15) << "时间点"
              << std::setw(15) << "PG值" << std::setw(15) << "W(P)阈值" << std::setw(15)
              << "趋势判断" << std::endl;

    std::cout << std::string(75, '-') << std::endl;

    // 输出测试结果
    for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

        std::cout << std::left << std::setw(15) << results.data[dataIndex] << std::setw(15)
                  << results.timePoints[dataIndex] << std::setw(15) << std::fixed
                  << std::setprecision(4) << results.results[i].pgValue << std::setw(15)
                  << results.results[i].wpThreshold << std::setw(15)
                  << (results.results[i].hasTrend ? "是" : "否") << std::endl;
    }

    std::cout << std::string(75, '-') << std::endl;

    // 输出汇总结果
    std::cout << std::endl;
    std::cout << "汇总结果:" << std::endl;
    std::cout << "整体是否存在趋势: " << (results.overallTrend ? "是" : "否") << std::endl;
    std::cout << "最小PG值: " << std::fixed << std::setprecision(4) << results.minPG << std::endl;
    std::cout << "最大PG值: " << results.maxPG << std::endl;
    std::cout << "平均PG值: " << results.avgPG << std::endl;

    std::cout << std::endl;
    std::cout << "诺依曼趋势测试结论:" << std::endl;
    if (results.overallTrend) {
        std::cout << "数据存在显著趋势，可能表明测试样品在所测量的时间内存在系统性变化。"
                  << std::endl;
    } else {
        std::cout << "数据无显著趋势，测试样品在所测量的时间内保持稳定。" << std::endl;
    }
}

void TerminalUI::showHelp()
{
    clearScreen();
    std::cout << "===== 帮助信息 =====" << std::endl;
    std::cout << std::endl;

    std::cout << "诺依曼趋势测试是一种统计方法，用于评估数据集是否存在系统性趋势。" << std::endl;
    std::cout << "这种测试常用于药物稳定性研究和质量控制等领域。" << std::endl;
    std::cout << std::endl;

    std::cout << "使用指南:" << std::endl;
    std::cout << "1. 运行新的诺依曼趋势测试 - 直接输入数据并执行测试" << std::endl;
    std::cout << "2. 加载数据集 - 加载之前保存的数据集并执行测试" << std::endl;
    std::cout << "3. 从CSV导入数据 - 从CSV文件导入数据以便测试" << std::endl;
    std::cout << std::endl;

    std::cout << "注意事项:" << std::endl;
    std::cout << "- 诺依曼趋势测试至少需要4个数据点" << std::endl;
    std::cout << "- 标准W(P)值基于特定的统计表，用于判断趋势是否显著" << std::endl;
    std::cout << "- 如果PG值小于或等于W(P)值，则判断为存在趋势" << std::endl;
    std::cout << "- 默认使用95%置信水平，可以手动调整" << std::endl;

    std::cout << std::endl;
    std::cout << "按Enter键返回主菜单...";
    std::cin.get();
}

void TerminalUI::showAbout()
{
    clearScreen();
    std::cout << "===== 关于 =====" << std::endl;
    std::cout << std::endl;

    std::cout << "诺依曼趋势测试工具 v1.2.0" << std::endl;
    std::cout << "Copyright © 2025" << std::endl;
    std::cout << std::endl;

    std::cout << "本工具用于执行诺依曼趋势测试，支持数据管理、测试执行和结果分析。" << std::endl;
    std::cout << "可广泛应用于药物稳定性研究、质量控制和时间序列分析等领域。" << std::endl;

    std::cout << std::endl;
    std::cout << "按Enter键返回主菜单...";
    std::cin.get();
}

}}  // namespace neumann::cli