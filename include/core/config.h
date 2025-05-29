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

    // 设置配置文件路径
    void setConfigFilePath(const std::string &path);

    /**
     * @brief 智能加载配置文件
     * @param userDataDir 用户数据目录
     * @param systemConfigDir 系统配置目录
     * @return 是否成功加载配置
     */
    bool loadConfigurationSmart(const std::string &userDataDir, const std::string &systemConfigDir);

    /**
     * @brief 保存配置到当前配置文件路径
     * @return 是否成功保存
     */
    bool saveConfiguration();

    /**
     * @brief 获取用户私有配置文件路径
     * @param userDataDir 用户数据目录
     * @return 用户私有配置文件路径
     */
    static std::string getUserConfigPath(const std::string &userDataDir);

    /**
     * @brief 获取系统默认配置文件路径
     * @param systemConfigDir 系统配置目录
     * @return 系统默认配置文件路径
     */
    static std::string getSystemConfigPath(const std::string &systemConfigDir);

    /**
     * @brief 智能管理系统文件（标准值、翻译文件等）
     * @param userDataDir 用户数据目录
     * @param systemRefDir 系统参考文件目录
     * @param systemConfigDir 系统配置目录
     * @return 是否成功管理所有系统文件
     */
    static bool manageSystemFilesSmart(const std::string &userDataDir,
                                       const std::string &systemRefDir,
                                       const std::string &systemConfigDir);

    /**
     * @brief 获取用户系统文件路径
     * @param userDataDir 用户数据目录
     * @param filename 文件名
     * @return 用户系统文件路径
     */
    static std::string getUserSystemFilePath(const std::string &userDataDir,
                                             const std::string &filename);

private:
    // 私有构造函数
    Config();

    // 禁用拷贝和赋值
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    // 初始化默认配置
    void initializeDefaults();

    // 路径转换辅助方法
    std::string makeRelativePath(const std::string &absolutePath) const;
    std::string makeAbsolutePath(const std::string &relativePath) const;

    /**
     * @brief 创建用户私有配置文件
     * @param userConfigPath 用户私有配置文件路径
     * @param systemConfigPath 系统默认配置文件路径
     * @return 是否成功创建
     */
    bool createUserConfigFromSystem(const std::string &userConfigPath,
                                    const std::string &systemConfigPath);

    /**
     * @brief 确保用户配置目录存在
     * @param userConfigPath 用户配置文件路径
     * @return 是否成功创建目录
     */
    bool ensureUserConfigDirectory(const std::string &userConfigPath);

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