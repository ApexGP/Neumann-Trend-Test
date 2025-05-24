#pragma once

#include <map>
#include <string>
#include <vector>

namespace neumann {

/**
 * @brief 支持的语言枚举
 */
enum class Language {
    CHINESE,  // 中文
    ENGLISH   // 英文
};

/**
 * @brief 轻量级国际化系统
 *
 * 提供多语言文本支持，支持运行时语言切换
 */
class I18n
{
public:
    /**
     * @brief 获取I18n单例实例
     * @return I18n的共享实例
     */
    static I18n &getInstance();

    /**
     * @brief 设置当前语言
     * @param lang 要设置的语言
     */
    void setLanguage(Language lang);

    /**
     * @brief 获取当前语言
     * @return 当前设置的语言
     */
    Language getCurrentLanguage() const;

    /**
     * @brief 获取翻译文本
     * @param key 文本键
     * @return 对应当前语言的文本，如果找不到则返回键本身
     */
    std::string getText(const std::string &key) const;

    /**
     * @brief 获取格式化翻译文本
     * @param key 文本键
     * @param args 格式化参数
     * @return 格式化后的文本
     */
    template <typename... Args>
    std::string getTextf(const std::string &key, Args... args) const;

    /**
     * @brief 加载翻译数据
     * @param filename JSON翻译文件路径
     * @return 是否成功加载
     */
    bool loadTranslations(const std::string &filename);

    /**
     * @brief 获取所有支持的语言
     * @return 支持的语言列表
     */
    std::vector<Language> getSupportedLanguages() const;

    /**
     * @brief 语言转字符串
     * @param lang 语言枚举
     * @return 语言字符串表示
     */
    static std::string languageToString(Language lang);

    /**
     * @brief 字符串转语言
     * @param str 语言字符串
     * @return 语言枚举
     */
    static Language stringToLanguage(const std::string &str);

private:
    // 私有构造函数
    I18n();

    // 禁用拷贝和赋值
    I18n(const I18n &) = delete;
    I18n &operator=(const I18n &) = delete;

    // 初始化内置翻译
    void initializeBuiltinTranslations();

    // 当前语言
    Language currentLanguage;

    // 翻译数据：map<语言, map<键, 文本>>
    std::map<Language, std::map<std::string, std::string>> translations;
};

// 便利宏定义
#define _(key) neumann::I18n::getInstance().getText(key)
#define _f(key, ...) neumann::I18n::getInstance().getTextf(key, __VA_ARGS__)

}  // namespace neumann