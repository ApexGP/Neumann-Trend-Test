#include "cli/file_browser.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

// 添加国际化支持
#include "core/i18n.h"

// 添加FTXUI的Terminal支持来获取终端尺寸
#include <ftxui/screen/terminal.hpp>

using namespace ftxui;
namespace fs = std::filesystem;

namespace neumann { namespace cli {

// UTF-8编码检测辅助函数
static bool isValidUtf8(const std::string& str)
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str.c_str());
    size_t len = str.length();

    for (size_t i = 0; i < len;) {
        if (bytes[i] <= 0x7F) {
            // ASCII字符
            i++;
        } else if ((bytes[i] & 0xE0) == 0xC0) {
            // 2字节UTF-8序列
            if (i + 1 >= len || (bytes[i + 1] & 0xC0) != 0x80) return false;
            i += 2;
        } else if ((bytes[i] & 0xF0) == 0xE0) {
            // 3字节UTF-8序列
            if (i + 2 >= len || (bytes[i + 1] & 0xC0) != 0x80 || (bytes[i + 2] & 0xC0) != 0x80)
                return false;
            i += 3;
        } else if ((bytes[i] & 0xF8) == 0xF0) {
            // 4字节UTF-8序列
            if (i + 3 >= len || (bytes[i + 1] & 0xC0) != 0x80 || (bytes[i + 2] & 0xC0) != 0x80 ||
                (bytes[i + 3] & 0xC0) != 0x80)
                return false;
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

// 安全的UTF-8字符串截断函数
static std::string safeSubstring(const std::string& str, size_t maxBytes)
{
    if (str.length() <= maxBytes) {
        return str;
    }

    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str.c_str());
    size_t safeEnd = maxBytes;

    // 向后查找，确保不在UTF-8字符中间截断
    while (safeEnd > 0) {
        unsigned char byte = bytes[safeEnd];
        if ((byte & 0x80) == 0) {
            // ASCII字符，安全位置
            break;
        } else if ((byte & 0xC0) == 0xC0) {
            // UTF-8序列开始，安全位置
            break;
        } else {
            // UTF-8序列中间，继续向前查找
            safeEnd--;
        }
    }

    return str.substr(0, safeEnd);
}

FileBrowser::FileBrowser(const BrowserConfig& config)
    : config_(config),
      selectedIndex_(0),
      parentScrollOffset_(0),
      currentScrollOffset_(0),
      previewScrollOffset_(0),
      directoriesOnly_(false),
      userCancelled_(false)
{
}

std::string FileBrowser::selectFile(const std::string& prompt, bool directoriesOnly,
                                    const std::string& startPath)
{
    directoriesOnly_ = directoriesOnly;
    userCancelled_ = false;
    selectedPath_.clear();

    // 设置起始路径
    if (!startPath.empty() && fs::exists(startPath)) {
        currentPath_ = fs::absolute(startPath).string();
    } else {
        currentPath_ = fs::current_path().string();
    }

    // 初始化所有栏
    refreshAllPanes();

    auto screen = ScreenInteractive::Fullscreen();
    auto renderer = createRenderer(prompt);
    auto component = Renderer([=] { return renderer(); });

    // 启用鼠标支持并添加事件处理
    component =
        CatchEvent(component, [this, &screen](Event event) { return handleEvent(event, screen); });

    screen.Loop(component);

    return userCancelled_ ? "" : selectedPath_;
}

std::string FileBrowser::selectDirectory(const std::string& prompt, const std::string& startPath)
{
    return selectFile(prompt, true, startPath);
}

void FileBrowser::setConfig(const BrowserConfig& config)
{
    config_ = config;
}

const FileBrowser::BrowserConfig& FileBrowser::getConfig() const
{
    return config_;
}

void FileBrowser::refreshAllPanes()
{
    refreshParentPane();
    refreshCurrentPane();
    refreshPreviewPane();
}

void FileBrowser::refreshParentPane()
{
    parentItems_.clear();

    fs::path current(currentPath_);

    // 检查是否为根目录（学习yazi的处理方式）
    // 在Windows下，根目录例如 "D:\" 或 "C:\"
    // 在Linux下，根目录为 "/"
    bool isRootDirectory = false;

#ifdef _WIN32
    // Windows系统：检查是否为驱动器根目录
    if (current.has_root_name() && current.relative_path().empty()) {
        isRootDirectory = true;
    }
#else
    // Unix/Linux系统：检查是否为根目录 "/"
    if (current == current.root_path()) {
        isRootDirectory = true;
    }
#endif

    if (!isRootDirectory && current.has_parent_path()) {
        parentPath_ = current.parent_path().string();
        parentItems_ = getDirectoryItems(parentPath_);
    } else {
        parentPath_.clear();
        // 在根目录时，父目录栏保持空白
    }
}

void FileBrowser::refreshCurrentPane()
{
    currentItems_ = getDirectoryItems(currentPath_);

    // 确保选中索引有效
    if (selectedIndex_ >= static_cast<int>(currentItems_.size())) {
        selectedIndex_ = std::max(0, static_cast<int>(currentItems_.size()) - 1);
    }

    // 动态计算可显示项目数
    int maxDisplayItems = calculateMaxDisplayItems();
    adjustScrollOffset(selectedIndex_, currentScrollOffset_, currentItems_.size(), maxDisplayItems);
}

void FileBrowser::refreshPreviewPane()
{
    previewItems_.clear();

    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(currentItems_.size())) {
        const auto& selectedItem = currentItems_[selectedIndex_];

        if (selectedItem.type == FileType::DIRECTORY) {
            // 预览子目录内容
            previewItems_ = getDirectoryItems(selectedItem.fullPath);
        } else {
            // 预览文件内容
            previewItems_ = getFilePreview(selectedItem);
        }
    }

    previewScrollOffset_ = 0;
}

int FileBrowser::calculateMaxDisplayItems() const
{
    try {
        // 获取终端尺寸
        auto dimensions = Terminal::Size();
        int terminalHeight = dimensions.dimy;

        // 保守估计固定UI元素占用的行数
        // 标题栏: 3行, 路径栏: 3行, 帮助栏: 3行, 其他边距: 4行
        int fixedUIRows = 13;

        // 写调试信息到文件
        std::ofstream debug("debug.txt", std::ios::app);
        debug << "=== FileBrowser 动态计算调试 ===" << std::endl;

        // 计算可用于显示文件的行数
        int availableRows = terminalHeight - fixedUIRows;
        debug << "可用行数: " << availableRows << std::endl;

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

        debug << "最终maxItems: " << maxItems << std::endl;
        debug.close();
        return maxItems;
    }
    catch (...) {
        // 如果终端尺寸获取失败，使用默认值
        return 20;
    }
}

std::vector<FileBrowser::FileItem> FileBrowser::getDirectoryItems(const std::string& path)
{
    std::vector<FileItem> items;

    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (!config_.showHiddenFiles && isHiddenFile(entry.path().filename().string())) {
                continue;
            }

            FileItem item;
            item.name = entry.path().filename().string();
            item.fullPath = entry.path().string();
            item.isHidden = isHiddenFile(item.name);

            if (entry.is_directory()) {
                item.type = FileType::DIRECTORY;
                item.size = 0;
            } else {
                item.type = detectFileType(item.name, false);
                try {
                    item.size = entry.file_size();
                }
                catch (...) {
                    item.size = 0;
                }
            }

            item.displayName = getFileIcon(item) + " " + item.name;
            items.push_back(item);
        }
    }
    catch (const fs::filesystem_error&) {
        // 无法访问目录，返回空列表
    }

    // 排序：目录在前，文件在后，按名称排序
    std::sort(items.begin(), items.end(), [](const FileItem& a, const FileItem& b) {
        if (a.type == FileType::DIRECTORY && b.type != FileType::DIRECTORY) return true;
        if (a.type != FileType::DIRECTORY && b.type == FileType::DIRECTORY) return false;
        return a.name < b.name;
    });

    return items;
}

std::vector<FileBrowser::FileItem> FileBrowser::getFilePreview(const FileItem& item)
{
    std::vector<FileItem> previewItems;

    try {
        // 获取文件扩展名
        std::string ext;
        if (item.name.find('.') != std::string::npos) {
            ext = item.name.substr(item.name.find_last_of('.'));
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }

        // 判断是否为文本文件（可预览内容）
        bool isTextFile = (ext == ".txt" || ext == ".md" || ext == ".json" || ext == ".csv" ||
                           ext == ".log" || ext == ".cpp" || ext == ".h" || ext == ".c" ||
                           ext == ".py" || ext == ".js" || ext == ".html" || ext == ".xml");

        if (isTextFile) {
            // 预览文本文件内容（改进版：支持UTF-8编码检测）
            std::ifstream file(item.fullPath, std::ios::binary);
            if (file.is_open()) {
                // 读取文件头部来检测编码
                std::string content;
                std::string buffer(4096, '\0');
                file.read(&buffer[0], buffer.size());
                std::streamsize bytesRead = file.gcount();
                content.append(buffer.data(), bytesRead);

                // 重置文件指针
                file.clear();
                file.seekg(0);

                // 检测是否为UTF-8编码
                bool isUtf8 = isValidUtf8(content);

                // 重新以文本模式打开
                file.close();
                std::ifstream textFile(item.fullPath);

                if (textFile.is_open()) {
                    std::string line;
                    int lineCount = 0;

                    // 读取完整文件内容（不限制行数，yazi风格）
                    while (std::getline(textFile, line)) {
                        FileItem lineItem;

                        // 改进的行长度处理：考虑UTF-8字符边界
                        if (line.length() > 80) {
                            line = safeSubstring(line, 77) + "...";
                        }

                        lineItem.name = line.empty() ? " " : line;
                        lineItem.displayName = line.empty() ? " " : line;
                        lineItem.type = FileType::REGULAR_FILE;
                        previewItems.push_back(lineItem);
                        lineCount++;

                        // 防止极大文件导致内存问题，设置合理上限
                        if (lineCount > 10000) {
                            FileItem limitItem;
                            limitItem.name = "... (文件过大，仅显示前10000行)";
                            limitItem.displayName = "... (文件过大，仅显示前10000行)";
                            limitItem.type = FileType::REGULAR_FILE;
                            previewItems.push_back(limitItem);
                            break;
                        }
                    }

                    textFile.close();
                }
            }
        } else {
            // 显示二进制文件信息（使用翻译键）
            FileItem headerItem;
            headerItem.name = _("file_browser.file_info");
            headerItem.displayName = _("file_browser.file_info");
            headerItem.type = FileType::REGULAR_FILE;
            previewItems.push_back(headerItem);

            // 添加分隔线
            FileItem separatorItem;
            separatorItem.name = "─────────────────────";
            separatorItem.displayName = "─────────────────────";
            separatorItem.type = FileType::REGULAR_FILE;
            previewItems.push_back(separatorItem);

            // 文件名
            FileItem nameItem;
            nameItem.name = _("file_browser.filename") + ": " + item.name;
            nameItem.displayName = _("file_browser.filename") + ": " + item.name;
            nameItem.type = FileType::REGULAR_FILE;
            previewItems.push_back(nameItem);

            // 文件大小
            FileItem sizeItem;
            sizeItem.name = _("file_browser.filesize") + ": " + formatFileSize(item.size);
            sizeItem.displayName = _("file_browser.filesize") + ": " + formatFileSize(item.size);
            sizeItem.type = FileType::REGULAR_FILE;
            previewItems.push_back(sizeItem);

            // 文件类型
            FileItem typeItem;
            std::string fileTypeStr;
            if (ext.empty()) {
                fileTypeStr = _("file_browser.unknown_type");
            } else if (ext == ".xlsx" || ext == ".xls") {
                fileTypeStr = _("file_browser.excel_file");
            } else if (ext == ".json") {
                fileTypeStr = _("file_browser.json_file");
            } else if (ext == ".csv") {
                fileTypeStr = _("file_browser.csv_file");
            } else {
                fileTypeStr = ext + " " + _("file_browser.file_suffix");
            }
            typeItem.name = _("file_browser.filetype") + ": " + fileTypeStr;
            typeItem.displayName = _("file_browser.filetype") + ": " + fileTypeStr;
            typeItem.type = FileType::REGULAR_FILE;
            previewItems.push_back(typeItem);

            // 如果是支持的数据文件，添加提示
            if (ext == ".csv" || ext == ".json" || ext == ".xlsx" || ext == ".xls") {
                FileItem emptyItem;
                emptyItem.name = " ";
                emptyItem.displayName = " ";
                emptyItem.type = FileType::REGULAR_FILE;
                previewItems.push_back(emptyItem);

                FileItem hintItem;
                hintItem.name = _("file_browser.supported_file");
                hintItem.displayName = _("file_browser.supported_file");
                hintItem.type = FileType::REGULAR_FILE;
                previewItems.push_back(hintItem);
            }
        }
    }
    catch (const std::exception&) {
        // 预览失败，显示错误信息
        FileItem errorItem;
        errorItem.name = _("file_browser.preview_error");
        errorItem.displayName = _("file_browser.preview_error");
        errorItem.type = FileType::REGULAR_FILE;
        previewItems.push_back(errorItem);
    }

    return previewItems;
}

std::function<Element()> FileBrowser::createRenderer(const std::string& prompt)
{
    return [this, prompt]() -> Element {
        // 动态计算最大显示项目数
        int maxDisplayItems = calculateMaxDisplayItems();

        // 标题栏
        auto title =
            hbox({text("🗂️ ") | color(Color::Yellow), text(prompt) | color(Color::Yellow) | bold}) |
            center | border | color(Color::Cyan);

        // 路径显示
        auto pathDisplay =
            hbox({text("📁 ") | color(Color::Cyan1), text(currentPath_) | color(Color::Default)}) |
            border | color(Color::Cyan);

        // 创建三栏布局
        auto leftPane = createPane(parentItems_, -1, parentScrollOffset_,
                                   _("file_browser.parent_dir"), maxDisplayItems);
        auto centerPane = createPane(currentItems_, selectedIndex_, currentScrollOffset_,
                                     _("file_browser.current_dir"), maxDisplayItems);
        auto rightPane = createPane(previewItems_, -1, previewScrollOffset_,
                                    _("file_browser.preview"), maxDisplayItems);

        // 重新设计三栏平均分配：让三栏更加均匀地分配终端宽度
        // 左栏：占1/3空间，最小25列
        // 中栏：占1/3空间，最小25列，主操作区域
        // 右栏：占1/3空间，最小25列
        // 使用相同的flex策略让三栏平均分配空间
        auto mainContent = hbox(
            {leftPane | size(WIDTH, GREATER_THAN, 25) | flex, separator() | color(Color::Cyan),
             centerPane | size(WIDTH, GREATER_THAN, 25) | flex, separator() | color(Color::Cyan),
             rightPane | size(WIDTH, GREATER_THAN, 25) | flex});

        // 帮助栏
        auto helpBar = hbox({text(_("file_browser.help_up")) | color(Color::Cyan),
                             text(" | ") | color(Color::Default),
                             text(_("file_browser.help_down")) | color(Color::Cyan),
                             text(" | ") | color(Color::Default),
                             text(_("file_browser.help_prev")) | color(Color::Cyan),
                             text(" | ") | color(Color::Default),
                             text(_("file_browser.help_next")) | color(Color::Cyan),
                             text(" | ") | color(Color::Default),
                             text(_("file_browser.help_select")) | color(Color::Green),
                             text(" | ") | color(Color::Default),
                             text(_("file_browser.help_preview_scroll")) | color(Color::Magenta),
                             text(" | ") | color(Color::Default),
                             text(_("file_browser.help_exit")) | color(Color::Red)}) |
                       border | color(Color::Cyan);

        return vbox({title, pathDisplay, mainContent | flex, helpBar});
    };
}

Element FileBrowser::createPane(const std::vector<FileItem>& items, int selectedIndex,
                                int scrollOffset, const std::string& title, int maxDisplayItems)
{
    Elements elements;

    // 添加标题
    elements.push_back(text(title) | color(Color::Yellow) | bold | center);
    elements.push_back(separator());

    if (items.empty()) {
        elements.push_back(text(_("file_browser.empty_dir")) | color(Color::GrayDark) | center);
    } else {
        int displayCount = std::min(maxDisplayItems, static_cast<int>(items.size()));
        int startIndex =
            std::max(0, std::min(scrollOffset, static_cast<int>(items.size()) - displayCount));

        for (int i = 0; i < displayCount && startIndex + i < static_cast<int>(items.size()); ++i) {
            int itemIndex = startIndex + i;
            const auto& item = items[itemIndex];
            bool isSelected = (selectedIndex == itemIndex);

            auto itemElement = text(item.displayName);

            if (isSelected) {
                itemElement = itemElement | bgcolor(Color::Cyan) | color(Color::Black);
            } else {
                itemElement = itemElement | color(getFileTypeColor(item, false));
            }

            // 只为当前目录栏（有selectedIndex）添加文件大小信息，预览栏不添加
            if (config_.showFileSize && selectedIndex >= 0 && item.type != FileType::DIRECTORY &&
                item.size > 0) {
                itemElement = hbox({itemElement, text(" (" + formatFileSize(item.size) + ")") |
                                                     color(Color::GrayLight)});
            }

            elements.push_back(itemElement);
        }

        // 添加填充空行以确保栏高度一致（当文件数少于maxDisplayItems时）
        int emptyLines = maxDisplayItems - displayCount;
        for (int i = 0; i < emptyLines; ++i) {
            elements.push_back(text(" "));
        }
    }

    return vbox(elements) | border | color(Color::Cyan);
}

bool FileBrowser::handleEvent(Event event, ScreenInteractive& screen)
{
    // 处理鼠标滚动事件（学习yazi风格）
    if (event.is_mouse()) {
        // 检查是否为鼠标滚轮事件
        if (event.mouse().button == Mouse::WheelUp) {
            scrollPreviewUp(3);  // 鼠标滚轮向上，预览栏向上滚动3行
            return true;
        }
        if (event.mouse().button == Mouse::WheelDown) {
            scrollPreviewDown(3);  // 鼠标滚轮向下，预览栏向下滚动3行
            return true;
        }

        // 其他鼠标事件暂时不处理，让FTXUI默认处理
        return false;
    }

    // Vim风格导航键
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
        // 标准方向键
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

    // 预览栏滚动支持（学习yazi，增强版）
    if (event == Event::PageUp) {
        scrollPreviewUp();
        return true;
    }
    if (event == Event::PageDown) {
        scrollPreviewDown();
        return true;
    }

    // 添加更多预览滚动快捷键（类似yazi）
    if (config_.enableVimKeys) {
        // Vim风格预览滚动：Ctrl+U/Ctrl+D
        if (event.input() == "\x15") {  // Ctrl+U
            scrollPreviewUp();
            return true;
        }
        if (event.input() == "\x04") {  // Ctrl+D
            scrollPreviewDown();
            return true;
        }

        // 单行滚动：Shift+J/Shift+K
        if (event == Event::Character('J')) {
            scrollPreviewDown(1);  // 单行向下
            return true;
        }
        if (event == Event::Character('K')) {
            scrollPreviewUp(1);  // 单行向上
            return true;
        }
    }

    // Home/End键快速跳转到预览开头/结尾
    if (event == Event::Home) {
        previewScrollOffset_ = 0;
        return true;
    }
    if (event == Event::End) {
        int maxDisplayItems = calculateMaxDisplayItems();
        int maxOffset = std::max(0, static_cast<int>(previewItems_.size()) - maxDisplayItems);
        previewScrollOffset_ = maxOffset;
        return true;
    }

    // 确认选择
    if (event == Event::Return) {
        confirmSelection(screen);
        return true;
    }

    // 退出
    if (event == Event::Character('q') || event == Event::Escape) {
        userCancelled_ = true;
        screen.ExitLoopClosure()();
        return true;
    }

    return false;
}

void FileBrowser::navigateUp()
{
    if (selectedIndex_ > 0) {
        selectedIndex_--;
        int maxDisplayItems = calculateMaxDisplayItems();
        adjustScrollOffset(selectedIndex_, currentScrollOffset_, currentItems_.size(),
                           maxDisplayItems);
        refreshPreviewPane();
    }
}

void FileBrowser::navigateDown()
{
    if (selectedIndex_ < static_cast<int>(currentItems_.size()) - 1) {
        selectedIndex_++;
        int maxDisplayItems = calculateMaxDisplayItems();
        adjustScrollOffset(selectedIndex_, currentScrollOffset_, currentItems_.size(),
                           maxDisplayItems);
        refreshPreviewPane();
    }
}

void FileBrowser::navigateLeft()
{
    // 返回父目录
    if (!parentPath_.empty()) {
        fs::path current(currentPath_);
        std::string currentDirName = current.filename().string();

        currentPath_ = parentPath_;
        refreshAllPanes();

        // 尝试在父目录中找到原来的目录并选中
        for (int i = 0; i < static_cast<int>(currentItems_.size()); ++i) {
            if (currentItems_[i].name == currentDirName) {
                selectedIndex_ = i;
                int maxDisplayItems = calculateMaxDisplayItems();
                adjustScrollOffset(selectedIndex_, currentScrollOffset_, currentItems_.size(),
                                   maxDisplayItems);
                break;
            }
        }
        refreshPreviewPane();
    }
}

void FileBrowser::navigateRight()
{
    // 进入选中的目录
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(currentItems_.size())) {
        const auto& selectedItem = currentItems_[selectedIndex_];

        if (selectedItem.type == FileType::DIRECTORY) {
            currentPath_ = selectedItem.fullPath;
            selectedIndex_ = 0;
            refreshAllPanes();
        }
    }
}

void FileBrowser::confirmSelection(ScreenInteractive& screen)
{
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(currentItems_.size())) {
        const auto& selectedItem = currentItems_[selectedIndex_];

        if (selectedItem.type == FileType::DIRECTORY) {
            if (directoriesOnly_) {
                // 选择目录
                selectedPath_ = selectedItem.fullPath;
                screen.ExitLoopClosure()();
            } else {
                // 进入目录
                navigateRight();
            }
        } else if (!directoriesOnly_) {
            // 选择文件
            selectedPath_ = selectedItem.fullPath;
            screen.ExitLoopClosure()();
        }
    }
}

void FileBrowser::scrollPreviewUp(int lines)
{
    int maxDisplayItems = calculateMaxDisplayItems();
    int scrollAmount = (lines > 0) ? lines : std::max(1, maxDisplayItems / 3);  // 默认1/3屏幕

    previewScrollOffset_ = std::max(0, previewScrollOffset_ - scrollAmount);
}

void FileBrowser::scrollPreviewDown(int lines)
{
    int maxDisplayItems = calculateMaxDisplayItems();
    int scrollAmount = (lines > 0) ? lines : std::max(1, maxDisplayItems / 3);  // 默认1/3屏幕
    int maxOffset = std::max(0, static_cast<int>(previewItems_.size()) - maxDisplayItems);

    previewScrollOffset_ = std::min(maxOffset, previewScrollOffset_ + scrollAmount);
}

void FileBrowser::adjustScrollOffset(int selectedIndex, int& scrollOffset, int itemCount,
                                     int maxDisplayItems)
{
    if (itemCount <= maxDisplayItems) {
        scrollOffset = 0;
        return;
    }

    // 保持选中项在可见区域内
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    } else if (selectedIndex >= scrollOffset + maxDisplayItems) {
        scrollOffset = selectedIndex - maxDisplayItems + 1;
    }

    // 确保偏移量在有效范围内
    scrollOffset = std::max(0, std::min(scrollOffset, itemCount - maxDisplayItems));
}

std::string FileBrowser::getFileIcon(const FileItem& item) const
{
    switch (item.type) {
        case FileType::DIRECTORY:
            return "📁";
        case FileType::ARCHIVE:
            return "📦";
        case FileType::REGULAR_FILE:
        default:
            // 根据文件扩展名返回不同图标
            if (item.name.find('.') != std::string::npos) {
                std::string ext = item.name.substr(item.name.find_last_of('.'));
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".txt" || ext == ".md") return "📄";
                if (ext == ".csv" || ext == ".json") return "📊";
                if (ext == ".cpp" || ext == ".h" || ext == ".c") return "💻";
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") return "🖼️";
                if (ext == ".pdf") return "📕";
            }
            return "📄";
    }
}

std::string FileBrowser::formatFileSize(std::size_t size) const
{
    if (size < 1024) {
        return std::to_string(size) + "B";
    } else if (size < 1024 * 1024) {
        return std::to_string(size / 1024) + "KB";
    } else if (size < 1024 * 1024 * 1024) {
        return std::to_string(size / (1024 * 1024)) + "MB";
    } else {
        return std::to_string(size / (1024 * 1024 * 1024)) + "GB";
    }
}

bool FileBrowser::isHiddenFile(const std::string& filename) const
{
#ifdef _WIN32
    return !filename.empty() && filename.front() == '.' && filename != "." && filename != "..";
#else
    return !filename.empty() && filename.front() == '.' && filename != "." && filename != "..";
#endif
}

FileBrowser::FileType FileBrowser::detectFileType(const std::string& filename,
                                                  bool isDirectory) const
{
    if (isDirectory) {
        return FileType::DIRECTORY;
    }

    // 检查是否为压缩包
    std::string lowerName = filename;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    // C++17兼容的ends_with实现
    auto endsWith = [](const std::string& str, const std::string& suffix) {
        if (suffix.length() > str.length()) return false;
        return str.substr(str.length() - suffix.length()) == suffix;
    };

    if (endsWith(lowerName, ".zip") || endsWith(lowerName, ".rar") || endsWith(lowerName, ".7z") ||
        endsWith(lowerName, ".tar") || endsWith(lowerName, ".gz") || endsWith(lowerName, ".bz2")) {
        return FileType::ARCHIVE;
    }

    return FileType::REGULAR_FILE;
}

Color FileBrowser::getFileTypeColor(const FileItem& item, bool isSelected) const
{
    if (isSelected) {
        return Color::Black;
    }

    // 检查是否为支持的数据文件类型，添加特殊高亮
    if (item.name.find('.') != std::string::npos) {
        std::string ext = item.name.substr(item.name.find_last_of('.'));
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        // 支持的数据文件类型使用特殊颜色
        if (ext == ".json") {
            return Color::Yellow;  // JSON文件用黄色
        }
        if (ext == ".csv") {
            return Color::Green;  // CSV文件用绿色
        }
        if (ext == ".xlsx" || ext == ".xls") {
            return Color::SeaGreen1;  // Excel文件用海绿色
        }
    }

    // 默认文件类型颜色
    switch (item.type) {
        case FileType::DIRECTORY:
            return Color::Cyan1;  // 使用淡蓝色，更显眼
        case FileType::ARCHIVE:
            return Color::Magenta;
        case FileType::REGULAR_FILE:
            return Color::White;
        default:
            return Color::Default;
    }
}

}}  // namespace neumann::cli