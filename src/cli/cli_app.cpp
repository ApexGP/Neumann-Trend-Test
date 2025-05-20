#include "cli/cli_app.h"
#include "core/data_manager.h"
#include "core/neumann_calculator.h"
#include "core/standard_values.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace neumann {
namespace cli {

CLIApp::CLIApp() {}

int CLIApp::run(int argc, char **argv) {
  // 处理命令行参数
  if (argc > 1) {
    if (processArgs(argc, argv)) {
      return 0;
    }
  }

  // 加载标准值
  if (!StandardValues::getInstance().loadFromFile(
          "data/standard_values.json")) {
    std::cerr << "警告: 无法加载标准值文件，将使用内置默认值。" << std::endl;
  }

  // 运行终端UI
  ui.run();

  return 0;
}

bool CLIApp::processArgs(int argc, char **argv) {
  std::string arg = argv[1];

  if (arg == "-h" || arg == "--help") {
    showHelp();
    return true;
  } else if (arg == "-v" || arg == "--version") {
    showVersion();
    return true;
  } else if (arg == "-f" || arg == "--file") {
    if (argc < 3) {
      std::cerr << "错误: 缺少文件路径参数。" << std::endl;
      showHelp();
      return true;
    }

    runWithData(argv[2]);
    return true;
  }

  return false;
}

void CLIApp::showHelp() {
  std::cout << "诺依曼趋势测试工具 v1.0.0" << std::endl;
  std::cout << "用法: neumann [选项]" << std::endl;
  std::cout << std::endl;
  std::cout << "选项:" << std::endl;
  std::cout << "  -h, --help       显示此帮助信息并退出" << std::endl;
  std::cout << "  -v, --version    显示版本信息并退出" << std::endl;
  std::cout << "  -f, --file PATH  处理指定路径的CSV数据文件" << std::endl;
  std::cout << std::endl;
  std::cout << "示例:" << std::endl;
  std::cout << "  neumann              启动交互式终端界面" << std::endl;
  std::cout << "  neumann -f data.csv  处理data.csv文件" << std::endl;
}

void CLIApp::showVersion() {
  std::cout << "诺依曼趋势测试工具 v1.0.0" << std::endl;
  std::cout << "Copyright © 2023" << std::endl;
}

void CLIApp::runWithData(const std::string &dataFile) {
  // 检查文件是否存在
  if (!fs::exists(dataFile)) {
    std::cerr << "错误: 文件不存在: " << dataFile << std::endl;
    return;
  }

  // 导入数据
  std::cout << "正在从文件导入数据: " << dataFile << std::endl;
  DataSet dataSet = DataManager::getInstance().importFromCSV(dataFile, true);

  if (dataSet.dataPoints.empty() || dataSet.dataPoints.size() < 4) {
    std::cerr << "错误: 数据点不足，需要至少4个数据点。" << std::endl;
    return;
  }

  // 运行诺依曼趋势测试
  std::cout << "正在运行诺依曼趋势测试..." << std::endl;
  NeumannCalculator calculator;
  NeumannTestResults results =
      calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

  // 输出结果
  std::cout << "===== 诺依曼趋势测试结果 =====" << std::endl;
  std::cout << std::endl;

  // 输出表头
  std::cout << "数据点\t时间点\tPG值\tW(P)阈值\t趋势判断" << std::endl;
  std::cout << "----------------------------------------------------"
            << std::endl;

  // 输出测试结果
  for (size_t i = 0; i < results.results.size(); ++i) {
    size_t dataIndex = i + 3; // 从第4个数据点开始计算PG值

    std::cout << results.data[dataIndex] << "\t"
              << results.timePoints[dataIndex] << "\t"
              << results.results[i].pgValue << "\t"
              << results.results[i].wpThreshold << "\t"
              << (results.results[i].hasTrend ? "是" : "否") << std::endl;
  }

  // 输出汇总结果
  std::cout << std::endl;
  std::cout << "汇总结果:" << std::endl;
  std::cout << "整体是否存在趋势: " << (results.overallTrend ? "是" : "否")
            << std::endl;

  // 测试结论
  std::cout << std::endl;
  std::cout << "诺依曼趋势测试结论:" << std::endl;
  if (results.overallTrend) {
    std::cout
        << "数据存在显著趋势，可能表明测试样品在所测量的时间内存在系统性变化。"
        << std::endl;
  } else {
    std::cout << "数据无显著趋势，测试样品在所测量的时间内保持稳定。"
              << std::endl;
  }

  // 询问是否保存结果到文件
  std::cout << std::endl;
  std::cout << "是否将结果保存到文件? [y/n]: ";
  std::string response;
  std::getline(std::cin, response);

  if (!response.empty() && std::tolower(response[0]) == 'y') {
    std::string outputFile =
        fs::path(dataFile).stem().string() + "_results.txt";
    std::cout << "正在保存结果到: " << outputFile << std::endl;

    std::ofstream file(outputFile);
    if (file.is_open()) {
      // 写入表头
      file << "===== 诺依曼趋势测试结果 =====" << std::endl;
      file << std::endl;
      file << "数据点\t时间点\tPG值\tW(P)阈值\t趋势判断" << std::endl;
      file << "----------------------------------------------------"
           << std::endl;

      // 写入测试结果
      for (size_t i = 0; i < results.results.size(); ++i) {
        size_t dataIndex = i + 3; // 从第4个数据点开始计算PG值

        file << results.data[dataIndex] << "\t" << results.timePoints[dataIndex]
             << "\t" << results.results[i].pgValue << "\t"
             << results.results[i].wpThreshold << "\t"
             << (results.results[i].hasTrend ? "是" : "否") << std::endl;
      }

      // 写入汇总结果
      file << std::endl;
      file << "汇总结果:" << std::endl;
      file << "整体是否存在趋势: " << (results.overallTrend ? "是" : "否")
           << std::endl;

      // 写入测试结论
      file << std::endl;
      file << "诺依曼趋势测试结论:" << std::endl;
      if (results.overallTrend) {
        file << "数据存在显著趋势，可能表明测试样品在所测量的时间内存在系统性变"
                "化。"
             << std::endl;
      } else {
        file << "数据无显著趋势，测试样品在所测量的时间内保持稳定。"
             << std::endl;
      }

      file.close();
      std::cout << "结果已保存。" << std::endl;
    } else {
      std::cerr << "无法创建输出文件。" << std::endl;
    }
  }
}

} // namespace cli
} // namespace neumann