#include "cli/cli_app.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "core/data_manager.h"
#include "core/i18n.h"
#include "core/neumann_calculator.h"
#include "core/standard_values.h"

namespace fs = std::filesystem;

namespace neumann { namespace cli {

CLIApp::CLIApp() {}

int CLIApp::run(int argc, char **argv)
{
    // 处理命令行参数
    if (argc > 1) {
        if (processArgs(argc, argv)) {
            return 0;
        }
    }

    // 加载标准值
    if (!StandardValues::getInstance().loadFromFile("data/standard_values.json")) {
        std::cerr << _("error.standard_values_not_found") << std::endl;
    }

    // 运行终端UI
    ui.run();

    return 0;
}

bool CLIApp::processArgs(int argc, char **argv)
{
    std::string arg = argv[1];

    if (arg == "-h" || arg == "--help") {
        showHelp();
        return true;
    } else if (arg == "-v" || arg == "--version") {
        showVersion();
        return true;
    } else if (arg == "-f" || arg == "--file") {
        if (argc < 3) {
            std::cerr << _("error.missing_file_argument") << std::endl;
            showHelp();
            return true;
        }

        runWithData(argv[2]);
        return true;
    }

    return false;
}

void CLIApp::showHelp()
{
    std::cout << _("app.title") << " v1.0.0" << std::endl;
    std::cout << _("help.usage") << std::endl;
    std::cout << std::endl;
    std::cout << _("help.options") << std::endl;
    std::cout << "  -h, --help       " << _("help.show_help") << std::endl;
    std::cout << "  -v, --version    " << _("help.show_version") << std::endl;
    std::cout << "  -f, --file PATH  " << _("help.process_file") << std::endl;
    std::cout << std::endl;
    std::cout << _("help.examples") << std::endl;
    std::cout << "  neumann              " << _("help.example_interactive") << std::endl;
    std::cout << "  neumann -f data.csv  " << _("help.example_file") << std::endl;
}

void CLIApp::showVersion()
{
    std::cout << _("app.title") << " v1.0.0" << std::endl;
    std::cout << "Copyright © 2023" << std::endl;
}

void CLIApp::runWithData(const std::string &dataFile)
{
    // 检查文件是否存在
    if (!fs::exists(dataFile)) {
        std::cerr << _("error.file_not_found") << ": " << dataFile << std::endl;
        return;
    }

    // 导入数据
    std::cout << _("status.importing_data") << ": " << dataFile << std::endl;
    DataSet dataSet = DataManager::getInstance().importFromCSV(dataFile, true);

    if (dataSet.dataPoints.empty() || dataSet.dataPoints.size() < 4) {
        std::cerr << _("error.insufficient_data") << std::endl;
        return;
    }

    // 运行诺依曼趋势测试
    std::cout << _("status.calculating") << std::endl;
    NeumannCalculator calculator;
    NeumannTestResults results = calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

    // 输出结果
    std::cout << "===== " << _("result.test_results") << " =====" << std::endl;
    std::cout << std::endl;

    // 输出表头
    std::cout << _("result.table_header") << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    // 输出测试结果
    for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

        std::cout << results.data[dataIndex] << "\t" << results.timePoints[dataIndex] << "\t"
                  << results.results[i].pgValue << "\t" << results.results[i].wpThreshold << "\t"
                  << (results.results[i].hasTrend ? _("result.has_trend") : _("result.no_trend"))
                  << std::endl;
    }

    // 输出汇总结果
    std::cout << std::endl;
    std::cout << _("result.summary") << std::endl;
    std::cout << _("result.overall_trend") << ": "
              << (results.overallTrend ? _("result.has_trend") : _("result.no_trend")) << std::endl;

    // 测试结论
    std::cout << std::endl;
    std::cout << _("result.conclusion") << std::endl;
    if (results.overallTrend) {
        std::cout << _("result.conclusion_trend") << std::endl;
    } else {
        std::cout << _("result.conclusion_no_trend") << std::endl;
    }

    // 询问是否保存结果到文件
    std::cout << std::endl;
    std::cout << _("prompt.save_results") << " [y/n]: ";
    std::string response;
    std::getline(std::cin, response);

    if (!response.empty() && std::tolower(response[0]) == 'y') {
        std::string outputFile = fs::path(dataFile).stem().string() + "_results.txt";
        std::cout << _("status.saving_results") << ": " << outputFile << std::endl;

        std::ofstream file(outputFile);
        if (file.is_open()) {
            // 写入表头
            file << "===== " << _("result.test_results") << " =====" << std::endl;
            file << std::endl;
            file << _("result.table_header") << std::endl;
            file << "----------------------------------------------------" << std::endl;

            // 写入测试结果
            for (size_t i = 0; i < results.results.size(); ++i) {
                size_t dataIndex = i + 3;  // 从第4个数据点开始计算PG值

                file << results.data[dataIndex] << "\t" << results.timePoints[dataIndex] << "\t"
                     << results.results[i].pgValue << "\t" << results.results[i].wpThreshold << "\t"
                     << (results.results[i].hasTrend ? _("result.has_trend") : _("result.no_trend"))
                     << std::endl;
            }

            // 写入汇总结果
            file << std::endl;
            file << _("result.summary") << std::endl;
            file << _("result.overall_trend") << ": "
                 << (results.overallTrend ? _("result.has_trend") : _("result.no_trend"))
                 << std::endl;

            // 写入测试结论
            file << std::endl;
            file << _("result.conclusion") << std::endl;
            if (results.overallTrend) {
                file << _("result.conclusion_trend") << std::endl;
            } else {
                file << _("result.conclusion_no_trend") << std::endl;
            }

            file.close();
            std::cout << _("status.results_saved") << std::endl;
        } else {
            std::cerr << _("error.file_write_error") << std::endl;
        }
    }
}

}}  // namespace neumann::cli