#pragma once

#include <functional>
#include <string>
#include <vector>

#include "data_manager.h"
#include "neumann_calculator.h"

namespace neumann {

/**
 * @brief 批量处理结果结构
 */
struct BatchProcessResult {
    std::string filename;
    std::string status;  // "success", "error", "skipped"
    std::string errorMessage;
    NeumannTestResults testResults;
    double processingTime;  // 处理时间（秒）
};

/**
 * @brief 批量处理统计信息
 */
struct BatchProcessStats {
    int totalFiles;
    int processedFiles;
    int successfulFiles;
    int errorFiles;
    int skippedFiles;
    int filesWithTrend;
    double totalProcessingTime;
    std::vector<std::string> supportedFormats;
};

/**
 * @brief 批量数据处理器
 * 
 * 提供多文件批量处理、进度跟踪和结果汇总功能
 */
class BatchProcessor
{
public:
    /**
     * @brief 进度回调函数类型
     * @param current 当前处理文件数
     * @param total 总文件数
     * @param filename 当前处理的文件名
     */
    using ProgressCallback =
        std::function<void(int current, int total, const std::string& filename)>;

    /**
     * @brief 构造函数
     * @param confidenceLevel 置信度水平
     */
    explicit BatchProcessor(double confidenceLevel = 0.95);

    /**
     * @brief 设置置信度水平
     */
    void setConfidenceLevel(double level);

    /**
     * @brief 处理指定目录中的所有数据文件
     * @param directoryPath 目录路径
     * @param progressCallback 进度回调函数
     * @return 批量处理结果
     */
    std::vector<BatchProcessResult> processDirectory(const std::string& directoryPath,
                                                     ProgressCallback progressCallback = nullptr);

    /**
     * @brief 处理指定的文件列表
     * @param filePaths 文件路径列表
     * @param progressCallback 进度回调函数
     * @return 批量处理结果
     */
    std::vector<BatchProcessResult> processFiles(const std::vector<std::string>& filePaths,
                                                 ProgressCallback progressCallback = nullptr);

    /**
     * @brief 处理单个文件
     * @param filePath 文件路径
     * @return 处理结果
     */
    BatchProcessResult processSingleFile(const std::string& filePath);

    /**
     * @brief 生成批量处理统计信息
     * @param results 批量处理结果
     * @return 统计信息
     */
    static BatchProcessStats generateStatistics(const std::vector<BatchProcessResult>& results);

    /**
     * @brief 导出批量处理结果到CSV文件
     * @param results 批量处理结果
     * @param outputPath 输出文件路径
     * @return 是否成功导出
     */
    static bool exportResultsToCSV(const std::vector<BatchProcessResult>& results,
                                   const std::string& outputPath);

    /**
     * @brief 导出批量处理结果到HTML报告
     * @param results 批量处理结果
     * @param outputPath 输出文件路径
     * @return 是否成功导出
     */
    static bool exportResultsToHTML(const std::vector<BatchProcessResult>& results,
                                    const std::string& outputPath);

    /**
     * @brief 获取支持的文件格式
     * @return 支持的文件扩展名列表
     */
    static std::vector<std::string> getSupportedFormats();

private:
    double confidenceLevel;

    /**
     * @brief 检查文件是否为支持的格式
     */
    static bool isSupportedFile(const std::string& filePath);

    /**
     * @brief 获取目录中的所有支持文件
     */
    static std::vector<std::string> findSupportedFiles(const std::string& directoryPath);
};

}  // namespace neumann