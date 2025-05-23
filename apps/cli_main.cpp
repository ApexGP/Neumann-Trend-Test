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
        // 获取可执行文件所在目录
        fs::path exePath = fs::canonical(argv[0]);
        fs::path exeDir = exePath.parent_path();
        fs::path releaseDir = exeDir.parent_path();  // 从bin目录上升到release目录

        // 在release文件结构下的路径设置
        std::string userDataDir = (releaseDir / "data").string();
        std::string configDir = (releaseDir / "config").string();
        std::string refDir = (releaseDir / "ref").string();

        // 初始化配置系统
        auto &config = neumann::Config::getInstance();

        // 设置用户数据目录
        config.setDataDirectory(userDataDir);

        // 确保config目录存在
        if (!fs::exists(configDir)) {
            fs::create_directories(configDir);
            std::cout << "创建配置目录: " << configDir << std::endl;
        }

        // 尝试加载配置文件（优先从config目录，回退到开发目录）
        std::string configFile = (fs::path(configDir) / "config.json").string();
        std::string fallbackConfigFile = (exeDir / "config.json").string();

        if (fs::exists(configFile)) {
            if (!config.loadFromFile(configFile)) {
                std::cout << "警告: 配置文件加载失败，使用默认配置" << std::endl;
            } else {
                std::cout << "已加载配置文件: " << configFile << std::endl;
            }
            // 设置配置文件路径，用于后续保存
            config.setConfigFilePath(configFile);
        } else if (fs::exists(fallbackConfigFile)) {
            if (!config.loadFromFile(fallbackConfigFile)) {
                std::cout << "警告: 配置文件加载失败，使用默认配置" << std::endl;
            } else {
                std::cout << "已加载开发配置文件: " << fallbackConfigFile << std::endl;
            }
            // 设置配置文件路径，用于后续保存
            config.setConfigFilePath(fallbackConfigFile);
        } else {
            std::cout << "使用默认配置设置" << std::endl;
            // 即使没有配置文件，也要设置预期的保存路径
            config.setConfigFilePath(configFile);
        }

        // 确保用户数据目录存在
        if (!fs::exists(userDataDir)) {
            fs::create_directories(userDataDir);
            std::cout << "创建用户数据目录: " << userDataDir << std::endl;
        }

        // 复制必要的系统文件到用户数据目录
        std::vector<std::string> systemFiles = {"standard_values.json", "translations.json"};
        for (const auto &fileName : systemFiles) {
            fs::path sourceFile = exeDir / "data" / fileName;
            fs::path targetFile = fs::path(userDataDir) / fileName;

            // 如果源文件存在但目标文件不存在，则复制
            if (fs::exists(sourceFile) && !fs::exists(targetFile)) {
                try {
                    fs::copy_file(sourceFile, targetFile);
                    std::cout << "复制系统文件: " << fileName << std::endl;
                }
                catch (const std::exception &e) {
                    std::cout << "复制文件失败 " << fileName << ": " << e.what() << std::endl;
                }
            }
        }

        // 初始化国际化系统
        auto &i18n = neumann::I18n::getInstance();
        i18n.setLanguage(config.getLanguage());

        // 尝试加载翻译文件（优先从config目录，回退到开发目录）
        std::string translationFile = (fs::path(configDir) / "translations.json").string();
        std::string fallbackTranslationFile = "data/translations.json";

        bool translationsLoaded = false;

        if (fs::exists(translationFile)) {
            if (i18n.loadTranslations(translationFile)) {
                std::cout << "已加载翻译文件: " << translationFile << std::endl;
                translationsLoaded = true;
            }
        }

        if (!translationsLoaded && fs::exists(fallbackTranslationFile)) {
            if (i18n.loadTranslations(fallbackTranslationFile)) {
                std::cout << "已加载开发翻译文件: " << fallbackTranslationFile << std::endl;
                translationsLoaded = true;
            }
        }

        if (!translationsLoaded) {
            std::cout << "警告: 翻译文件加载失败，使用内置翻译" << std::endl;
        }

        // 显示欢迎信息
        if (config.getShowWelcomeMessage()) {
            std::cout << "\n=== " << _("app.title") << " ===" << std::endl;
            std::cout << _("app.description") << std::endl;
            std::cout << "语言/Language: "
                      << (i18n.getCurrentLanguage() == neumann::Language::CHINESE ? "中文"
                                                                                  : "English")
                      << std::endl;
            std::cout << "数据目录: " << userDataDir << std::endl;
            std::cout << std::endl;
        }

        // 加载标准值（优先从ref目录，回退到开发目录）
        auto &standardValues = neumann::StandardValues::getInstance();
        std::string standardValuesFile = (fs::path(refDir) / "standard_values.json").string();
        std::string fallbackStandardValuesFile = "data/standard_values.json";

        if (fs::exists(standardValuesFile)) {
            if (!standardValues.loadFromFile(standardValuesFile)) {
                std::cout << _("error.standard_values_not_found") << std::endl;
            } else {
                std::cout << "已加载标准值文件: " << standardValuesFile << std::endl;
            }
        } else if (fs::exists(fallbackStandardValuesFile)) {
            if (!standardValues.loadFromFile(fallbackStandardValuesFile)) {
                std::cout << _("error.standard_values_not_found") << std::endl;
            } else {
                std::cout << "已加载开发标准值文件: " << fallbackStandardValuesFile << std::endl;
            }
        } else {
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