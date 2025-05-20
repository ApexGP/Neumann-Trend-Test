#include "cli/cli_app.h"
#include "core/standard_values.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  try {
    // 确保数据目录存在
    if (!fs::exists("data")) {
      fs::create_directory("data");
    }

    // 创建并运行CLI应用
    neumann::cli::CLIApp app;
    return app.run(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << "发生错误: " << e.what() << std::endl;
    return 1;
  }
}