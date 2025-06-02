#include "cli/data_editor.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

// 添加国际化支持
#include "core/i18n.h"

// 添加FTXUI的Terminal支持来获取终端尺寸
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;

namespace neumann { namespace cli {

DataEditor::DataEditor(const EditorConfig& config)
    : config_(config),
      selectedRow_(0),
      currentPane_(EditPane::TIME_PANE),
      scrollOffset_(0),
      userCancelled_(false),
      editMode_(false)
{
    // 如果配置中的标签为空，则使用翻译的默认值
    if (config_.timeLabel.empty()) {
        config_.timeLabel = _("ui.time_column");
    }
    if (config_.dataLabel.empty()) {
        config_.dataLabel = _("ui.data_column");
    }
}

std::pair<std::vector<double>, std::vector<double>> DataEditor::editData(
    const std::string& prompt, const std::vector<double>& initialTimePoints,
    const std::vector<double>& initialDataPoints)
{
    userCancelled_ = false;
    selectedRow_ = 0;
    currentPane_ = EditPane::TIME_PANE;
    scrollOffset_ = 0;
    editMode_ = false;
    editBuffer_.clear();

    // 初始化数据条目
    dataEntries_.clear();
    size_t maxSize = std::max(initialTimePoints.size(), initialDataPoints.size());
    maxSize = std::max(maxSize, static_cast<size_t>(10));  // 至少10行

    for (size_t i = 0; i < maxSize; ++i) {
        DataEntry entry;
        if (i < initialTimePoints.size()) {
            entry.timeValue = std::to_string(initialTimePoints[i]);
            entry.timeValid = true;
        }
        if (i < initialDataPoints.size()) {
            entry.dataValue = std::to_string(initialDataPoints[i]);
            entry.dataValid = true;
        }
        dataEntries_.push_back(entry);
    }

    auto screen = ScreenInteractive::Fullscreen();
    auto renderer = createRenderer(prompt);
    auto component = Renderer([=] { return renderer(); });

    component = CatchEvent(component,
                           [this, &screen](Event event) { return handleKeyEvent(event, screen); });

    screen.Loop(component);

    if (userCancelled_) {
        return {{}, {}};
    }

    return validateAndConvert();
}

void DataEditor::setConfig(const EditorConfig& config)
{
    config_ = config;
}

const DataEditor::EditorConfig& DataEditor::getConfig() const
{
    return config_;
}

int DataEditor::calculateMaxDisplayItems() const
{
    try {
        // 获取终端尺寸
        auto dimensions = Terminal::Size();
        int terminalHeight = dimensions.dimy;

        // 固定UI元素占用的行数
        // 标题栏: 3行, 帮助栏: 4行, 其他边距: 6行
        int fixedUIRows = 13;

        // 计算可用于显示数据的行数
        int availableRows = terminalHeight - fixedUIRows;

        // 每栏平分可用空间，确保合理范围
        int maxItems;
        if (availableRows < 20) {
            maxItems = 15;  // 小终端
        } else if (availableRows < 40) {
            maxItems = 25;  // 中等终端
        } else {
            maxItems = 40;  // 大终端
        }

        // 如果配置有限制，优先使用配置值
        if (config_.maxDisplayItems > 0 && config_.maxDisplayItems < maxItems) {
            maxItems = config_.maxDisplayItems;
        }

        return maxItems;
    }
    catch (...) {
        // 如果终端尺寸获取失败，使用默认值
        return 20;
    }
}

std::function<Element()> DataEditor::createRenderer(const std::string& prompt)
{
    return [this, prompt]() -> Element {
        // 动态计算最大显示项目数
        int maxDisplayItems = calculateMaxDisplayItems();

        // 标题栏
        auto title =
            hbox({text("📊 ") | color(Color::Yellow), text(prompt) | color(Color::Yellow) | bold}) |
            center | border | color(Color::Cyan);

        // 创建三栏布局：行号栏 + 时间点栏 + 数据值栏
        auto rowNumberPane = createRowNumberPane(maxDisplayItems);  // 独立行号栏
        auto timePane = createDataPane(true, maxDisplayItems);      // 时间栏（不再显示行号）
        auto dataPane = createDataPane(false, maxDisplayItems);     // 数据栏（不再显示行号）

        // 主内容区域：三栏并排，类似Excel布局
        auto mainContent = hbox({
            rowNumberPane | size(WIDTH, EQUAL, 6),  // 行号栏固定宽度
            separator() | color(Color::Cyan),
            timePane | flex,  // 时间栏自适应
            separator() | color(Color::Cyan),
            dataPane | flex  // 数据栏自适应
        });

        // 帮助栏
        auto helpBar =
            vbox({hbox({text("↑↓") | color(Color::Cyan),
                        text(": " + _("ui.help_navigation") + "  ") | color(Color::Default),
                        text("←→") | color(Color::Cyan),
                        text(": " + _("ui.help_switch") + "  ") | color(Color::Default),
                        text("Enter") | color(Color::Green),
                        text(": " + _("ui.help_edit") + "  ") | color(Color::Default),
                        text("Ctrl+S") | color(Color::Green),
                        text(": " + _("ui.help_save")) | color(Color::Default)}),
                  hbox({text("Esc") | color(Color::Red),
                        text(": " + _("ui.help_exit") + "  ") | color(Color::Default),
                        text(_("ui.current_area") + ": ") | color(Color::Default),
                        text(currentPane_ == EditPane::TIME_PANE ? config_.timeLabel
                                                                 : config_.dataLabel) |
                            color(Color::Yellow) | bold})}) |
            border | color(Color::Cyan);

        return vbox({title, mainContent | flex, helpBar});
    };
}

Element DataEditor::createRowNumberPane(int maxDisplayItems)
{
    Elements elements;

    // 添加标题
    auto titleElement = text(_("ui.row_number")) | color(Color::Yellow) | bold | center;
    elements.push_back(titleElement);
    elements.push_back(separator());

    // 确保有足够的数据行
    int displayCount = std::min(maxDisplayItems, static_cast<int>(dataEntries_.size()));
    int startIndex =
        std::max(0, std::min(scrollOffset_, static_cast<int>(dataEntries_.size()) - displayCount));

    for (int i = 0; i < displayCount && startIndex + i < static_cast<int>(dataEntries_.size());
         ++i) {
        int rowIndex = startIndex + i;
        bool isSelected = (selectedRow_ == rowIndex);

        // 格式化行号为两位数，右对齐
        std::string rowNumberText =
            (rowIndex + 1 < 10) ? " " + std::to_string(rowIndex + 1) : std::to_string(rowIndex + 1);

        auto rowNumberElement = text(rowNumberText) | color(Color::GrayLight);

        // 如果是选中行，添加高亮背景
        if (isSelected) {
            rowNumberElement = rowNumberElement | bgcolor(Color::Cyan);
        }

        elements.push_back(rowNumberElement | center);
    }

    // 添加填充空行
    int emptyLines = maxDisplayItems - displayCount;
    for (int i = 0; i < emptyLines; ++i) {
        elements.push_back(text("  ") | center);
    }

    return vbox(elements) | border | color(Color::Cyan);
}

Element DataEditor::createDataPane(bool isTimePane, int maxDisplayItems)
{
    Elements elements;

    // 添加标题
    std::string title = isTimePane ? config_.timeLabel : config_.dataLabel;
    bool isCurrentPane = (isTimePane && currentPane_ == EditPane::TIME_PANE) ||
                         (!isTimePane && currentPane_ == EditPane::DATA_PANE);

    auto titleElement = text(title) | color(Color::Yellow) | bold | center;
    if (isCurrentPane) {
        titleElement = titleElement | bgcolor(Color::Blue);
    }
    elements.push_back(titleElement);
    elements.push_back(separator());

    // 确保有足够的数据行
    int displayCount = std::min(maxDisplayItems, static_cast<int>(dataEntries_.size()));
    int startIndex =
        std::max(0, std::min(scrollOffset_, static_cast<int>(dataEntries_.size()) - displayCount));

    for (int i = 0; i < displayCount && startIndex + i < static_cast<int>(dataEntries_.size());
         ++i) {
        int rowIndex = startIndex + i;
        bool isSelected = (selectedRow_ == rowIndex);

        // 构建单元格内容
        std::string cellText = getCellText(rowIndex, isTimePane);

        // 如果处于编辑模式且是当前选中的单元格
        if (editMode_ && isSelected && isCurrentPane) {
            cellText = editBuffer_;
        }

        // 单元格内容（不再显示行号）
        auto cellElement = text(cellText.empty() ? _("ui.empty_cell") : cellText);

        // 设置颜色
        Color cellColor = getCellColor(rowIndex, isTimePane, isSelected, isCurrentPane);
        cellElement = cellElement | color(cellColor);

        // 如果选中且是当前栏，添加背景色
        if (isSelected && isCurrentPane) {
            if (editMode_) {
                cellElement = cellElement | bgcolor(Color::Green);
            } else {
                cellElement = cellElement | bgcolor(Color::Cyan);
            }
        }

        elements.push_back(cellElement);
    }

    // 添加填充空行
    int emptyLines = maxDisplayItems - displayCount;
    for (int i = 0; i < emptyLines; ++i) {
        elements.push_back(text(" "));
    }

    return vbox(elements) | border | color(isCurrentPane ? Color::Yellow : Color::Cyan);
}

bool DataEditor::handleKeyEvent(Event event, ScreenInteractive& screen)
{
    // 如果处于编辑模式
    if (editMode_) {
        if (event == Event::Return) {
            finishEdit();
            return true;
        }
        if (event == Event::Escape) {
            cancelEdit();
            return true;
        }
        if (event == Event::Backspace) {
            if (!editBuffer_.empty()) {
                editBuffer_.pop_back();
            }
            return true;
        }
        // 处理字符输入
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= 32 && c <= 126) {  // 可打印字符
                editBuffer_ += c;
            }
            return true;
        }
        return false;
    }

    // 导航键
    if (config_.enableVimKeys) {
        if (event == Event::Character('j') || event == Event::ArrowDown) {
            navigateDown();
            return true;
        }
        if (event == Event::Character('k') || event == Event::ArrowUp) {
            navigateUp();
            return true;
        }
        if (event == Event::Character('h') || event == Event::ArrowLeft) {
            navigateLeft();
            return true;
        }
        if (event == Event::Character('l') || event == Event::ArrowRight) {
            navigateRight();
            return true;
        }
    } else {
        if (event == Event::ArrowDown) {
            navigateDown();
            return true;
        }
        if (event == Event::ArrowUp) {
            navigateUp();
            return true;
        }
        if (event == Event::ArrowLeft) {
            navigateLeft();
            return true;
        }
        if (event == Event::ArrowRight) {
            navigateRight();
            return true;
        }
    }

    // 编辑
    if (event == Event::Return) {
        startEdit();
        return true;
    }

    // 保存
    if (event == Event::Special({19})) {  // Ctrl+S (ASCII 19)
        saveAndExit(screen);
        return true;
    }

    // 退出
    if (event == Event::Escape) {
        userCancelled_ = true;
        screen.ExitLoopClosure()();
        return true;
    }

    return false;
}

void DataEditor::navigateUp()
{
    if (selectedRow_ > 0) {
        selectedRow_--;
        int maxDisplayItems = calculateMaxDisplayItems();
        adjustScrollOffset(selectedRow_, maxDisplayItems);
    }
}

void DataEditor::navigateDown()
{
    ensureDataRows();
    if (selectedRow_ < static_cast<int>(dataEntries_.size()) - 1) {
        selectedRow_++;
        int maxDisplayItems = calculateMaxDisplayItems();
        adjustScrollOffset(selectedRow_, maxDisplayItems);
    }
}

void DataEditor::navigateLeft()
{
    currentPane_ = EditPane::TIME_PANE;
}

void DataEditor::navigateRight()
{
    currentPane_ = EditPane::DATA_PANE;
}

void DataEditor::startEdit()
{
    ensureDataRows();
    editMode_ = true;

    // 初始化编辑缓冲区
    if (currentPane_ == EditPane::TIME_PANE) {
        editBuffer_ = dataEntries_[selectedRow_].timeValue;
    } else {
        editBuffer_ = dataEntries_[selectedRow_].dataValue;
    }
}

void DataEditor::finishEdit()
{
    editMode_ = false;

    // 保存编辑结果
    if (currentPane_ == EditPane::TIME_PANE) {
        dataEntries_[selectedRow_].timeValue = editBuffer_;
        dataEntries_[selectedRow_].timeValid = !editBuffer_.empty();
    } else {
        dataEntries_[selectedRow_].dataValue = editBuffer_;
        dataEntries_[selectedRow_].dataValid = !editBuffer_.empty();
    }

    editBuffer_.clear();

    // 自动移动到下一行
    navigateDown();
}

void DataEditor::cancelEdit()
{
    editMode_ = false;
    editBuffer_.clear();
}

void DataEditor::saveAndExit(ScreenInteractive& screen)
{
    userCancelled_ = false;
    screen.ExitLoopClosure()();
}

void DataEditor::adjustScrollOffset(int selectedRow, int maxDisplayItems)
{
    if (static_cast<int>(dataEntries_.size()) <= maxDisplayItems) {
        scrollOffset_ = 0;
        return;
    }

    // 保持选中项在可见区域内
    if (selectedRow < scrollOffset_) {
        scrollOffset_ = selectedRow;
    } else if (selectedRow >= scrollOffset_ + maxDisplayItems) {
        scrollOffset_ = selectedRow - maxDisplayItems + 1;
    }

    // 确保偏移量在有效范围内
    scrollOffset_ = std::max(
        0, std::min(scrollOffset_, static_cast<int>(dataEntries_.size()) - maxDisplayItems));
}

void DataEditor::ensureDataRows()
{
    // 确保至少有选中行+5行数据
    while (static_cast<int>(dataEntries_.size()) <= selectedRow_ + 5) {
        dataEntries_.push_back(DataEntry{});
    }
}

std::pair<std::vector<double>, std::vector<double>> DataEditor::validateAndConvert()
{
    std::vector<double> timePoints;
    std::vector<double> dataPoints;

    for (const auto& entry : dataEntries_) {
        if (entry.timeValid && entry.dataValid && !entry.timeValue.empty() &&
            !entry.dataValue.empty()) {
            try {
                double timeVal = std::stod(entry.timeValue);
                double dataVal = std::stod(entry.dataValue);
                timePoints.push_back(timeVal);
                dataPoints.push_back(dataVal);
            }
            catch (const std::exception&) {
                // 跳过无效数据
                continue;
            }
        }
    }

    return {timePoints, dataPoints};
}

std::string DataEditor::getCellText(int row, bool isTimePane) const
{
    if (row >= static_cast<int>(dataEntries_.size())) {
        return "";
    }

    const auto& entry = dataEntries_[row];
    return isTimePane ? entry.timeValue : entry.dataValue;
}

Color DataEditor::getCellColor(int row, bool isTimePane, bool isSelected, bool isCurrentPane) const
{
    if (row >= static_cast<int>(dataEntries_.size())) {
        return Color::Default;
    }

    const auto& entry = dataEntries_[row];
    bool isValid = isTimePane ? entry.timeValid : entry.dataValid;

    if (isSelected && isCurrentPane) {
        return Color::Black;  // 选中时使用黑色文字（背景会是亮色）
    }

    if (!isValid || (isTimePane ? entry.timeValue.empty() : entry.dataValue.empty())) {
        return Color::GrayLight;  // 空或无效数据用灰色
    }

    return Color::Default;  // 有效数据用默认颜色
}

}}  // namespace neumann::cli