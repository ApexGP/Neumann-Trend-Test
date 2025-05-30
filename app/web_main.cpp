#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "core/config.h"
#include "core/i18n.h"
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
    auto &i18n = neumann::I18n::getInstance();
    std::cout << i18n.getTextf("web.app.signal_received", std::to_string(signal)) << std::endl;
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

        // 获取可执行文件所在目录
        fs::path exePath = fs::canonical(argv[0]);
        fs::path exeDir = exePath.parent_path();
        fs::path releaseDir = exeDir.parent_path();  // 从bin目录上升到release目录

        // 初始化配置系统和国际化系统
        auto &config = neumann::Config::getInstance();
        auto &i18n = neumann::I18n::getInstance();

        // 设置数据和配置目录路径
        std::string userDataDir = (releaseDir / "data").string();
        std::string configDir = (releaseDir / "config").string();
        std::string refDir = (releaseDir / "ref").string();

        config.setDataDirectory(userDataDir);

        // 尝试加载翻译文件
        std::string systemTranslationFile = (fs::path(configDir) / "translations.json").string();
        std::string devTranslationFile = "config/translations.json";

        bool translationsLoaded = false;
        if (fs::exists(systemTranslationFile)) {
            if (i18n.loadTranslations(systemTranslationFile)) {
                translationsLoaded = true;
            }
        } else if (fs::exists(devTranslationFile)) {
            if (i18n.loadTranslations(devTranslationFile)) {
                translationsLoaded = true;
            }
        }

        // 加载配置
        config.loadConfigurationSmart(userDataDir, configDir);
        i18n.setLanguage(config.getLanguage());

        // 解析命令行参数
        int port = 8080;                                         // 默认端口
        std::string webRootDir = (releaseDir / "web").string();  // release文件结构下的Web资源目录
        std::string dataDir = (releaseDir / "data").string();    // release文件结构下的数据目录

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
                try {
                    port = std::stoi(argv[++i]);
                }
                catch (...) {
                    std::cerr << i18n.getText("web.app.invalid_port") << ": " << argv[i]
                              << std::endl;
                    return 1;
                }
            } else if ((arg == "-d" || arg == "--dir") && i + 1 < argc) {
                webRootDir = argv[++i];
            } else if ((arg == "--data-dir") && i + 1 < argc) {
                dataDir = argv[++i];
            } else if (arg == "-h" || arg == "--help") {
                std::cout << i18n.getText("web.app.title") << std::endl;
                std::cout << i18n.getTextf("web.app.help_usage", argv[0]) << std::endl;
                std::cout << i18n.getText("web.app.help_options") << std::endl;
                std::cout << i18n.getText("web.app.help_port") << std::endl;
                std::cout << i18n.getText("web.app.help_dir") << std::endl;
                std::cout << i18n.getText("web.app.help_data_dir") << std::endl;
                std::cout << i18n.getText("web.app.help_help") << std::endl;
                return 0;
            }
        }

        // 尝试获取当前工作目录
        try {
            std::cout << i18n.getText("web.app.current_directory") << ": "
                      << fs::current_path().string() << std::endl;
        }
        catch (const std::exception &e) {
            std::cerr << i18n.getText("web.app.directory_get_warning") << ": " << e.what()
                      << std::endl;
        }

        // 确保数据目录存在
        if (!fs::exists(dataDir)) {
            std::cout << i18n.getText("web.app.data_directory_missing") << ": " << dataDir
                      << std::endl;
            try {
                fs::create_directory(dataDir);
            }
            catch (const std::exception &e) {
                std::cerr << i18n.getText("web.app.data_directory_create_error") << ": " << e.what()
                          << std::endl;
                std::cerr << i18n.getText("web.app.data_directory_permission_warning") << std::endl;
                return 1;
            }
        }

        // 检查Web资源目录（如果不存在，尝试使用开发时的web目录）
        if (!fs::exists(webRootDir)) {
            std::string fallbackWebDir = "web";
            if (fs::exists(fallbackWebDir)) {
                webRootDir = fallbackWebDir;
                std::cout << i18n.getText("web.app.fallback_web_directory") << ": " << webRootDir
                          << std::endl;
            } else {
                std::cout << i18n.getText("web.app.web_directory_missing") << ": " << webRootDir
                          << std::endl;
                try {
                    fs::create_directory(webRootDir);
                }
                catch (const std::exception &e) {
                    std::cerr << i18n.getText("web.app.web_directory_create_error") << ": "
                              << e.what() << std::endl;
                    std::cerr << i18n.getText("web.app.web_directory_permission_warning")
                              << std::endl;
                    return 1;
                }
            }
        }

        // 标准值文件路径（使用智能系统文件管理）
        std::string userStandardValuesFile =
            (fs::path(dataDir) / "usr" / "standard_values.json").string();
        std::string systemStandardValuesFile = (fs::path(refDir) / "standard_values.json").string();
        std::string devStandardValuesFile = "ref/standard_values.json";  // 开发时的路径

        auto &standardValues = neumann::StandardValues::getInstance();

        // 按优先级加载标准值文件
        if (fs::exists(userStandardValuesFile)) {
            if (!standardValues.loadFromFile(userStandardValuesFile)) {
                std::cerr << i18n.getText("web.app.user_standard_values_load_warning") << std::endl;
            } else {
                std::cout << i18n.getText("web.app.user_standard_values_loaded") << ": "
                          << userStandardValuesFile << std::endl;
            }
        } else if (fs::exists(systemStandardValuesFile)) {
            if (!standardValues.loadFromFile(systemStandardValuesFile)) {
                std::cerr << i18n.getText("web.app.system_standard_values_load_warning")
                          << std::endl;
            } else {
                std::cout << i18n.getText("web.app.system_standard_values_loaded") << ": "
                          << systemStandardValuesFile << std::endl;
            }
        } else if (fs::exists(devStandardValuesFile)) {
            if (!standardValues.loadFromFile(devStandardValuesFile)) {
                std::cerr << i18n.getText("web.app.dev_standard_values_load_warning") << std::endl;
            } else {
                std::cout << i18n.getText("web.app.dev_standard_values_loaded") << ": "
                          << devStandardValuesFile << std::endl;
            }
        } else {
            std::cerr << i18n.getText("web.app.standard_values_not_found") << std::endl;
        }

        // 设置用户标准值文件路径，确保自定义标准值保存到正确的用户目录
        standardValues.setUserFilePath(userStandardValuesFile);

        // 创建并启动Web服务器
        std::cout << i18n.getText("web.app.initializing_web_server") << std::endl;
        neumann::web::WebServer server(port, webRootDir);
        g_server = &server;

        std::cout << std::endl;
        std::cout << i18n.getText("web.app.title") << std::endl;
        std::cout << i18n.getText("web.app.server_info_header") << std::endl;
        std::cout << i18n.getText("web.app.port") << ": " << port << std::endl;
        std::cout << i18n.getText("web.app.web_directory") << ": " << webRootDir << std::endl;
        std::cout << i18n.getText("web.app.data_directory") << ": " << dataDir << std::endl;
        std::cout << i18n.getText("web.app.web_url") << ": " << server.getUrl() << std::endl;
        std::cout << i18n.getText("web.app.server_info_footer") << std::endl;
        std::cout << i18n.getText("web.app.press_ctrl_c_stop") << std::endl;
        std::cout << std::endl;

        // 启动服务器（在当前线程中）
        server.start(false);

        return 0;
    }
    catch (const std::exception &e) {
        auto &i18n = neumann::I18n::getInstance();
        std::cerr << i18n.getText("web.app.error_occurred") << ": " << e.what() << std::endl;
        return 1;
    }
}