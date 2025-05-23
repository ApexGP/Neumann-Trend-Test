#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "core/standard_values.h"
#include "web/web_server.h"

// Windows专用函数声明
#ifdef _WIN32
extern "C" {
__declspec(dllimport) bool __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
}
#endif

namespace fs = std::filesystem;

// 全局Web服务器实例（用于信号处理）
neumann::web::WebServer *g_server = nullptr;

// 信号处理函数
void signalHandler(int signal)
{
    std::cout << "接收到信号 " << signal << "，正在关闭服务器..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char **argv)
{
    try {
        // 在Windows平台上设置控制台为UTF-8模式
#ifdef _WIN32
        SetConsoleOutputCP(65001);
#endif

        // 注册信号处理器
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // 解析命令行参数
        int port = 8080;                 // 默认端口
        std::string webRootDir = "web";  // 默认Web资源目录
        std::string dataDir = "data";    // 默认数据目录

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
                try {
                    port = std::stoi(argv[++i]);
                }
                catch (...) {
                    std::cerr << "无效的端口号: " << argv[i] << std::endl;
                    return 1;
                }
            } else if ((arg == "-d" || arg == "--dir") && i + 1 < argc) {
                webRootDir = argv[++i];
            } else if ((arg == "--data-dir") && i + 1 < argc) {
                dataDir = argv[++i];
            } else if (arg == "-h" || arg == "--help") {
                std::cout << "诺依曼趋势测试 Web服务器" << std::endl;
                std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
                std::cout << "选项:" << std::endl;
                std::cout << "  -p, --port PORT  设置监听端口 (默认: 8080)" << std::endl;
                std::cout << "  -d, --dir DIR    设置Web资源目录 (默认: web)" << std::endl;
                std::cout << "  --data-dir DIR   设置数据目录 (默认: data)" << std::endl;
                std::cout << "  -h, --help       显示此帮助信息并退出" << std::endl;
                return 0;
            }
        }

        // 尝试获取当前工作目录
        try {
            std::cout << "当前工作目录: " << fs::current_path().string() << std::endl;
        }
        catch (const std::exception &e) {
            std::cerr << "警告: 无法获取当前工作目录: " << e.what() << std::endl;
        }

        // 确保数据目录存在
        if (!fs::exists(dataDir)) {
            std::cout << "数据目录不存在，正在创建: " << dataDir << std::endl;
            try {
                fs::create_directory(dataDir);
            }
            catch (const std::exception &e) {
                std::cerr << "错误: 无法创建数据目录: " << e.what() << std::endl;
                std::cerr << "可能需要管理员权限或当前用户无写入权限" << std::endl;
                return 1;
            }
        }

        // 确保Web资源目录存在
        if (!fs::exists(webRootDir)) {
            std::cout << "Web资源目录不存在，正在创建: " << webRootDir << std::endl;
            try {
                fs::create_directory(webRootDir);
            }
            catch (const std::exception &e) {
                std::cerr << "错误: 无法创建Web资源目录: " << e.what() << std::endl;
                std::cerr << "可能需要管理员权限或当前用户无写入权限" << std::endl;
                return 1;
            }
        }

        // 标准值文件路径
        std::string standardValuesFile = dataDir + "/standard_values.json";

        // 加载标准值 - 注意这会创建文件如果不存在
        if (!neumann::StandardValues::getInstance().loadFromFile(standardValuesFile)) {
            std::cerr << "警告: 无法加载标准值文件，将使用内置默认值。" << std::endl;
        }

        // 创建并启动Web服务器
        std::cout << "初始化Web服务器..." << std::endl;
        neumann::web::WebServer server(port, webRootDir);
        g_server = &server;

        std::cout << std::endl;
        std::cout << "诺依曼趋势测试Web服务器" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << "监听端口: " << port << std::endl;
        std::cout << "Web资源目录: " << webRootDir << std::endl;
        std::cout << "数据目录: " << dataDir << std::endl;
        std::cout << "Web界面访问URL: " << server.getUrl() << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << "按Ctrl+C停止服务器..." << std::endl;
        std::cout << std::endl;

        // 启动服务器（在当前线程中）
        server.start(false);

        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "发生错误: " << e.what() << std::endl;
        return 1;
    }
}