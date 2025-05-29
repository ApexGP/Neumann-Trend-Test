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

        // 设置Web资源目录
        std::string webRootDir = (releaseDir / "web").string();
        config.setWebRootDirectory(webRootDir);

        // 初始化国际化系统（提前到配置加载之前）
        auto &i18n = neumann::I18n::getInstance();

        // 尝试加载翻译文件（系统级文件，只从config目录加载）
        std::string systemTranslationFile = (fs::path(configDir) / "translations.json").string();
        std::string devTranslationFile = "config/translations.json";  // 开发时的路径
        std::string oldTranslationFile =
            (fs::path(userDataDir) / "translations.json").string();  // 旧位置（兼容性迁移）

        bool translationsLoaded = false;

        if (fs::exists(systemTranslationFile)) {
            if (i18n.loadTranslations(systemTranslationFile)) {
                std::cout << _("startup.translation_system_loaded") << ": " << systemTranslationFile
                          << std::endl;
                translationsLoaded = true;
            }
        }

        if (!translationsLoaded && fs::exists(devTranslationFile)) {
            if (i18n.loadTranslations(devTranslationFile)) {
                std::cout << _("startup.translation_dev_loaded") << ": " << devTranslationFile
                          << std::endl;
                translationsLoaded = true;
            }
        }

        // 兼容性处理：如果发现旧位置的翻译文件，删除它并提示用户
        if (fs::exists(oldTranslationFile)) {
            try {
                fs::remove(oldTranslationFile);
                std::cout << _("startup.translation_old_cleaned") << ": " << oldTranslationFile
                          << std::endl;
            }
            catch (const std::exception &e) {
                std::cout << _("startup.translation_cleanup_failed") << ": " << e.what()
                          << std::endl;
            }
        }

        if (!translationsLoaded) {
            std::cout << _("startup.translation_load_warning") << std::endl;
        }

        // 使用智能配置加载系统（现在翻译系统已经初始化，可以正确显示翻译文本）
        if (!config.loadConfigurationSmart(userDataDir, configDir)) {
            std::cout << _("startup.config_load_warning") << std::endl;
        }

        // 设置语言（从配置文件读取后设置）
        i18n.setLanguage(config.getLanguage());

        // 使用智能系统文件管理
        neumann::Config::manageSystemFilesSmart(userDataDir, refDir, configDir);

        // 确保用户数据目录存在
        if (!fs::exists(userDataDir)) {
            fs::create_directories(userDataDir);
            std::cout << _("startup.user_data_dir_created") << ": " << userDataDir << std::endl;
        }

        // 显示欢迎信息
        if (config.getShowWelcomeMessage()) {
            std::cout << "\n=== " << _("app.title") << " ===" << std::endl;
            std::cout << _("app.description") << std::endl;
            std::cout << _("startup.language_display") << ": "
                      << (i18n.getCurrentLanguage() == neumann::Language::CHINESE ? "中文"
                                                                                  : "English")
                      << std::endl;
            std::cout << _("startup.data_directory") << ": " << userDataDir << std::endl;
            std::cout << std::endl;
        }

        // 加载标准值（优先从用户usr目录，回退到系统目录和开发目录）
        auto &standardValues = neumann::StandardValues::getInstance();
        std::string userStandardValuesFile =
            neumann::Config::getUserSystemFilePath(userDataDir, "standard_values.json");
        std::string systemStandardValuesFile = (fs::path(refDir) / "standard_values.json").string();
        std::string devStandardValuesFile = "ref/standard_values.json";  // 开发时的路径
        std::string oldStandardValuesFile =
            (fs::path(userDataDir) / "standard_values.json").string();  // 旧位置（兼容性）

        if (fs::exists(userStandardValuesFile)) {
            if (!standardValues.loadFromFile(userStandardValuesFile)) {
                std::cout << _("error.standard_values_not_found") << std::endl;
            } else {
                std::cout << _("startup.standard_values_user_loaded") << ": "
                          << userStandardValuesFile << std::endl;
            }
        } else if (fs::exists(systemStandardValuesFile)) {
            if (!standardValues.loadFromFile(systemStandardValuesFile)) {
                std::cout << _("error.standard_values_not_found") << std::endl;
            } else {
                std::cout << _("startup.standard_values_system_loaded") << ": "
                          << systemStandardValuesFile << std::endl;
            }
        } else if (fs::exists(devStandardValuesFile)) {
            if (!standardValues.loadFromFile(devStandardValuesFile)) {
                std::cout << _("error.standard_values_not_found") << std::endl;
            } else {
                std::cout << _("startup.standard_values_dev_loaded") << ": "
                          << devStandardValuesFile << std::endl;
            }
        } else if (fs::exists(oldStandardValuesFile)) {
            if (!standardValues.loadFromFile(oldStandardValuesFile)) {
                std::cout << _("error.standard_values_not_found") << std::endl;
            } else {
                std::cout << _("startup.standard_values_old_loaded") << ": "
                          << oldStandardValuesFile << std::endl;
            }
        } else {
            std::cout << _("error.standard_values_not_found") << std::endl;
        }

        // 设置用户标准值文件路径，确保自定义标准值保存到正确的用户目录
        standardValues.setUserFilePath(userStandardValuesFile);

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
                                                  _("startup.cli_app_start_failed"), e.what());
        errorHandler.displayError(errorInfo, true);
        return 1;
    }
}