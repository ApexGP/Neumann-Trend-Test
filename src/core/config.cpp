#include "core/config.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace neumann {

Config::Config()
{
    initializeDefaults();

    // 设置配置文件路径
    configFilePath = "config.json";
}

Config &Config::getInstance()
{
    static Config instance;
    return instance;
}

bool Config::loadFromFile(const std::string &filename)
{
    try {
        if (!fs::exists(filename)) {
            std::cout << "配置文件不存在，创建默认配置: " << filename << std::endl;
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
            dataDirectory = data["dataDirectory"].get<std::string>();
        }

        if (data.contains("webRootDirectory")) {
            webRootDirectory = data["webRootDirectory"].get<std::string>();
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

        std::cout << "成功加载配置文件: " << filename << std::endl;
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
        data["dataDirectory"] = dataDirectory;
        data["webRootDirectory"] = webRootDirectory;
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

        std::cout << "成功保存配置文件: " << filename << std::endl;
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
    std::cout << "配置已重置为默认值" << std::endl;
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

}  // namespace neumann