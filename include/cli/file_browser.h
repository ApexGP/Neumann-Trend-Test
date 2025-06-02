#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

// 确保max_align_t定义可用
#ifdef _WIN32
#include <malloc.h>

#include <cstdlib>

#else
#include <cstdlib>
#endif

// FTXUI支持（静态编译，总是可用）
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace neumann { namespace cli {

/**
 * @brief 现代化文件浏览器类（三栏布局，类似yazi）
 * 
 * 提供现代文件管理器的交互式文件选择界面
 * 左栏：父目录内容
 * 中栏：当前目录内容 
 * 右栏：预览/子目录内容
 */
class FileBrowser
{
public:
    /**
     * @brief 文件类型枚举
     */
    enum class FileType {
        PARENT_DIR,   // 上级目录
        DIRECTORY,    // 目录
        ARCHIVE,      // 压缩包
        REGULAR_FILE  // 普通文件
    };

    /**
     * @brief 文件项结构
     */
    struct FileItem {
        std::string name;         // 文件名
        std::string displayName;  // 显示名称（包含图标）
        std::string fullPath;     // 完整路径
        FileType type;            // 文件类型
        std::size_t size;         // 文件大小
        bool isHidden;            // 是否隐藏文件
    };

    /**
     * @brief 浏览器配置结构
     */
    struct BrowserConfig {
        bool showHiddenFiles;  // 是否显示隐藏文件
        bool showFileSize;     // 是否显示文件大小
        bool enableVimKeys;    // 是否启用Vim快捷键
        int maxDisplayItems;   // 每栏最大显示项目数

        // 默认构造函数
        BrowserConfig()
            : showHiddenFiles(false), showFileSize(true), enableVimKeys(true), maxDisplayItems(40)
        {
        }
    };

    /**
     * @brief 构造函数
     * @param config 浏览器配置
     */
    explicit FileBrowser(const BrowserConfig& config = BrowserConfig{});

    /**
     * @brief 析构函数
     */
    ~FileBrowser() = default;

    /**
     * @brief 启动文件浏览器进行文件选择
     * @param prompt 提示信息
     * @param directoriesOnly 是否只选择目录
     * @param startPath 起始路径（默认为当前目录）
     * @return 选择的文件路径，空字符串表示用户取消
     */
    std::string selectFile(const std::string& prompt, bool directoriesOnly = false,
                           const std::string& startPath = "");

    /**
     * @brief 启动文件浏览器进行目录选择
     * @param prompt 提示信息
     * @param startPath 起始路径（默认为当前目录）
     * @return 选择的目录路径，空字符串表示用户取消
     */
    std::string selectDirectory(const std::string& prompt, const std::string& startPath = "");

    /**
     * @brief 设置浏览器配置
     * @param config 新的配置
     */
    void setConfig(const BrowserConfig& config);

    /**
     * @brief 获取当前配置
     * @return 当前配置
     */
    const BrowserConfig& getConfig() const;

    /**
     * @brief 处理所有事件（键盘和鼠标）
     * @param event 事件对象
     * @param screen 屏幕对象引用
     * @return 是否处理了该事件
     */
    bool handleEvent(ftxui::Event event, ftxui::ScreenInteractive& screen);

private:
    BrowserConfig config_;                // 浏览器配置
    std::string currentPath_;             // 当前路径
    std::string parentPath_;              // 父目录路径
    std::vector<FileItem> parentItems_;   // 父目录文件列表
    std::vector<FileItem> currentItems_;  // 当前目录文件列表
    std::vector<FileItem> previewItems_;  // 预览/子目录文件列表
    int selectedIndex_;                   // 当前选中项索引
    int parentScrollOffset_;              // 父目录滚动偏移
    int currentScrollOffset_;             // 当前目录滚动偏移
    int previewScrollOffset_;             // 预览区滚动偏移
    bool directoriesOnly_;                // 是否只选择目录
    bool userCancelled_;                  // 用户是否取消
    std::string selectedPath_;            // 选择的路径

    /**
     * @brief 刷新所有栏的文件列表
     */
    void refreshAllPanes();

    /**
     * @brief 刷新父目录列表
     */
    void refreshParentPane();

    /**
     * @brief 刷新当前目录列表
     */
    void refreshCurrentPane();

    /**
     * @brief 刷新预览区列表
     */
    void refreshPreviewPane();

    /**
     * @brief 获取目录中的文件列表
     * @param path 目录路径
     * @return 文件列表
     */
    std::vector<FileItem> getDirectoryItems(const std::string& path);

    /**
     * @brief 获取文件内容预览
     * @param item 文件项
     * @return 预览内容列表（作为虚拟文件项）
     */
    std::vector<FileItem> getFilePreview(const FileItem& item);

    /**
     * @brief 创建FTXUI渲染器
     * @param prompt 提示信息
     * @return 渲染函数
     */
    std::function<ftxui::Element()> createRenderer(const std::string& prompt);

    /**
     * @brief 调整滚动偏移量
     * @param selectedIndex 选中索引
     * @param scrollOffset 滚动偏移引用
     * @param itemCount 项目总数
     * @param maxDisplayItems 最大显示项目数
     */
    void adjustScrollOffset(int selectedIndex, int& scrollOffset, int itemCount,
                            int maxDisplayItems);

    /**
     * @brief 动态计算最大显示项目数（基于终端高度）
     * @return 可显示的最大项目数
     */
    int calculateMaxDisplayItems() const;

    /**
     * @brief 创建单个文件栏的元素
     * @param items 文件列表
     * @param selectedIndex 选中索引（-1表示无选中）
     * @param scrollOffset 滚动偏移
     * @param title 栏标题
     * @param maxDisplayItems 最大显示项目数
     * @return 栏元素
     */
    ftxui::Element createPane(const std::vector<FileItem>& items, int selectedIndex,
                              int scrollOffset, const std::string& title, int maxDisplayItems);

    /**
     * @brief 向上导航
     */
    void navigateUp();

    /**
     * @brief 向下导航
     */
    void navigateDown();

    /**
     * @brief 向左导航（到父目录）
     */
    void navigateLeft();

    /**
     * @brief 向右导航（进入子目录）
     */
    void navigateRight();

    /**
     * @brief 确认选择
     * @param screen 屏幕对象引用
     */
    void confirmSelection(ftxui::ScreenInteractive& screen);

    /**
     * @brief 预览栏向上滚动
     * @param lines 滚动行数（默认为0，表示使用默认滚动量）
     */
    void scrollPreviewUp(int lines = 0);

    /**
     * @brief 预览栏向下滚动
     * @param lines 滚动行数（默认为0，表示使用默认滚动量）
     */
    void scrollPreviewDown(int lines = 0);

    /**
     * @brief 获取文件类型图标
     * @param item 文件项
     * @return 图标字符串
     */
    std::string getFileIcon(const FileItem& item) const;

    /**
     * @brief 获取文件大小的可读格式
     * @param size 文件大小（字节）
     * @return 格式化的大小字符串
     */
    std::string formatFileSize(std::size_t size) const;

    /**
     * @brief 检查文件是否为隐藏文件
     * @param filename 文件名
     * @return 是否为隐藏文件
     */
    bool isHiddenFile(const std::string& filename) const;

    /**
     * @brief 检测文件类型
     * @param filename 文件名
     * @param isDirectory 是否为目录
     * @return 文件类型
     */
    FileType detectFileType(const std::string& filename, bool isDirectory) const;

    /**
     * @brief 获取FTXUI颜色
     * @param item 文件项
     * @param isSelected 是否选中
     * @return FTXUI颜色
     */
    ftxui::Color getFileTypeColor(const FileItem& item, bool isSelected) const;
};

}}  // namespace neumann::cli