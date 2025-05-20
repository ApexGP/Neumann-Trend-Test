#include "core/standard_values.h"
#include "web/web_server.h"
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

namespace fs = std::filesystem;

// 全局Web服务器实例（用于信号处理）
neumann::web::WebServer *g_server = nullptr;

// 信号处理函数
void signalHandler(int signal) {
  std::cout << "接收到信号 " << signal << "，正在关闭服务器..." << std::endl;
  if (g_server) {
    g_server->stop();
  }
}

int main(int argc, char **argv) {
  try {
    // 注册信号处理器
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // 解析命令行参数
    int port = 8080;                // 默认端口
    std::string webRootDir = "web"; // 默认Web资源目录

    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
        try {
          port = std::stoi(argv[++i]);
        } catch (...) {
          std::cerr << "无效的端口号: " << argv[i] << std::endl;
          return 1;
        }
      } else if ((arg == "-d" || arg == "--dir") && i + 1 < argc) {
        webRootDir = argv[++i];
      } else if (arg == "-h" || arg == "--help") {
        std::cout << "诺依曼趋势测试 Web服务器" << std::endl;
        std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
        std::cout << "选项:" << std::endl;
        std::cout << "  -p, --port PORT  设置监听端口 (默认: 8080)"
                  << std::endl;
        std::cout << "  -d, --dir DIR    设置Web资源目录 (默认: web)"
                  << std::endl;
        std::cout << "  -h, --help       显示此帮助信息并退出" << std::endl;
        return 0;
      }
    }

    // 确保数据目录存在
    if (!fs::exists("data")) {
      fs::create_directory("data");
    }

    // 加载标准值
    if (!neumann::StandardValues::getInstance().loadFromFile(
            "data/standard_values.json")) {
      std::cerr << "警告: 无法加载标准值文件，将使用内置默认值。" << std::endl;
    }

    // 创建并启动Web服务器
    neumann::web::WebServer server(port, webRootDir);
    g_server = &server;

    std::cout << "诺依曼趋势测试Web服务器" << std::endl;
    std::cout << "监听端口: " << port << std::endl;
    std::cout << "Web资源目录: " << webRootDir << std::endl;
    std::cout << "Web界面访问URL: " << server.getUrl() << std::endl;
    std::cout << "按Ctrl+C停止服务器..." << std::endl;

    // 启动服务器（在当前线程中）
    server.start(false);

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "发生错误: " << e.what() << std::endl;
    return 1;
  }
}