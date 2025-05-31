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

    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".csv") {
        return DataManager::getInstance().importFromCSV(filename, hasHeader);
    } else if (ext == ".xlsx") {
        return importFromXlsx(filename, sheetName, hasHeader);
    } else if (ext == ".xls") {
        // 对于.xls文件，提示用户转换为.xlsx或.csv
        THROW_ERROR(
            ErrorCode::INVALID_DATA_FORMAT,
            "Legacy Excel format (.xls) is not supported. Please save as .xlsx or .csv format.");
    }

    THROW_ERROR(ErrorCode::INVALID_DATA_FORMAT, "Unsupported file format");
}

DataSet ExcelReader::importFromXlsx(const std::string& filename, const std::string& sheetName,
                                    bool hasHeader)
{
    DataSet dataSet;

    try {
        // 尝试将xlsx文件作为ZIP文件解压并读取
        std::string tempDir = extractXlsxToTemp(filename);

        // 读取共享字符串表
        auto sharedStrings = readSharedStrings(tempDir + "/xl/sharedStrings.xml");

        // 查找目标工作表
        std::string worksheetPath = findWorksheet(tempDir, sheetName);
        if (worksheetPath.empty()) {
            cleanupTempDir(tempDir);
            THROW_ERROR(ErrorCode::INVALID_DATA_FORMAT, "Worksheet not found: " + sheetName);
        }

        // 读取工作表数据
        auto rawData = readWorksheetData(worksheetPath, sharedStrings);

        // 清理临时目录
        cleanupTempDir(tempDir);

        if (rawData.empty()) {
            THROW_ERROR(ErrorCode::INVALID_DATA_FORMAT, "No data found in worksheet");
        }

        // 处理数据
        dataSet = processWorksheetData(rawData, hasHeader);

        // 设置基本信息
        dataSet.name = fs::path(filename).stem().string();
        dataSet.source = filename;

        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
        dataSet.createdAt = ss.str();
    }
    catch (const std::exception& e) {
        // 如果Excel解析失败，尝试降级处理
        auto& i18n = I18n::getInstance();
        std::string errorMsg = i18n.getText("excel.parse_failed") + ": " + e.what() + "\n" +
                               i18n.getText("excel.fallback_suggestion");
        THROW_ERROR(ErrorCode::INVALID_DATA_FORMAT, errorMsg);
    }

    return dataSet;
}

std::string ExcelReader::extractXlsxToTemp(const std::string& filename)
{
    // 创建临时目录
    std::string tempDir =
        fs::temp_directory_path().string() + "/neumann_excel_" +
        std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    try {
        fs::create_directories(tempDir);

        // 使用系统命令解压ZIP文件（适用于大多数系统）
        std::string command;
#ifdef _WIN32
        // Windows下使用PowerShell解压
        command = "powershell -Command \"Expand-Archive -Path '" + filename +
                  "' -DestinationPath '" + tempDir + "' -Force\"";
#else
        // Linux/macOS下使用unzip
        command = "unzip -q '" + filename + "' -d '" + tempDir + "'";
#endif

        int result = std::system(command.c_str());
        if (result != 0) {
            throw std::runtime_error("Failed to extract xlsx file");
        }

        return tempDir;
    }
    catch (const std::exception& e) {
        // 清理可能创建的目录
        try {
            if (fs::exists(tempDir)) {
                fs::remove_all(tempDir);
            }
        }
        catch (...) {
        }
        throw;
    }
}

std::vector<std::string> ExcelReader::readSharedStrings(const std::string& filePath)
{
    std::vector<std::string> sharedStrings;

    if (!fs::exists(filePath)) {
        return sharedStrings;  // 某些文件可能没有共享字符串
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return sharedStrings;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 简单的XML解析：查找<t>标签内容
    std::regex tagRegex("<t[^>]*>(.*?)</t>");
    std::smatch match;
    std::string::const_iterator searchStart(content.cbegin());

    while (std::regex_search(searchStart, content.cend(), match, tagRegex)) {
        std::string text = match[1];
        // 解码XML实体
        text = decodeXmlEntities(text);
        sharedStrings.push_back(text);
        searchStart = match.suffix().first;
    }

    return sharedStrings;
}

std::string ExcelReader::findWorksheet(const std::string& tempDir, const std::string& sheetName)
{
    // 如果没有指定工作表名，使用第一个工作表
    if (sheetName.empty()) {
        std::string defaultSheet = tempDir + "/xl/worksheets/sheet1.xml";
        if (fs::exists(defaultSheet)) {
            return defaultSheet;
        }
    }

    // 读取workbook.xml查找工作表信息
    std::string workbookPath = tempDir + "/xl/workbook.xml";
    if (!fs::exists(workbookPath)) {
        return "";
    }

    std::ifstream file(workbookPath);
    if (!file.is_open()) {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 查找工作表名称和ID的映射
    std::regex sheetRegex("<sheet[^>]*name=\"([^\"]*)\"|rId=\"([^\"]*)\"[^>]*>");
    std::smatch match;
    std::string::const_iterator searchStart(content.cbegin());

    int sheetIndex = 1;
    while (std::regex_search(searchStart, content.cend(), match, sheetRegex)) {
        if (sheetName.empty() || match[1] == sheetName) {
            // 返回对应的工作表文件路径
            std::string worksheetPath =
                tempDir + "/xl/worksheets/sheet" + std::to_string(sheetIndex) + ".xml";
            if (fs::exists(worksheetPath)) {
                return worksheetPath;
            }
        }
        sheetIndex++;
        searchStart = match.suffix().first;
    }

    return "";
}

std::vector<std::vector<std::string>> ExcelReader::readWorksheetData(
    const std::string& worksheetPath, const std::vector<std::string>& sharedStrings)
{
    std::vector<std::vector<std::string>> data;

    std::ifstream file(worksheetPath);
    if (!file.is_open()) {
        return data;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 解析行数据
    std::regex rowRegex("<row[^>]*>(.*?)</row>");
    std::smatch rowMatch;
    std::string::const_iterator rowSearchStart(content.cbegin());

    while (std::regex_search(rowSearchStart, content.cend(), rowMatch, rowRegex)) {
        std::string rowContent = rowMatch[1];
        std::vector<std::string> rowData = parseCellsInRow(rowContent, sharedStrings);

        if (!rowData.empty()) {
            data.push_back(rowData);
        }

        rowSearchStart = rowMatch.suffix().first;
    }

    return data;
}

std::vector<std::string> ExcelReader::parseCellsInRow(const std::string& rowContent,
                                                      const std::vector<std::string>& sharedStrings)
{
    std::vector<std::string> rowData;

    // 解析单元格
    std::regex cellRegex("<c[^>]*>(.*?)</c>");
    std::smatch cellMatch;
    std::string::const_iterator cellSearchStart(rowContent.cbegin());

    while (std::regex_search(cellSearchStart, rowContent.cend(), cellMatch, cellRegex)) {
        std::string cellContent = cellMatch[0];  // 完整的cell标签
        std::string cellValue = extractCellValue(cellContent, sharedStrings);
        rowData.push_back(cellValue);
        cellSearchStart = cellMatch.suffix().first;
    }

    return rowData;
}

std::string ExcelReader::extractCellValue(const std::string& cellXml,
                                          const std::vector<std::string>& sharedStrings)
{
    // 检查单元格类型
    bool isSharedString = cellXml.find("t=\"s\"") != std::string::npos;

    // 提取值
    std::regex valueRegex("<v>(.*?)</v>");
    std::smatch valueMatch;

    if (std::regex_search(cellXml, valueMatch, valueRegex)) {
        std::string value = valueMatch[1];

        if (isSharedString) {
            // 共享字符串引用
            try {
                int index = std::stoi(value);
                if (index >= 0 && index < static_cast<int>(sharedStrings.size())) {
                    return sharedStrings[index];
                }
            }
            catch (...) {
                // 解析失败，返回原始值
            }
        }

        return value;
    }

    return "";
}

DataSet ExcelReader::processWorksheetData(const std::vector<std::vector<std::string>>& rawData,
                                          bool hasHeader)
{
    DataSet dataSet;

    if (rawData.empty()) {
        return dataSet;
    }

    // 确定数据开始行
    size_t dataStartRow = hasHeader ? 1 : 0;
    if (dataStartRow >= rawData.size()) {
        THROW_ERROR(ErrorCode::INVALID_DATA_FORMAT, "No data rows found");
    }

    // 自动检测时间列和数据列
    auto [timeCol, dataCol] = detectTimeAndDataColumns(rawData);

    // 提取数据
    for (size_t row = dataStartRow; row < rawData.size(); ++row) {
        if (timeCol < static_cast<int>(rawData[row].size()) &&
            dataCol < static_cast<int>(rawData[row].size())) {
            double timeValue, dataValue;

            // 解析时间值
            if (tryParseDouble(rawData[row][timeCol], timeValue)) {
                dataSet.timePoints.push_back(timeValue);
            } else {
                // 如果时间列解析失败，使用行索引
                dataSet.timePoints.push_back(static_cast<double>(row - dataStartRow));
            }

            // 解析数据值
            if (tryParseDouble(rawData[row][dataCol], dataValue)) {
                dataSet.dataPoints.push_back(dataValue);
            } else {
                // 跳过无效的数据行
                if (!dataSet.timePoints.empty()) {
                    dataSet.timePoints.pop_back();
                }
            }
        }
    }

    // 如果时间点和数据点数量不匹配，生成默认时间点
    if (dataSet.timePoints.size() != dataSet.dataPoints.size()) {
        dataSet.timePoints.clear();
        for (size_t i = 0; i < dataSet.dataPoints.size(); ++i) {
            dataSet.timePoints.push_back(static_cast<double>(i));
        }
    }

    return dataSet;
}

std::string ExcelReader::decodeXmlEntities(const std::string& text)
{
    std::string result = text;

    // 常见的XML实体解码
    std::regex entityRegex("&(amp|lt|gt|quot|apos);");
    std::smatch match;

    while (std::regex_search(result, match, entityRegex)) {
        std::string entity = match[1];
        std::string replacement;

        if (entity == "amp")
            replacement = "&";
        else if (entity == "lt")
            replacement = "<";
        else if (entity == "gt")
            replacement = ">";
        else if (entity == "quot")
            replacement = "\"";
        else if (entity == "apos")
            replacement = "'";

        result = std::regex_replace(result, std::regex("&" + entity + ";"), replacement);
    }

    return result;
}

void ExcelReader::cleanupTempDir(const std::string& tempDir)
{
    try {
        if (fs::exists(tempDir)) {
            fs::remove_all(tempDir);
        }
    }
    catch (...) {
        // 忽略清理错误
    }
}

std::vector<std::string> ExcelReader::getSheetNames(const std::string& filename)
{
    fs::path filePath(filename);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".csv") {
        return {"Sheet1"};
    } else if (ext == ".xlsx") {
        return getXlsxSheetNames(filename);
    }

    return {};
}

std::vector<std::string> ExcelReader::getXlsxSheetNames(const std::string& filename)
{
    std::vector<std::string> sheetNames;

    try {
        std::string tempDir = extractXlsxToTemp(filename);

        // 读取workbook.xml
        std::string workbookPath = tempDir + "/xl/workbook.xml";
        if (fs::exists(workbookPath)) {
            std::ifstream file(workbookPath);
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                file.close();

                // 提取工作表名称
                std::regex sheetRegex("<sheet[^>]*name=\"([^\"]*)\"|[^>]*>");
                std::smatch match;
                std::string::const_iterator searchStart(content.cbegin());

                while (std::regex_search(searchStart, content.cend(), match, sheetRegex)) {
                    if (match[1].matched) {
                        sheetNames.push_back(match[1]);
                    }
                    searchStart = match.suffix().first;
                }
            }
        }

        cleanupTempDir(tempDir);
    }
    catch (...) {
        // 如果解析失败，返回默认名称
        sheetNames.push_back("Sheet1");
    }

    return sheetNames;
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

    if (ext == ".csv") {
        return previewCsvData(filename, maxRows);
    } else if (ext == ".xlsx") {
        return previewXlsxData(filename, sheetName, maxRows);
    }

    return preview;
}

std::vector<std::vector<std::string>> ExcelReader::previewCsvData(const std::string& filename,
                                                                  int maxRows)
{
    std::vector<std::vector<std::string>> preview;

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

std::vector<std::vector<std::string>> ExcelReader::previewXlsxData(const std::string& filename,
                                                                   const std::string& sheetName,
                                                                   int maxRows)
{
    try {
        std::string tempDir = extractXlsxToTemp(filename);
        auto sharedStrings = readSharedStrings(tempDir + "/xl/sharedStrings.xml");
        std::string worksheetPath = findWorksheet(tempDir, sheetName);

        if (!worksheetPath.empty()) {
            auto fullData = readWorksheetData(worksheetPath, sharedStrings);
            cleanupTempDir(tempDir);

            // 限制返回行数
            std::vector<std::vector<std::string>> preview;
            int count = std::min(maxRows, static_cast<int>(fullData.size()));
            for (int i = 0; i < count; ++i) {
                preview.push_back(fullData[i]);
            }
            return preview;
        }

        cleanupTempDir(tempDir);
    }
    catch (...) {
        // 预览失败时返回空数据
    }

    return {};
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