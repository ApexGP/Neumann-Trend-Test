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
    zh["input.has_header"] = "文件是否包含表头";
    zh["input.file_preview"] = "文件预览";
    zh["input.tab_completion_hint"] = "💡 提示：使用方向键浏览文件，支持文件路径补全";
    zh["input.tab_completion_directory_hint"] = "💡 提示：使用方向键浏览目录，支持目录路径补全";
    zh["input.tab_completion_files_hint"] = "💡 提示：使用方向键浏览文件，多个文件用逗号分隔";
    zh["input.enter_for_list"] = "💡 提示：使用方向键导航，Enter选择文件，q退出";

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
    zh["web.opening_browser_silently"] = "正在静默打开浏览器...";
    zh["web.browser_opened_silently"] = "浏览器已在后台打开";
    zh["web.browser_background_info"] = "💡 浏览器已在后台启动，不会抢夺终端焦点";

    // 添加路径输入帮助的翻译
    zh["help.path_input_title"] = "📝 路径输入帮助:";
    zh["help.path_input_enter"] = "直接按回车: 显示当前目录内容";
    zh["help.path_input_use_path"] = "输入路径后按回车: 使用该路径";
    zh["help.path_input_browse_dir"] = "输入目录路径+'/': 显示该目录内容";
    zh["help.path_input_quit"] = "输入 'q' 或 'quit': 退出";
    zh["help.path_input_help"] = "输入 '?': 显示此帮助";

    // 添加tab补全相关的翻译
    zh["completion.no_matches"] = "没有匹配的文件或目录";
    zh["completion.options"] = "可选项:";

    // 添加目录相关的翻译
    zh["directory.current"] = "📁 当前目录";
    zh["error.cannot_read_directory"] = "无法读取当前目录";
    zh["error.directory_not_exist"] = "目录不存在";
    zh["directory.folders"] = "📁 目录:";
    zh["directory.files"] = "📄 文件:";
    zh["error.cannot_read_dir"] = "无法读取目录";

    zh["load.no_datasets"] = "没有可用的数据集。";

    // 添加更多tab补全相关的翻译
    zh["input.tab_completion_instruction"] =
        "💡 按 Tab 键进行自动补全，输入部分路径后按 Tab 可显示匹配的文件/目录";
    zh["input.windows_tab_help"] = "💡 Windows提示：输入路径+回车浏览目录，输入 'tab' 显示补全选项";

    // 文件浏览器相关翻译
    zh["browser.title"] = "📁 文件浏览器";
    zh["browser.current_path"] = "当前路径";
    zh["browser.help"] = "操作：↑↓选择 ←返回上级 →进入目录 Enter确认 q退出";
    zh["browser.no_files"] = "目录为空";
    zh["browser.parent_dir"] = ".. (上级目录)";
    zh["browser.selected"] = "已选择";
    zh["browser.help_detailed"] =
        "💡 提示：使用方向键导航，Enter选择，左键返回上级，右键进入目录，q退出";
    zh["browser.scroll_info"] = "显示";
    zh["browser.of_total"] = "/";

    // 现代化数据输入相关翻译
    zh["test.modern_input_hint"] = "现代化数据输入界面（类似Excel的双栏输入）：";
    zh["test.modern_input_navigation"] = "使用↑↓键上下移动，←→键切换时间/数据栏";
    zh["test.modern_input_edit"] = "按Enter键编辑单元格，输入数据后再按Enter确认";
    zh["test.modern_input_save"] = "按Ctrl+S保存所有数据并继续测试";
    zh["test.modern_input_exit"] = "按Esc键随时退出而不保存数据";
    zh["input.enter_test_data"] = "请输入测试数据";
    zh["prompt.press_enter_to_continue"] = "按Enter键继续...";

    // 数据输入界面UI翻译
    zh["ui.time_column"] = "时间点";
    zh["ui.data_column"] = "数据值";
    zh["ui.help_navigation"] = "上下移动";
    zh["ui.help_switch"] = "切换区域";
    zh["ui.help_edit"] = "编辑";
    zh["ui.help_save"] = "保存";
    zh["ui.help_exit"] = "退出";
    zh["ui.current_area"] = "当前区域";
    zh["ui.empty_cell"] = "---";
    zh["ui.row_number"] = "行号";

    // 文件浏览器详细翻译
    zh["file_browser.parent_dir"] = "父目录";
    zh["file_browser.current_dir"] = "当前目录";
    zh["file_browser.preview"] = "预览";
    zh["file_browser.empty_dir"] = "目录为空";
    zh["file_browser.help_up"] = "↑上移";
    zh["file_browser.help_down"] = "↓下移";
    zh["file_browser.help_prev"] = "←返回";
    zh["file_browser.help_next"] = "→进入";
    zh["file_browser.help_select"] = "Enter选择";
    zh["file_browser.help_preview_scroll"] = "PgUp/PgDn/鼠标滚轮预览滚动";
    zh["file_browser.help_exit"] = "q退出";
    zh["file_browser.file_info"] = "文件信息";
    zh["file_browser.filename"] = "文件名";
    zh["file_browser.filesize"] = "文件大小";
    zh["file_browser.filetype"] = "文件类型";
    zh["file_browser.unknown_type"] = "未知类型";
    zh["file_browser.excel_file"] = "Excel文件";
    zh["file_browser.json_file"] = "JSON文件";
    zh["file_browser.csv_file"] = "CSV文件";
    zh["file_browser.file_suffix"] = "文件";
    zh["file_browser.supported_file"] = "✓ 支持的数据文件格式";
    zh["file_browser.preview_error"] = "预览失败";

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
    en["input.has_header"] = "Does the file contain a header";
    en["input.file_preview"] = "File Preview";
    en["input.tab_completion_hint"] =
        "💡 Tip: Use arrow keys to browse files, supports file path completion";
    en["input.tab_completion_directory_hint"] =
        "💡 Tip: Use arrow keys to browse directories, supports directory path completion";
    en["input.tab_completion_files_hint"] =
        "💡 Tip: Use arrow keys to browse files, separate multiple files with commas";
    en["input.enter_for_list"] =
        "💡 Tip: Use arrow keys to navigate, Enter to select file, q to quit";

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
    en["web.opening_browser_silently"] = "Opening browser silently...";
    en["web.browser_opened_silently"] = "Browser opened in the background";
    en["web.browser_background_info"] =
        "💡 Browser started in the background, will not steal focus";

    // 添加路径输入帮助的英文翻译
    en["help.path_input_title"] = "📝 Path Input Help:";
    en["help.path_input_enter"] = "Press Enter directly: Show current directory contents";
    en["help.path_input_use_path"] = "Enter path then press Enter: Use that path";
    en["help.path_input_browse_dir"] = "Enter directory path+'/': Show that directory contents";
    en["help.path_input_quit"] = "Enter 'q' or 'quit': Exit";
    en["help.path_input_help"] = "Enter '?': Show this help";

    // 添加更多tab补全相关的英文翻译
    en["input.tab_completion_instruction"] =
        "💡 Press Tab for auto-completion, type partial path then Tab to show matching "
        "files/directories";
    en["input.windows_tab_help"] =
        "💡 Windows tip: Enter path+Enter to browse directory, type 'tab' to show completion "
        "options";

    // 添加tab补全相关的英文翻译
    en["completion.no_matches"] = "No matching files or directories";
    en["completion.options"] = "Options:";

    // 添加目录相关的英文翻译
    en["directory.current"] = "📁 Current Directory";
    en["error.cannot_read_directory"] = "Cannot read current directory";
    en["error.directory_not_exist"] = "Directory does not exist";
    en["directory.folders"] = "📁 Directories:";
    en["directory.files"] = "📄 Files:";
    en["error.cannot_read_dir"] = "Cannot read directory";

    // 文件浏览器相关英文翻译
    en["browser.title"] = "📁 File Browser";
    en["browser.current_path"] = "Current Path";
    en["browser.help"] = "Controls: ↑↓Select ←Back →Enter Dir Enter Confirm q Quit";
    en["browser.no_files"] = "Directory is empty";
    en["browser.parent_dir"] = ".. (Parent Directory)";
    en["browser.selected"] = "Selected";
    en["browser.help_detailed"] =
        "💡 Tip: Use arrow keys to navigate, Enter to select, ← back, → enter dir, q quit";
    en["browser.scroll_info"] = "Showing";
    en["browser.of_total"] = "of";

    // 现代化数据输入相关英文翻译
    en["test.modern_input_hint"] = "Modern data input interface (Excel-like dual-column input):";
    en["test.modern_input_navigation"] =
        "Use ↑↓ keys to move up/down, ←→ keys to switch between time/data columns";
    en["test.modern_input_edit"] =
        "Press Enter to edit cell, input data then press Enter to confirm";
    en["test.modern_input_save"] = "Press Ctrl+S to save all data and continue testing";
    en["test.modern_input_exit"] = "Press Esc to exit anytime without saving data";
    en["input.enter_test_data"] = "Please enter test data";
    en["prompt.press_enter_to_continue"] = "Press Enter to continue...";

    // 数据输入界面UI英文翻译
    en["ui.time_column"] = "Time Points";
    en["ui.data_column"] = "Data Values";
    en["ui.help_navigation"] = "Navigate";
    en["ui.help_switch"] = "Switch Area";
    en["ui.help_edit"] = "Edit";
    en["ui.help_save"] = "Save";
    en["ui.help_exit"] = "Exit";
    en["ui.current_area"] = "Current Area";
    en["ui.empty_cell"] = "---";
    en["ui.row_number"] = "Row Number";

    // 文件浏览器详细英文翻译
    en["file_browser.parent_dir"] = "Parent Directory";
    en["file_browser.current_dir"] = "Current Directory";
    en["file_browser.preview"] = "Preview";
    en["file_browser.empty_dir"] = "Directory is empty";
    en["file_browser.help_up"] = "↑Up";
    en["file_browser.help_down"] = "↓Down";
    en["file_browser.help_prev"] = "←Back";
    en["file_browser.help_next"] = "→Enter";
    en["file_browser.help_select"] = "Enter Select";
    en["file_browser.help_preview_scroll"] = "PgUp/PgDn/MouseWheel Preview Scroll";
    en["file_browser.help_exit"] = "q Quit";
    en["file_browser.file_info"] = "File Information";
    en["file_browser.filename"] = "Filename";
    en["file_browser.filesize"] = "File Size";
    en["file_browser.filetype"] = "File Type";
    en["file_browser.unknown_type"] = "Unknown Type";
    en["file_browser.excel_file"] = "Excel File";
    en["file_browser.json_file"] = "JSON File";
    en["file_browser.csv_file"] = "CSV File";
    en["file_browser.file_suffix"] = "File";
    en["file_browser.supported_file"] = "✓ Supported data file format";
    en["file_browser.preview_error"] = "Preview failed";
}

// 显式实例化模板
template std::string I18n::getTextf(const std::string &key, int) const;
template std::string I18n::getTextf(const std::string &key, double) const;
template std::string I18n::getTextf(const std::string &key, const std::string &) const;
template std::string I18n::getTextf(const std::string &key, std::string) const;  // 添加非引用版本
template std::string I18n::getTextf(const std::string &key, size_t) const;
template std::string I18n::getTextf(const std::string &key, char *) const;
template std::string I18n::getTextf(const std::string &key, const char *) const;

}  // namespace neumann