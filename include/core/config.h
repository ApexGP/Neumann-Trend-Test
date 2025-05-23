#pragma once

#include <map>
#include <string>

#include "i18n.h"

namespace neumann {

/**
 * @brief 应用程序配置管理器
 *
 * 管理所有应用程序配置选项，支持加载、保存和运行时修改
 */
class Config
{
public:
    /**
     * @brief 获取Config单例实例
     * @return Config的共享实例
     */
    static Config &getInstance();

    /**
     * @brief 从文件加载配置
     * @param filename 配置文件路径
     * @return 是否成功加载
     */
    bool loadFromFile(const std::string &filename);

    /**
     * @brief 保存配置到文件
     * @param filename 配置文件路径
     * @return 是否成功保存
     */
    bool saveToFile(const std::string &filename);

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults();

    // 语言设置
    Language getLanguage() const;
    void setLanguage(Language lang);

    // 路径设置
    std::string getDataDirectory() const;
    void setDataDirectory(const std::string &path);

    std::string getWebRootDirectory() const;
    void setWebRootDirectory(const std::string &path);

    // 默认参数设置
    double getDefaultConfidenceLevel() const;
    void setDefaultConfidenceLevel(double level);

    int getDefaultWebPort() const;
    void setDefaultWebPort(int port);

    // 界面设置
    bool getShowWelcomeMessage() const;
    void setShowWelcomeMessage(bool show);

    bool getEnableColorOutput() const;
    void setEnableColorOutput(bool enable);

    // 数据处理设置
    int getMaxDataPoints() const;
    void setMaxDataPoints(int max);

    bool getAutoSaveResults() const;
    void setAutoSaveResults(bool autoSave);

    // 获取配置文件路径
    std::string getConfigFilePath() const;

private:
    // 私有构造函数
    Config();

    // 禁用拷贝和赋值
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    // 初始化默认配置
    void initializeDefaults();

    // 配置项
    Language language;
    std::string dataDirectory;
    std::string webRootDirectory;
    double defaultConfidenceLevel;
    int defaultWebPort;
    bool showWelcomeMessage;
    bool enableColorOutput;
    int maxDataPoints;
    bool autoSaveResults;

    // 配置文件路径
    std::string configFilePath;
};

}  // namespace neumann