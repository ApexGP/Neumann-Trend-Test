#pragma once

#include <map>
#include <string>
#include <vector>

#include "data_manager.h"

namespace neumann {

/**
 * @brief Excel文件读取器类
 * 
 * 提供读取Excel文件(.xlsx)的功能，支持多种数据格式
 */
class ExcelReader
{
public:
    /**
     * @brief 构造函数
     */
    ExcelReader();

    /**
     * @brief 析构函数
     */
    ~ExcelReader();

    /**
     * @brief 检查文件是否为Excel格式
     * @param filename 文件路径
     * @return 是否为Excel文件
     */
    static bool isExcelFile(const std::string& filename);

    /**
     * @brief 从Excel文件导入数据
     * @param filename Excel文件路径
     * @param sheetName 工作表名称（默认为第一个工作表）
     * @param hasHeader 是否包含表头
     * @return 导入的数据集
     */
    DataSet importFromExcel(const std::string& filename, const std::string& sheetName = "",
                            bool hasHeader = true);

    /**
     * @brief 获取Excel文件中所有工作表名称
     * @param filename Excel文件路径
     * @return 工作表名称列表
     */
    std::vector<std::string> getSheetNames(const std::string& filename);

    /**
     * @brief 预览Excel文件数据
     * @param filename Excel文件路径
     * @param sheetName 工作表名称
     * @param maxRows 最大预览行数
     * @return 预览数据（行列格式）
     */
    std::vector<std::vector<std::string>> previewExcelData(const std::string& filename,
                                                           const std::string& sheetName = "",
                                                           int maxRows = 10);

    /**
     * @brief 检测数据列类型
     * @param data 数据
     * @return 列类型映射（列索引 -> 类型名称）
     */
    std::map<int, std::string> detectColumnTypes(const std::vector<std::vector<std::string>>& data);

    /**
     * @brief 验证数据完整性
     * @param dataSet 数据集
     * @return 验证结果和错误信息
     */
    std::pair<bool, std::string> validateData(const DataSet& dataSet);

private:
    // 清理和验证数据
    std::vector<double> cleanNumericData(const std::vector<std::string>& rawData);

    // 自动检测时间列和数据列
    std::pair<int, int> detectTimeAndDataColumns(const std::vector<std::vector<std::string>>& data);

    // 转换字符串到数字
    bool tryParseDouble(const std::string& str, double& result);
};

}  // namespace neumann