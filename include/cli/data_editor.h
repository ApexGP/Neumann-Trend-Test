#pragma once

#include <functional>
#include <string>
#include <vector>

// FTXUI支持
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace neumann { namespace cli {

/**
 * @brief 现代化数据编辑器类（双栏布局，类似Excel）
 * 
 * 提供类似Excel的交互式数据编辑界面
 * 左栏：时间点编辑
 * 右栏：数据点编辑
 */
class DataEditor
{
public:
    /**
     * @brief 数据编辑配置结构
     */
    struct EditorConfig {
        bool showRowNumbers;    // 是否显示行号
        bool enableVimKeys;     // 是否启用Vim快捷键
        int maxDisplayItems;    // 最大显示项目数（0表示自动计算）
        std::string timeLabel;  // 时间栏标题
        std::string dataLabel;  // 数据栏标题

        // 默认构造函数
        EditorConfig()
            : showRowNumbers(true),
              enableVimKeys(true),
              maxDisplayItems(0),
              timeLabel(""),  // 空字符串，将在构造函数中设置翻译
              dataLabel("")   // 空字符串，将在构造函数中设置翻译
        {
        }
    };

    /**
     * @brief 数据项结构
     */
    struct DataEntry {
        std::string timeValue;  // 时间值字符串
        std::string dataValue;  // 数据值字符串
        bool timeValid;         // 时间值是否有效
        bool dataValid;         // 数据值是否有效

        DataEntry() : timeValid(false), dataValid(false) {}
    };

    /**
     * @brief 编辑区域枚举
     */
    enum class EditPane {
        TIME_PANE,  // 时间编辑区域
        DATA_PANE   // 数据编辑区域
    };

    /**
     * @brief 构造函数
     * @param config 编辑器配置
     */
    explicit DataEditor(const EditorConfig& config = EditorConfig{});

    /**
     * @brief 析构函数
     */
    ~DataEditor() = default;

    /**
     * @brief 启动数据编辑界面
     * @param prompt 提示信息
     * @param initialTimePoints 初始时间点（可选）
     * @param initialDataPoints 初始数据点（可选）
     * @return 编辑的数据对，第一个是时间点，第二个是数据点；空表示用户取消
     */
    std::pair<std::vector<double>, std::vector<double>> editData(
        const std::string& prompt, const std::vector<double>& initialTimePoints = {},
        const std::vector<double>& initialDataPoints = {});

    /**
     * @brief 设置编辑器配置
     * @param config 新的配置
     */
    void setConfig(const EditorConfig& config);

    /**
     * @brief 获取当前配置
     * @return 当前配置
     */
    const EditorConfig& getConfig() const;

private:
    EditorConfig config_;                 // 编辑器配置
    std::vector<DataEntry> dataEntries_;  // 数据条目列表
    int selectedRow_;                     // 当前选中行
    EditPane currentPane_;                // 当前编辑区域
    int scrollOffset_;                    // 滚动偏移
    bool userCancelled_;                  // 用户是否取消
    bool editMode_;                       // 是否处于编辑模式
    std::string editBuffer_;              // 编辑缓冲区

    /**
     * @brief 动态计算最大显示项目数（基于终端高度）
     * @return 可显示的最大项目数
     */
    int calculateMaxDisplayItems() const;

    /**
     * @brief 创建FTXUI渲染器
     * @param prompt 提示信息
     * @return 渲染函数
     */
    std::function<ftxui::Element()> createRenderer(const std::string& prompt);

    /**
     * @brief 创建单个数据栏的元素
     * @param isTimePane 是否为时间栏
     * @param maxDisplayItems 最大显示项目数
     * @return 栏元素
     */
    ftxui::Element createDataPane(bool isTimePane, int maxDisplayItems);

    /**
     * @brief 创建独立的行号栏元素
     * @param maxDisplayItems 最大显示项目数
     * @return 行号栏元素
     */
    ftxui::Element createRowNumberPane(int maxDisplayItems);

    /**
     * @brief 处理按键事件
     * @param event 按键事件
     * @param screen 屏幕对象引用
     * @return 是否处理了该事件
     */
    bool handleKeyEvent(ftxui::Event event, ftxui::ScreenInteractive& screen);

    /**
     * @brief 向上导航
     */
    void navigateUp();

    /**
     * @brief 向下导航
     */
    void navigateDown();

    /**
     * @brief 向左导航（切换到时间栏）
     */
    void navigateLeft();

    /**
     * @brief 向右导航（切换到数据栏）
     */
    void navigateRight();

    /**
     * @brief 开始编辑当前单元格
     */
    void startEdit();

    /**
     * @brief 完成编辑并保存
     */
    void finishEdit();

    /**
     * @brief 取消编辑
     */
    void cancelEdit();

    /**
     * @brief 保存所有数据并退出
     * @param screen 屏幕对象引用
     */
    void saveAndExit(ftxui::ScreenInteractive& screen);

    /**
     * @brief 调整滚动偏移量
     * @param selectedRow 选中行
     * @param maxDisplayItems 最大显示项目数
     */
    void adjustScrollOffset(int selectedRow, int maxDisplayItems);

    /**
     * @brief 确保有足够的数据行
     */
    void ensureDataRows();

    /**
     * @brief 验证并转换编辑的数据
     * @return 转换后的时间点和数据点对
     */
    std::pair<std::vector<double>, std::vector<double>> validateAndConvert();

    /**
     * @brief 获取单元格显示文本
     * @param row 行索引
     * @param isTimePane 是否为时间栏
     * @return 显示文本
     */
    std::string getCellText(int row, bool isTimePane) const;

    /**
     * @brief 获取单元格颜色
     * @param row 行索引
     * @param isTimePane 是否为时间栏
     * @param isSelected 是否选中
     * @param isCurrentPane 是否为当前栏
     * @return FTXUI颜色
     */
    ftxui::Color getCellColor(int row, bool isTimePane, bool isSelected, bool isCurrentPane) const;
};

}}  // namespace neumann::cli