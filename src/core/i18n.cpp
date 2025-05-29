#include "core/i18n.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace neumann {

I18n::I18n() : currentLanguage(Language::CHINESE)
{
    initializeBuiltinTranslations();
}

I18n &I18n::getInstance()
{
    static I18n instance;
    return instance;
}

void I18n::setLanguage(Language lang)
{
    currentLanguage = lang;
}

Language I18n::getCurrentLanguage() const
{
    return currentLanguage;
}

std::string I18n::getText(const std::string &key) const
{
    auto langIt = translations.find(currentLanguage);
    if (langIt != translations.end()) {
        auto textIt = langIt->second.find(key);
        if (textIt != langIt->second.end()) {
            return textIt->second;
        }
    }

    // 如果当前语言没有找到，尝试英文作为后备
    if (currentLanguage != Language::ENGLISH) {
        auto englishIt = translations.find(Language::ENGLISH);
        if (englishIt != translations.end()) {
            auto textIt = englishIt->second.find(key);
            if (textIt != englishIt->second.end()) {
                return textIt->second;
            }
        }
    }

// 如果都没找到，输出调试信息（仅在调试模式下）
#ifdef DEBUG
    std::cerr << "Translation not found for key: " << key
              << ", current language: " << static_cast<int>(currentLanguage) << std::endl;
#endif

    // 如果都没找到，返回键本身
    return key;
}

template <typename... Args>
std::string I18n::getTextf(const std::string &key, Args... args) const
{
    std::string format = getText(key);

    // 简单的格式化实现：将{0}, {1}, {2}等替换为对应的参数
    std::vector<std::string> argStrings;

    // 将所有参数转换为字符串
    auto toString = [](const auto &arg) {
        std::ostringstream oss;
        oss << arg;
        return oss.str();
    };

    // 使用折叠表达式将参数转换为字符串并添加到vector中
    (argStrings.push_back(toString(args)), ...);

    // 替换格式字符串中的占位符
    std::string result = format;
    for (size_t i = 0; i < argStrings.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), argStrings[i]);
            pos += argStrings[i].length();
        }
    }

    return result;
}

bool I18n::loadTranslations(const std::string &filename)
{
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开翻译文件: " << filename << std::endl;
            return false;
        }

        json data;
        file >> data;

        // 解析翻译数据
        for (auto &langItem : data.items()) {
            Language lang = stringToLanguage(langItem.key());

            for (auto &textItem : langItem.value().items()) {
                translations[lang][textItem.key()] = textItem.value().get<std::string>();
            }
        }

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "加载翻译文件时出错: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Language> I18n::getSupportedLanguages() const
{
    return {Language::CHINESE, Language::ENGLISH};
}

std::string I18n::languageToString(Language lang)
{
    switch (lang) {
        case Language::CHINESE:
            return "zh";
        case Language::ENGLISH:
            return "en";
        default:
            return "zh";
    }
}

Language I18n::stringToLanguage(const std::string &str)
{
    if (str == "en" || str == "english" || str == "English") {
        return Language::ENGLISH;
    }
    return Language::CHINESE;  // 默认中文
}

void I18n::initializeBuiltinTranslations()
{
    // 中文翻译
    auto &zh = translations[Language::CHINESE];
    zh["app.title"] = "诺依曼趋势测试工具";
    zh["app.description"] = "用于评估数据集是否存在系统性趋势的统计工具";
    zh["menu.main"] = "主菜单";
    zh["menu.new_test"] = "运行新的诺依曼趋势测试";
    zh["menu.load_data"] = "加载数据集";
    zh["menu.import_csv"] = "从CSV导入数据";
    zh["menu.import_excel"] = "从Excel导入数据";
    zh["menu.help"] = "帮助";
    zh["menu.about"] = "关于";
    zh["menu.exit"] = "退出";
    zh["menu.settings"] = "设置";
    zh["menu.language"] = "语言设置";
    zh["menu.back"] = "返回";

    zh["input.data_points"] = "请输入数据点 (用空格分隔):";
    zh["input.time_points"] = "请输入时间点 (用空格分隔):";
    zh["input.confidence_level"] = "请选择置信水平:";
    zh["input.filename"] = "请输入文件路径";
    zh["input.exit_hint"] = "输入 'q' 或直接按回车退出";

    zh["load.no_datasets"] = "没有可用的数据集。";
    zh["load.available_datasets"] = "可用数据集:";
    zh["load.select_dataset"] = "请选择数据集";

    zh["prompt.press_enter"] = "按Enter键继续...";
    zh["prompt.select_option"] = "请选择操作";

    zh["result.no_trend"] = "无趋势";
    zh["result.has_trend"] = "有趋势";
    zh["result.pg_value"] = "PG值";
    zh["result.threshold"] = "阈值";
    zh["result.overall_trend"] = "整体趋势";

    // 错误消息
    zh["error.file_not_found"] = "文件未找到";
    zh["error.file_read_error"] = "文件读取失败";
    zh["error.file_write_error"] = "文件写入失败";
    zh["error.file_permission_error"] = "文件权限错误";
    zh["error.invalid_data"] = "无效的数据格式";
    zh["error.insufficient_data"] = "数据点不足，至少需要4个数据点";
    zh["error.data_out_of_range"] = "数据超出范围";
    zh["error.data_parsing_error"] = "数据解析错误";
    zh["error.invalid_confidence_level"] = "无效的置信水平";
    zh["error.calculation_failed"] = "计算失败";
    zh["error.standard_values_not_found"] = "未找到标准值";
    zh["error.network_error"] = "网络错误";
    zh["error.port_in_use"] = "端口已被使用";
    zh["error.web_server_error"] = "Web服务器错误";
    zh["error.config_load_error"] = "配置加载失败";
    zh["error.config_save_error"] = "配置保存失败";
    zh["error.invalid_config_value"] = "无效的配置值";
    zh["error.memory_error"] = "内存错误";
    zh["error.system_error"] = "系统错误";
    zh["error.permission_denied"] = "权限被拒绝";
    zh["error.unknown"] = "未知错误";

    // 建议消息
    zh["suggestion.title"] = "建议";
    zh["suggestion.check_file_path"] = "请检查文件路径是否正确";
    zh["suggestion.check_file_permissions"] = "请检查文件读取权限";
    zh["suggestion.check_write_permissions"] = "请检查文件写入权限";
    zh["suggestion.run_as_admin"] = "请以管理员身份运行";
    zh["suggestion.check_data_format"] = "请检查数据格式是否正确";
    zh["suggestion.add_more_data"] = "请添加更多数据点";
    zh["suggestion.check_data_values"] = "请检查数据值是否在有效范围内";
    zh["suggestion.fix_data_format"] = "请修正数据格式";
    zh["suggestion.use_valid_confidence"] = "请使用有效的置信水平 (0.90, 0.95, 0.975, 0.99)";
    zh["suggestion.check_input_data"] = "请检查输入数据";
    zh["suggestion.check_standard_values_file"] = "请检查标准值文件是否存在";
    zh["suggestion.check_network"] = "请检查网络连接";
    zh["suggestion.try_different_port"] = "请尝试使用其他端口";
    zh["suggestion.restart_server"] = "请重启服务器";
    zh["suggestion.reset_config"] = "请重置配置为默认值";
    zh["suggestion.reduce_data_size"] = "请减少数据大小";
    zh["suggestion.contact_support"] = "请联系技术支持";

    zh["status.loading"] = "加载中...";
    zh["status.calculating"] = "计算中...";
    zh["status.complete"] = "完成";
    zh["status.error"] = "错误";
    zh["technical.details"] = "技术详情";

    zh["web.server_starting"] = "启动Web服务器，监听端口:";
    zh["web.server_url"] = "Web界面访问URL:";
    zh["web.server_stopping"] = "停止Web服务器";

    // 英文翻译
    auto &en = translations[Language::ENGLISH];
    en["app.title"] = "Neumann Trend Test Tool";
    en["app.description"] = "Statistical tool for evaluating systematic trends in datasets";
    en["menu.main"] = "Main Menu";
    en["menu.new_test"] = "Run New Neumann Trend Test";
    en["menu.load_data"] = "Load Dataset";
    en["menu.import_csv"] = "Import Data from CSV";
    en["menu.import_excel"] = "Import Data from Excel";
    en["menu.help"] = "Help";
    en["menu.about"] = "About";
    en["menu.exit"] = "Exit";
    en["menu.settings"] = "Settings";
    en["menu.language"] = "Language Settings";
    en["menu.back"] = "Back";

    en["input.data_points"] = "Please enter data points (separated by spaces):";
    en["input.time_points"] = "Please enter time points (separated by spaces):";
    en["input.confidence_level"] = "Please select confidence level:";
    en["input.filename"] = "Please enter filename:";
    en["input.exit_hint"] = "Enter 'q' or press Enter to exit";

    en["load.no_datasets"] = "No datasets available.";
    en["load.available_datasets"] = "Available datasets:";
    en["load.select_dataset"] = "Please select a dataset";

    en["prompt.press_enter"] = "Press Enter to continue...";
    en["prompt.select_option"] = "Please select an option";

    en["result.no_trend"] = "No Trend";
    en["result.has_trend"] = "Has Trend";
    en["result.pg_value"] = "PG Value";
    en["result.threshold"] = "Threshold";
    en["result.overall_trend"] = "Overall Trend";

    // Error messages
    en["error.file_not_found"] = "File not found";
    en["error.file_read_error"] = "File read error";
    en["error.file_write_error"] = "File write error";
    en["error.file_permission_error"] = "File permission error";
    en["error.invalid_data"] = "Invalid data format";
    en["error.insufficient_data"] = "Insufficient data points, at least 4 points required";
    en["error.data_out_of_range"] = "Data out of range";
    en["error.data_parsing_error"] = "Data parsing error";
    en["error.invalid_confidence_level"] = "Invalid confidence level";
    en["error.calculation_failed"] = "Calculation failed";
    en["error.standard_values_not_found"] = "Standard values not found";
    en["error.network_error"] = "Network error";
    en["error.port_in_use"] = "Port already in use";
    en["error.web_server_error"] = "Web server error";
    en["error.config_load_error"] = "Configuration load failed";
    en["error.config_save_error"] = "Configuration save failed";
    en["error.invalid_config_value"] = "Invalid configuration value";
    en["error.memory_error"] = "Memory error";
    en["error.system_error"] = "System error";
    en["error.permission_denied"] = "Permission denied";
    en["error.unknown"] = "Unknown error";

    // Suggestion messages
    en["suggestion.title"] = "Suggestion";
    en["suggestion.check_file_path"] = "Please check if the file path is correct";
    en["suggestion.check_file_permissions"] = "Please check file read permissions";
    en["suggestion.check_write_permissions"] = "Please check file write permissions";
    en["suggestion.run_as_admin"] = "Please run as administrator";
    en["suggestion.check_data_format"] = "Please check if the data format is correct";
    en["suggestion.add_more_data"] = "Please add more data points";
    en["suggestion.check_data_values"] = "Please check if data values are within valid range";
    en["suggestion.fix_data_format"] = "Please fix the data format";
    en["suggestion.use_valid_confidence"] =
        "Please use valid confidence level (0.90, 0.95, 0.975, 0.99)";
    en["suggestion.check_input_data"] = "Please check input data";
    en["suggestion.check_standard_values_file"] = "Please check if standard values file exists";
    en["suggestion.check_network"] = "Please check network connection";
    en["suggestion.try_different_port"] = "Please try a different port";
    en["suggestion.restart_server"] = "Please restart the server";
    en["suggestion.reset_config"] = "Please reset configuration to defaults";
    en["suggestion.reduce_data_size"] = "Please reduce data size";
    en["suggestion.contact_support"] = "Please contact technical support";

    en["status.loading"] = "Loading...";
    en["status.calculating"] = "Calculating...";
    en["status.complete"] = "Complete";
    en["status.error"] = "Error";
    en["technical.details"] = "Technical Details";

    en["web.server_starting"] = "Starting web server on port:";
    en["web.server_url"] = "Web interface URL:";
    en["web.server_stopping"] = "Stopping web server";
}

// 显式实例化模板
template std::string I18n::getTextf(const std::string &key, int) const;
template std::string I18n::getTextf(const std::string &key, double) const;
template std::string I18n::getTextf(const std::string &key, const std::string &) const;
template std::string I18n::getTextf(const std::string &key, size_t) const;

}  // namespace neumann