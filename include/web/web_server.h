#pragma once

#include <memory>
#include <string>

namespace neumann { namespace web {

// 前向声明Impl类
class WebServerImpl;

/**
 * @brief Web服务器类
 *
 * 提供基于HTTP的Web接口，用于在浏览器中访问诺依曼趋势测试功能
 */
class WebServer
{
public:
    /**
     * @brief 构造函数
     * @param port 服务器监听端口
     * @param webRootDir Web资源文件目录
     */
    WebServer(int port = 8080, const std::string &webRootDir = "web");

    /**
     * @brief 析构函数
     */
    ~WebServer();

    /**
     * @brief 启动Web服务器
     * @param background 是否在后台运行
     */
    void start(bool background = false);

    /**
     * @brief 停止Web服务器
     */
    void stop();

    /**
     * @brief 获取服务器状态
     * @return 服务器是否运行中
     */
    bool isRunning() const;

    /**
     * @brief 获取服务器URL
     * @return 服务器访问URL
     */
    std::string getUrl() const;

private:
    // 服务器端口
    int port;

    // Web资源目录
    std::string webRootDir;

    // 运行状态
    bool running;

    // PIMPL实现
    std::unique_ptr<WebServerImpl> impl;

    /**
     * @brief 初始化路由
     */
    void initializeRoutes();

    /**
     * @brief 注册API端点
     */
    void registerApiEndpoints();

    /**
     * @brief 处理静态文件请求
     * @param path 文件路径
     * @return HTTP响应
     */
    std::string handleStaticFile(const std::string &path);

    // 核心功能API处理函数
    /**
     * @brief 处理诺依曼趋势测试请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleNeumannTestRequest(const std::string &requestBody);

    // 数据集管理API处理函数
    /**
     * @brief 处理数据集列表请求
     * @return JSON响应
     */
    std::string handleDataSetListRequest();

    /**
     * @brief 处理数据集加载请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleDataSetLoadRequest(const std::string &requestBody);

    /**
     * @brief 处理数据集加载请求（通过名称）
     * @param name 数据集名称
     * @return JSON响应
     */
    std::string handleDataSetLoadRequestByName(const std::string &name);

    /**
     * @brief 处理数据集保存请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleDataSetSaveRequest(const std::string &requestBody);

    /**
     * @brief 处理数据集删除请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleDataSetDeleteRequest(const std::string &requestBody);

    // 样本数据API处理函数
    /**
     * @brief 处理样本数据列表请求
     * @return JSON响应
     */
    std::string handleSampleDataListRequest();

    /**
     * @brief 处理样本数据加载请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleSampleDataLoadRequest(const std::string &requestBody);

    // 配置和设置API处理函数
    /**
     * @brief 处理配置获取请求
     * @return JSON响应
     */
    std::string handleConfigGetRequest();

    /**
     * @brief 处理配置更新请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleConfigUpdateRequest(const std::string &requestBody);

    /**
     * @brief 处理语言切换请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleLanguageChangeRequest(const std::string &requestBody);

    /**
     * @brief 处理置信度切换请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleConfidenceLevelChangeRequest(const std::string &requestBody);

    // 标准值和自定义置信度API处理函数
    /**
     * @brief 处理标准值获取请求
     * @return JSON响应
     */
    std::string handleStandardValuesGetRequest();

    /**
     * @brief 处理自定义置信度导入请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleCustomConfidenceImportRequest(const std::string &requestBody);

    /**
     * @brief 处理自定义置信度列表请求
     * @return JSON响应
     */
    std::string handleCustomConfidenceListRequest();

    /**
     * @brief 处理自定义置信度删除请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleCustomConfidenceDeleteRequest(const std::string &requestBody);

    // 数据可视化API处理函数
    /**
     * @brief 处理图表数据请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleChartDataRequest(const std::string &requestBody);

    /**
     * @brief 处理SVG导出请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleExportSVGRequest(const std::string &requestBody);

    /**
     * @brief 处理SVG文件列表请求
     * @return JSON响应
     */
    std::string handleSVGListRequest();

    /**
     * @brief 处理SVG文件内容请求
     * @param filename SVG文件名
     * @return SVG文件内容或错误响应
     */
    std::string handleSVGFileRequest(const std::string &filename);

    // 批量处理API处理函数
    /**
     * @brief 处理批量处理请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleBatchProcessRequest(const std::string &requestBody);

    /**
     * @brief 处理批量状态查询请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleBatchStatusRequest(const std::string &requestBody);

    // 文件导入API处理函数
    /**
     * @brief 处理CSV导入请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleCSVImportRequest(const std::string &requestBody);

    /**
     * @brief 处理文件上传请求
     * @param requestBody 请求体
     * @return JSON响应
     */
    std::string handleFileUploadRequest(const std::string &requestBody);

    // 国际化API处理函数
    /**
     * @brief 处理翻译获取请求
     * @param language 语言代码
     * @return JSON响应
     */
    std::string handleTranslationsGetRequest(const std::string &language);

    // 统计分析API处理函数
    /**
     * @brief 处理统计信息请求
     * @return JSON响应
     */
    std::string handleStatisticsRequest();
};

}}  // namespace neumann::web