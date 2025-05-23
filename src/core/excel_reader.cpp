#include "core/excel_reader.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

#include "core/data_manager.h"
#include "core/error_handler.h"
#include "core/i18n.h"

namespace fs = std::filesystem;

namespace neumann {

ExcelReader::ExcelReader() {}

ExcelReader::~ExcelReader() {}

bool ExcelReader::isExcelFile(const std::string& filename)
{
    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return (ext == ".xlsx" || ext == ".xls" || ext == ".csv");
}

DataSet ExcelReader::importFromExcel(const std::string& filename, const std::string& sheetName,
                                     bool hasHeader)
{
    DataSet dataSet;

    // 检查文件是否存在
    if (!fs::exists(filename)) {
        THROW_ERROR(ErrorCode::FILE_NOT_FOUND, filename);
    }

    // 目前先实现CSV格式支持，后续可扩展真正的Excel支持
    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".csv") {
        return DataManager::getInstance().importFromCSV(filename, hasHeader);
    } else if (ext == ".xlsx" || ext == ".xls") {
        // 对于真正的Excel文件，目前返回错误提示用户转换为CSV
        THROW_ERROR(
            ErrorCode::INVALID_DATA_FORMAT,
            "Excel files (.xlsx/.xls) are not yet supported. Please convert to CSV format.");
    }

    THROW_ERROR(ErrorCode::INVALID_DATA_FORMAT, "Unsupported file format");
}

std::vector<std::string> ExcelReader::getSheetNames(const std::string& filename)
{
    // 对于CSV文件，只有一个"工作表"
    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".csv") {
        return {"Sheet1"};
    }

    // 对于真正的Excel文件，目前返回空列表
    return {};
}

std::vector<std::vector<std::string>> ExcelReader::previewExcelData(const std::string& filename,
                                                                    const std::string& sheetName,
                                                                    int maxRows)
{
    std::vector<std::vector<std::string>> preview;

    if (!fs::exists(filename)) {
        return preview;
    }

    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext != ".csv") {
        return preview;  // 目前只支持CSV预览
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        return preview;
    }

    std::string line;
    int rowCount = 0;

    while (std::getline(file, line) && rowCount < maxRows) {
        if (line.empty()) continue;

        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;

        // 简单的CSV解析
        while (std::getline(ss, cell, ',')) {
            // 去除首尾空格
            cell.erase(0, cell.find_first_not_of(" \t\r\n"));
            cell.erase(cell.find_last_not_of(" \t\r\n") + 1);
            row.push_back(cell);
        }

        preview.push_back(row);
        rowCount++;
    }

    file.close();
    return preview;
}

std::map<int, std::string> ExcelReader::detectColumnTypes(
    const std::vector<std::vector<std::string>>& data)
{
    std::map<int, std::string> columnTypes;

    if (data.empty()) {
        return columnTypes;
    }

    size_t columnCount = data[0].size();

    for (size_t col = 0; col < columnCount; ++col) {
        int numericCount = 0;
        int totalCount = 0;

        for (size_t row = 1; row < data.size() && row < 20; ++row) {  // 检查前20行
            if (col < data[row].size() && !data[row][col].empty()) {
                totalCount++;
                double value;
                if (tryParseDouble(data[row][col], value)) {
                    numericCount++;
                }
            }
        }

        if (totalCount > 0) {
            double numericRatio = static_cast<double>(numericCount) / totalCount;
            if (numericRatio > 0.8) {
                columnTypes[col] = "Numeric";
            } else {
                columnTypes[col] = "Text";
            }
        } else {
            columnTypes[col] = "Unknown";
        }
    }

    return columnTypes;
}

std::pair<bool, std::string> ExcelReader::validateData(const DataSet& dataSet)
{
    if (dataSet.dataPoints.empty()) {
        return {false, "No data points found"};
    }

    if (dataSet.dataPoints.size() < 4) {
        return {false, "Insufficient data points (minimum 4 required for Neumann test)"};
    }

    if (dataSet.timePoints.size() != dataSet.dataPoints.size()) {
        return {false, "Time points and data points count mismatch"};
    }

    // 检查数据是否包含无效值
    for (double value : dataSet.dataPoints) {
        if (std::isnan(value) || std::isinf(value)) {
            return {false, "Data contains invalid numeric values (NaN or Infinity)"};
        }
    }

    for (double value : dataSet.timePoints) {
        if (std::isnan(value) || std::isinf(value)) {
            return {false, "Time points contain invalid numeric values (NaN or Infinity)"};
        }
    }

    return {true, ""};
}

std::vector<double> ExcelReader::cleanNumericData(const std::vector<std::string>& rawData)
{
    std::vector<double> cleanData;

    for (const auto& str : rawData) {
        double value;
        if (tryParseDouble(str, value)) {
            cleanData.push_back(value);
        }
    }

    return cleanData;
}

std::pair<int, int> ExcelReader::detectTimeAndDataColumns(
    const std::vector<std::vector<std::string>>& data)
{
    if (data.empty() || data[0].size() < 2) {
        return {0, 1};  // 默认前两列
    }

    // 检测列类型
    auto columnTypes = detectColumnTypes(data);

    int timeColumn = -1;
    int dataColumn = -1;

    // 查找第一个数值列作为时间列
    for (const auto& [col, type] : columnTypes) {
        if (type == "Numeric" && timeColumn == -1) {
            timeColumn = col;
        } else if (type == "Numeric" && dataColumn == -1) {
            dataColumn = col;
            break;
        }
    }

    // 如果只找到一个数值列，假设第一列是时间，第二列是数据
    if (timeColumn == -1) timeColumn = 0;
    if (dataColumn == -1) dataColumn = (timeColumn == 0) ? 1 : 0;

    return {timeColumn, dataColumn};
}

bool ExcelReader::tryParseDouble(const std::string& str, double& result)
{
    if (str.empty()) {
        return false;
    }

    // 去除空格
    std::string cleanStr = str;
    cleanStr.erase(0, cleanStr.find_first_not_of(" \t\r\n"));
    cleanStr.erase(cleanStr.find_last_not_of(" \t\r\n") + 1);

    if (cleanStr.empty()) {
        return false;
    }

    try {
        size_t pos;
        result = std::stod(cleanStr, &pos);

        // 确保整个字符串都被解析
        return pos == cleanStr.length();
    }
    catch (...) {
        return false;
    }
}

}  // namespace neumann