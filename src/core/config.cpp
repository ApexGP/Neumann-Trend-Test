#include "core/config.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "core/i18n.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace neumann {

Config::Config()
{
    initializeDefaults();
}

Config &Config::getInstance()
{
    static Config instance;
    return instance;
}

// 将绝对路径转换为相对于可执行文件的相对路径
std::string Config::makeRelativePath(const std::string &absolutePath) const
{
    try {
        if (configFilePath.empty()) {
            return absolutePath;  // 如果没有配置文件路径，返回原始路径
        }

        fs::path configDir = fs::path(configFilePath).parent_path();

        // 根据配置文件位置确定基准目录，与makeAbsolutePath保持一致
        fs::path baseDir;
        if (configDir.filename() == "usr") {
            // 如果是用户配置文件(在data/usr/目录下)，基准目录向上两级到release目录
            baseDir = configDir.parent_path().parent_path();
        } else if (configDir.filename() == "config") {
            // 如果是系统配置文件(在config/目录下)，基准目录向上一级到release目录
            baseDir = configDir.parent_path();
        } else {
            // 开发环境或其他情况，使用当前工作目录
            baseDir = fs::current_path();
        }

        fs::path absPath = fs::absolute(absolutePath);
        fs::path relPath = fs::relative(absPath, baseDir);

        return relPath.string();
    }
    catch (const std::exception &e) {
        // 如果转换失败，返回原始路径
        return absolutePath;
    }
}

// 将相对路径转换为绝对路径
std::string Config::makeAbsolutePath(const std::string &relativePath) const
{
    try {
        if (configFilePath.empty()) {
            return relativePath;  // 如果没有配置文件路径，返回原始路径
        }

        fs::path configDir = fs::path(configFilePath).parent_path();

        // 根据配置文件位置确定基准目录
        fs::path baseDir;
        if (configDir.filename() == "usr") {
            // 如果是用户配置文件(在data/usr/目录下)，基准目录向上两级到release目录
            baseDir = configDir.parent_path().parent_path();
        } else if (configDir.filename() == "config") {
            // 如果是系统配置文件(在config/目录下)，基准目录向上一级到release目录
            baseDir = configDir.parent_path();
        } else {
            // 开发环境或其他情况，使用当前工作目录
            baseDir = fs::current_path();
        }

        fs::path absPath = baseDir / relativePath;
        return fs::absolute(absPath).string();
    }
    catch (const std::exception &e) {
        // 如果转换失败，返回原始路径
        return relativePath;
    }
}

bool Config::loadFromFile(const std::string &filename)
{
    try {
        if (!fs::exists(filename)) {
            std::cout << _("config.file_not_found_creating") << ": " << filename << std::endl;
            return saveToFile(filename);
        }

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开配置文件: " << filename << std::endl;
            return false;
        }

        json data;
        file >> data;

        // 读取配置项
        if (data.contains("language")) {
            language = I18n::stringToLanguage(data["language"].get<std::string>());
        }

        if (data.contains("dataDirectory")) {
            std::string relPath = data["dataDirectory"].get<std::string>();
            // 将相对路径转换为绝对路径
            dataDirectory = makeAbsolutePath(relPath);
        }

        if (data.contains("webRootDirectory")) {
            std::string relPath = data["webRootDirectory"].get<std::string>();
            // 将相对路径转换为绝对路径
            webRootDirectory = makeAbsolutePath(relPath);
        }

        if (data.contains("defaultConfidenceLevel")) {
            defaultConfidenceLevel = data["defaultConfidenceLevel"].get<double>();
        }

        if (data.contains("defaultWebPort")) {
            defaultWebPort = data["defaultWebPort"].get<int>();
        }

        if (data.contains("showWelcomeMessage")) {
            showWelcomeMessage = data["showWelcomeMessage"].get<bool>();
        }

        if (data.contains("enableColorOutput")) {
            enableColorOutput = data["enableColorOutput"].get<bool>();
        }

        if (data.contains("maxDataPoints")) {
            maxDataPoints = data["maxDataPoints"].get<int>();
        }

        if (data.contains("autoSaveResults")) {
            autoSaveResults = data["autoSaveResults"].get<bool>();
        }

        std::cout << _("config.load_success") << ": " << filename << std::endl;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "加载配置文件时出错: " << e.what() << std::endl;
        return false;
    }
}

bool Config::saveToFile(const std::string &filename)
{
    try {
        json data;

        // 保存所有配置项
        data["language"] = I18n::languageToString(language);
        // 将绝对路径转换为相对路径存储
        data["dataDirectory"] = makeRelativePath(dataDirectory);
        data["webRootDirectory"] = makeRelativePath(webRootDirectory);
        data["defaultConfidenceLevel"] = defaultConfidenceLevel;
        data["defaultWebPort"] = defaultWebPort;
        data["showWelcomeMessage"] = showWelcomeMessage;
        data["enableColorOutput"] = enableColorOutput;
        data["maxDataPoints"] = maxDataPoints;
        data["autoSaveResults"] = autoSaveResults;

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法创建配置文件: " << filename << std::endl;
            return false;
        }

        file << data.dump(2);  // 缩进2空格
        file.close();

        std::cout << _("config.save_success") << ": " << filename << std::endl;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "保存配置文件时出错: " << e.what() << std::endl;
        return false;
    }
}

void Config::resetToDefaults()
{
    initializeDefaults();
    std::cout << _("config.reset_to_defaults") << std::endl;
}

void Config::initializeDefaults()
{
    language = Language::CHINESE;
    dataDirectory = "data";
    webRootDirectory = "web";
    defaultConfidenceLevel = 0.95;
    defaultWebPort = 8080;
    showWelcomeMessage = true;
    enableColorOutput = true;
    maxDataPoints = 1000;
    autoSaveResults = true;
}

// Getter方法
Language Config::getLanguage() const
{
    return language;
}
std::string Config::getDataDirectory() const
{
    return dataDirectory;
}
std::string Config::getWebRootDirectory() const
{
    return webRootDirectory;
}
double Config::getDefaultConfidenceLevel() const
{
    return defaultConfidenceLevel;
}
int Config::getDefaultWebPort() const
{
    return defaultWebPort;
}
bool Config::getShowWelcomeMessage() const
{
    return showWelcomeMessage;
}
bool Config::getEnableColorOutput() const
{
    return enableColorOutput;
}
int Config::getMaxDataPoints() const
{
    return maxDataPoints;
}
bool Config::getAutoSaveResults() const
{
    return autoSaveResults;
}
std::string Config::getConfigFilePath() const
{
    return configFilePath;
}

// Setter方法
void Config::setLanguage(Language lang)
{
    language = lang;
    // 同时更新I18n
    I18n::getInstance().setLanguage(lang);
}

void Config::setDataDirectory(const std::string &path)
{
    dataDirectory = path;
}
void Config::setWebRootDirectory(const std::string &path)
{
    webRootDirectory = path;
}
void Config::setDefaultConfidenceLevel(double level)
{
    defaultConfidenceLevel = level;
}
void Config::setDefaultWebPort(int port)
{
    defaultWebPort = port;
}
void Config::setShowWelcomeMessage(bool show)
{
    showWelcomeMessage = show;
}
void Config::setEnableColorOutput(bool enable)
{
    enableColorOutput = enable;
}
void Config::setMaxDataPoints(int max)
{
    maxDataPoints = max;
}
void Config::setAutoSaveResults(bool autoSave)
{
    autoSaveResults = autoSave;
}

void Config::setConfigFilePath(const std::string &path)
{
    configFilePath = path;
}

bool Config::loadConfigurationSmart(const std::string &userDataDir,
                                    const std::string &systemConfigDir)
{
    // 获取用户私有配置文件路径和系统默认配置文件路径
    std::string userConfigPath = getUserConfigPath(userDataDir);
    std::string systemConfigPath = getSystemConfigPath(systemConfigDir);
    std::string devConfigPath = "config/config.json";  // 开发环境配置文件

    std::cout << _("config.smart_loading_start") << std::endl;

    // 优先级1: 尝试加载用户私有配置文件
    if (fs::exists(userConfigPath)) {
        std::cout << _("config.user_private_found") << ": " << userConfigPath << std::endl;
        setConfigFilePath(userConfigPath);
        if (loadFromFile(userConfigPath)) {
            std::cout << _("config.user_private_load_success") << std::endl;
            return true;
        } else {
            std::cout << _("config.user_private_corrupted") << std::endl;
            // 如果用户配置文件损坏，尝试从系统配置重新创建
            if (fs::exists(systemConfigPath) &&
                createUserConfigFromSystem(userConfigPath, systemConfigPath)) {
                if (loadFromFile(userConfigPath)) {
                    std::cout << _("config.user_private_recreated") << std::endl;
                    return true;
                }
            }
        }
    }

    // 优先级2: 如果用户私有配置不存在，尝试从系统配置创建
    if (fs::exists(systemConfigPath)) {
        std::cout << _("config.system_found") << ": " << systemConfigPath << std::endl;
        if (createUserConfigFromSystem(userConfigPath, systemConfigPath)) {
            std::cout << _("config.user_private_created_success") << ": " << userConfigPath
                      << std::endl;
            setConfigFilePath(userConfigPath);
            if (loadFromFile(userConfigPath)) {
                std::cout << _("config.user_private_load_new_success") << std::endl;
                return true;
            }
        } else {
            std::cout << _("config.system_readonly_fallback") << std::endl;
            setConfigFilePath(systemConfigPath);
            if (loadFromFile(systemConfigPath)) {
                std::cout << _("config.system_load_success") << std::endl;
                return true;
            }
        }
    }

    // 优先级3: 开发环境配置文件
    if (fs::exists(devConfigPath)) {
        std::cout << _("config.dev_found") << ": " << devConfigPath << std::endl;
        setConfigFilePath(devConfigPath);
        if (loadFromFile(devConfigPath)) {
            std::cout << _("config.dev_load_success") << std::endl;
            return true;
        }
    }

    // 优先级4: 使用默认配置
    std::cout << _("config.no_config_default") << std::endl;
    setConfigFilePath(userConfigPath);  // 设置为用户配置路径，以便后续保存
    return false;                       // 返回false表示使用默认配置
}

bool Config::saveConfiguration()
{
    if (configFilePath.empty()) {
        std::cerr << "错误: 未设置配置文件路径，无法保存配置" << std::endl;
        return false;
    }
    return saveToFile(configFilePath);
}

std::string Config::getUserConfigPath(const std::string &userDataDir)
{
    return (fs::path(userDataDir) / "usr" / "config_private.json").string();
}

std::string Config::getSystemConfigPath(const std::string &systemConfigDir)
{
    return (fs::path(systemConfigDir) / "config.json").string();
}

bool Config::createUserConfigFromSystem(const std::string &userConfigPath,
                                        const std::string &systemConfigPath)
{
    try {
        // 确保用户配置目录存在
        if (!ensureUserConfigDirectory(userConfigPath)) {
            return false;
        }

        // 读取系统默认配置文件
        if (!fs::exists(systemConfigPath)) {
            std::cerr << "系统配置文件不存在: " << systemConfigPath << std::endl;
            return false;
        }

        // 复制系统配置文件到用户私有配置
        fs::copy_file(systemConfigPath, userConfigPath, fs::copy_options::overwrite_existing);
        std::cout << _("config.user_config_created_from_system") << ": " << userConfigPath
                  << std::endl;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "创建用户私有配置文件失败: " << e.what() << std::endl;
        return false;
    }
}

bool Config::ensureUserConfigDirectory(const std::string &userConfigPath)
{
    try {
        fs::path configDir = fs::path(userConfigPath).parent_path();
        if (!fs::exists(configDir)) {
            fs::create_directories(configDir);
            std::cout << _("config.user_dir_created") << ": " << configDir.string() << std::endl;
        }
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "创建用户配置目录失败: " << e.what() << std::endl;
        return false;
    }
}

bool Config::manageSystemFilesSmart(const std::string &userDataDir, const std::string &systemRefDir,
                                    const std::string &systemConfigDir)
{
    std::cout << _("system.management_start") << std::endl;

    // 确保usr目录存在
    std::string usrDir = (fs::path(userDataDir) / "usr").string();
    try {
        if (!fs::exists(usrDir)) {
            fs::create_directories(usrDir);
            std::cout << _("system.usr_dir_created") << ": " << usrDir << std::endl;
        }
    }
    catch (const std::exception &e) {
        std::cerr << "创建用户系统文件目录失败: " << e.what() << std::endl;
        return false;
    }

    // 系统文件列表及其来源优先级
    struct SystemFileInfo {
        std::string filename;
        std::vector<std::string> sourcePaths;  // 按优先级排序
        bool isRequired;
    };

    std::vector<SystemFileInfo> systemFiles = {{
        "standard_values.json",
        {
            systemRefDir + "/standard_values.json",  // ref/standard_values.json (最高优先级)
            "ref/standard_values.json",              // 开发环境
            userDataDir + "/standard_values.json"    // 旧位置（兼容性）
        },
        true  // 必需文件
    }};

    bool allSuccess = true;

    for (const auto &fileInfo : systemFiles) {
        std::string targetPath = getUserSystemFilePath(userDataDir, fileInfo.filename);
        bool fileManaged = false;

        // 如果目标文件已存在且为最新，跳过
        if (fs::exists(targetPath)) {
            std::cout << _("system.file_already_exists") << ": " << targetPath << std::endl;
            fileManaged = true;
        } else {
            // 按优先级尝试复制源文件
            for (const auto &sourcePath : fileInfo.sourcePaths) {
                if (fs::exists(sourcePath)) {
                    try {
                        fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
                        std::cout << _("system.file_copied_success") << ": " << fileInfo.filename
                                  << " (" << sourcePath << " -> " << targetPath << ")" << std::endl;
                        fileManaged = true;

                        // 如果是从旧位置复制的，删除旧文件（迁移）
                        if (sourcePath == userDataDir + "/" + fileInfo.filename) {
                            try {
                                fs::remove(sourcePath);
                                std::cout << _("system.file_migrated_success") << ": " << sourcePath
                                          << std::endl;
                            }
                            catch (const std::exception &e) {
                                std::cout << _("system.file_delete_failed_ignorable") << ": "
                                          << e.what() << std::endl;
                            }
                        }
                        break;
                    }
                    catch (const std::exception &e) {
                        std::cerr << "复制系统文件失败 " << fileInfo.filename << " (" << sourcePath
                                  << "): " << e.what() << std::endl;
                    }
                }
            }
        }

        if (!fileManaged && fileInfo.isRequired) {
            std::cerr << "必需的系统文件无法管理: " << fileInfo.filename << std::endl;
            allSuccess = false;
        } else if (!fileManaged) {
            std::cout << _("system.optional_file_unavailable_msg") << ": " << fileInfo.filename
                      << std::endl;
        }
    }

    return allSuccess;
}

std::string Config::getUserSystemFilePath(const std::string &userDataDir,
                                          const std::string &filename)
{
    return (fs::path(userDataDir) / "usr" / filename).string();
}

}  // namespace neumann