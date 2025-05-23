#include <filesystem>
#include <iostream>

#include "cli/cli_app.h"
#include "core/config.h"
#include "core/error_handler.h"
#include "core/i18n.h"
#include "core/standard_values.h"

namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    try {
        // 初始化配置系统
        auto &config = neumann::Config::getInstance();

        // 尝试加载配置文件
        std::string configFile = "config.json";
        if (!config.loadFromFile(configFile)) {
            std::cout << "使用默认配置设置" << std::endl;
        }

        // 初始化国际化系统
        auto &i18n = neumann::I18n::getInstance();
        i18n.setLanguage(config.getLanguage());

        // 尝试加载翻译文件
        std::string translationFile = config.getDataDirectory() + "/translations.json";
        if (fs::exists(translationFile)) {
            i18n.loadTranslations(translationFile);
        }

        // 确保数据目录存在
        std::string dataDir = config.getDataDirectory();
        if (!fs::exists(dataDir)) {
            fs::create_directories(dataDir);
            std::cout << _("status.loading") << " " << dataDir << std::endl;
        }

        // 显示欢迎信息
        if (config.getShowWelcomeMessage()) {
            std::cout << "\n=== " << _("app.title") << " ===" << std::endl;
            std::cout << _("app.description") << std::endl;
            std::cout << "语言/Language: "
                      << (i18n.getCurrentLanguage() == neumann::Language::CHINESE ? "中文"
                                                                                  : "English")
                      << std::endl;
            std::cout << std::endl;
        }

        // 加载标准值
        auto &standardValues = neumann::StandardValues::getInstance();
        std::string standardValuesFile = dataDir + "/standard_values.json";
        if (!standardValues.loadFromFile(standardValuesFile)) {
            std::cout << _("error.standard_values_not_found") << std::endl;
        }

        // 创建并运行CLI应用
        neumann::cli::CLIApp app;
        return app.run(argc, argv);
    }
    catch (const neumann::NeumannException &e) {
        // 使用新的错误处理系统
        auto &errorHandler = neumann::ErrorHandler::getInstance();
        errorHandler.displayError(e.getErrorInfo(), true);
        return static_cast<int>(e.getErrorCode());
    }
    catch (const std::exception &e) {
        // 处理其他标准异常
        auto &errorHandler = neumann::ErrorHandler::getInstance();
        auto errorInfo = errorHandler.handleError(neumann::ErrorCode::UNKNOWN_ERROR,
                                                  "CLI应用程序启动失败", e.what());
        errorHandler.displayError(errorInfo, true);
        return 1;
    }
}