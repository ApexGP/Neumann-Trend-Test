#include "core/batch_processor.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "core/excel_reader.h"

namespace fs = std::filesystem;

namespace neumann {

BatchProcessor::BatchProcessor(double confidenceLevel) : confidenceLevel(confidenceLevel) {}

void BatchProcessor::setConfidenceLevel(double level)
{
    confidenceLevel = level;
}

std::vector<BatchProcessResult> BatchProcessor::processDirectory(const std::string& directoryPath,
                                                                 ProgressCallback progressCallback)
{
    std::vector<std::string> files = findSupportedFiles(directoryPath);
    return processFiles(files, progressCallback);
}

std::vector<BatchProcessResult> BatchProcessor::processFiles(
    const std::vector<std::string>& filePaths, ProgressCallback progressCallback)
{
    std::vector<BatchProcessResult> results;
    results.reserve(filePaths.size());

    for (size_t i = 0; i < filePaths.size(); ++i) {
        if (progressCallback) {
            progressCallback(static_cast<int>(i), static_cast<int>(filePaths.size()), filePaths[i]);
        }

        BatchProcessResult result = processSingleFile(filePaths[i]);
        results.push_back(std::move(result));
    }

    // 最终进度回调
    if (progressCallback) {
        progressCallback(static_cast<int>(filePaths.size()), static_cast<int>(filePaths.size()),
                         "Complete");
    }

    return results;
}

BatchProcessResult BatchProcessor::processSingleFile(const std::string& filePath)
{
    BatchProcessResult result;
    result.filename = fs::path(filePath).filename().string();

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        if (!fs::exists(filePath)) {
            result.status = "error";
            result.errorMessage = "File not found";
            return result;
        }

        if (!isSupportedFile(filePath)) {
            result.status = "skipped";
            result.errorMessage = "Unsupported file format";
            return result;
        }

        // 根据文件类型选择相应的加载方法
        DataSet dataSet;
        std::string extension = fs::path(filePath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == ".csv") {
            dataSet = DataManager::getInstance().importFromCSV(filePath, true);  // 假设有表头
        } else if (extension == ".xlsx" || extension == ".xls") {
            ExcelReader reader;
            dataSet = reader.importFromExcel(filePath, "", true);  // 假设有表头
        } else {
            result.status = "error";
            result.errorMessage = "Unsupported file format: " + extension;
            return result;
        }

        // 检查数据有效性
        if (dataSet.dataPoints.size() < 4) {
            result.status = "error";
            result.errorMessage = "Insufficient data points (minimum 4 required)";
            return result;
        }

        // 执行诺依曼趋势测试
        NeumannCalculator calculator(confidenceLevel);
        result.testResults = calculator.performTest(dataSet.dataPoints, dataSet.timePoints);

        result.status = "success";
        result.errorMessage = "";
    }
    catch (const std::exception& e) {
        result.status = "error";
        result.errorMessage = e.what();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    result.processingTime = duration.count() / 1000.0;

    return result;
}

BatchProcessStats BatchProcessor::generateStatistics(const std::vector<BatchProcessResult>& results)
{
    BatchProcessStats stats;
    stats.totalFiles = static_cast<int>(results.size());
    stats.processedFiles = 0;
    stats.successfulFiles = 0;
    stats.errorFiles = 0;
    stats.skippedFiles = 0;
    stats.filesWithTrend = 0;
    stats.totalProcessingTime = 0.0;
    stats.supportedFormats = getSupportedFormats();

    for (const auto& result : results) {
        stats.totalProcessingTime += result.processingTime;

        if (result.status == "success") {
            stats.successfulFiles++;
            stats.processedFiles++;
            if (result.testResults.overallTrend) {
                stats.filesWithTrend++;
            }
        } else if (result.status == "error") {
            stats.errorFiles++;
            stats.processedFiles++;
        } else if (result.status == "skipped") {
            stats.skippedFiles++;
        }
    }

    return stats;
}

bool BatchProcessor::exportResultsToCSV(const std::vector<BatchProcessResult>& results,
                                        const std::string& outputPath)
{
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            return false;
        }

        // CSV标题行
        file << "Filename,Status,Processing Time (s),Data Points,Overall Trend,Min PG,Max PG,Avg "
                "PG,Error Message\n";

        for (const auto& result : results) {
            file << result.filename << ",";
            file << result.status << ",";
            file << std::fixed << std::setprecision(3) << result.processingTime << ",";

            if (result.status == "success") {
                file << result.testResults.data.size() << ",";
                file << (result.testResults.overallTrend ? "YES" : "NO") << ",";
                file << std::setprecision(6) << result.testResults.minPG << ",";
                file << result.testResults.maxPG << ",";
                file << result.testResults.avgPG << ",";
                file << "\n";
            } else {
                file << ",,,,," << result.errorMessage << "\n";
            }
        }

        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool BatchProcessor::exportResultsToHTML(const std::vector<BatchProcessResult>& results,
                                         const std::string& outputPath)
{
    try {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            return false;
        }

        // 生成HTML报告
        file << "<!DOCTYPE html>\n";
        file << "<html lang='zh-CN'>\n";
        file << "<head>\n";
        file << "    <meta charset='UTF-8'>\n";
        file << "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
        file << "    <title>Neumann Trend Test - Batch Processing Report</title>\n";
        file << "    <style>\n";
        file << "        body { font-family: Arial, sans-serif; margin: 20px; }\n";
        file << "        .header { background-color: #f0f8ff; padding: 20px; border-radius: 8px; "
                "margin-bottom: 20px; }\n";
        file << "        .stats { display: flex; gap: 20px; margin-bottom: 20px; }\n";
        file << "        .stat-card { background: #f9f9f9; padding: 15px; border-radius: 5px; "
                "flex: 1; }\n";
        file << "        table { width: 100%; border-collapse: collapse; }\n";
        file << "        th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; "
                "}\n";
        file << "        th { background-color: #f2f2f2; }\n";
        file << "        .success { color: green; }\n";
        file << "        .error { color: red; }\n";
        file << "        .skipped { color: orange; }\n";
        file << "        .trend-yes { background-color: #ffebee; }\n";
        file << "        .trend-no { background-color: #e8f5e8; }\n";
        file << "    </style>\n";
        file << "</head>\n";
        file << "<body>\n";

        // 报告标题
        file << "    <div class='header'>\n";
        file << "        <h1>诺依曼趋势测试 - 批量处理报告</h1>\n";
        auto now = std::time(nullptr);
        file << "        <p>生成时间: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
             << "</p>\n";
        file << "    </div>\n";

        // 统计信息
        BatchProcessStats stats = generateStatistics(results);
        file << "    <div class='stats'>\n";
        file << "        <div class='stat-card'>\n";
        file << "            <h3>总文件数</h3>\n";
        file << "            <h2>" << stats.totalFiles << "</h2>\n";
        file << "        </div>\n";
        file << "        <div class='stat-card'>\n";
        file << "            <h3>成功处理</h3>\n";
        file << "            <h2 class='success'>" << stats.successfulFiles << "</h2>\n";
        file << "        </div>\n";
        file << "        <div class='stat-card'>\n";
        file << "            <h3>处理错误</h3>\n";
        file << "            <h2 class='error'>" << stats.errorFiles << "</h2>\n";
        file << "        </div>\n";
        file << "        <div class='stat-card'>\n";
        file << "            <h3>发现趋势</h3>\n";
        file << "            <h2>" << stats.filesWithTrend << "</h2>\n";
        file << "        </div>\n";
        file << "        <div class='stat-card'>\n";
        file << "            <h3>总处理时间</h3>\n";
        file << "            <h2>" << std::fixed << std::setprecision(2)
             << stats.totalProcessingTime << "s</h2>\n";
        file << "        </div>\n";
        file << "    </div>\n";

        // 详细结果表格
        file << "    <h2>详细结果</h2>\n";
        file << "    <table>\n";
        file << "        <thead>\n";
        file << "            <tr>\n";
        file << "                <th>文件名</th>\n";
        file << "                <th>状态</th>\n";
        file << "                <th>处理时间(s)</th>\n";
        file << "                <th>数据点数</th>\n";
        file << "                <th>总体趋势</th>\n";
        file << "                <th>最小PG值</th>\n";
        file << "                <th>最大PG值</th>\n";
        file << "                <th>平均PG值</th>\n";
        file << "                <th>错误信息</th>\n";
        file << "            </tr>\n";
        file << "        </thead>\n";
        file << "        <tbody>\n";

        for (const auto& result : results) {
            std::string rowClass = "";
            if (result.status == "success" && result.testResults.overallTrend) {
                rowClass = "trend-yes";
            } else if (result.status == "success") {
                rowClass = "trend-no";
            }

            file << "            <tr class='" << rowClass << "'>\n";
            file << "                <td>" << result.filename << "</td>\n";
            file << "                <td class='" << result.status << "'>" << result.status
                 << "</td>\n";
            file << "                <td>" << std::fixed << std::setprecision(3)
                 << result.processingTime << "</td>\n";

            if (result.status == "success") {
                file << "                <td>" << result.testResults.data.size() << "</td>\n";
                file << "                <td>" << (result.testResults.overallTrend ? "是" : "否")
                     << "</td>\n";
                file << "                <td>" << std::setprecision(6) << result.testResults.minPG
                     << "</td>\n";
                file << "                <td>" << result.testResults.maxPG << "</td>\n";
                file << "                <td>" << result.testResults.avgPG << "</td>\n";
                file << "                <td></td>\n";
            } else {
                file << "                <td>-</td>\n";
                file << "                <td>-</td>\n";
                file << "                <td>-</td>\n";
                file << "                <td>-</td>\n";
                file << "                <td>-</td>\n";
                file << "                <td>" << result.errorMessage << "</td>\n";
            }

            file << "            </tr>\n";
        }

        file << "        </tbody>\n";
        file << "    </table>\n";
        file << "</body>\n";
        file << "</html>";

        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::vector<std::string> BatchProcessor::getSupportedFormats()
{
    return {".csv", ".xlsx", ".xls"};
}

bool BatchProcessor::isSupportedFile(const std::string& filePath)
{
    std::string extension = fs::path(filePath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    auto supportedFormats = getSupportedFormats();
    return std::find(supportedFormats.begin(), supportedFormats.end(), extension) !=
           supportedFormats.end();
}

std::vector<std::string> BatchProcessor::findSupportedFiles(const std::string& directoryPath)
{
    std::vector<std::string> supportedFiles;

    try {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            return supportedFiles;
        }

        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && isSupportedFile(entry.path().string())) {
                supportedFiles.push_back(entry.path().string());
            }
        }

        // 按文件名排序
        std::sort(supportedFiles.begin(), supportedFiles.end());
    }
    catch (const std::exception&) {
        // 出错时返回空列表
    }

    return supportedFiles;
}

}  // namespace neumann