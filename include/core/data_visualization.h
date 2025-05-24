#pragma once

#include <string>
#include <vector>

#include "neumann_calculator.h"

namespace neumann {

/**
 * @brief 数据可视化类
 * 
 * 提供趋势测试结果的图表生成和可视化功能
 */
class DataVisualization
{
public:
    /**
     * @brief 生成趋势图表数据
     * @param results 诺依曼测试结果
     * @return SVG格式的图表字符串
     */
    static std::string generateTrendChart(const NeumannTestResults& results);

    /**
     * @brief 生成PG值分布图
     * @param results 诺依曼测试结果
     * @return SVG格式的图表字符串
     */
    static std::string generatePGDistributionChart(const NeumannTestResults& results);

    /**
     * @brief 生成阈值比较图
     * @param results 诺依曼测试结果
     * @return SVG格式的图表字符串
     */
    static std::string generateThresholdComparisonChart(const NeumannTestResults& results);

    /**
     * @brief 保存图表到文件
     * @param chartSVG SVG格式的图表字符串
     * @param filename 输出文件名
     * @return 是否成功保存
     */
    static bool saveChartToFile(const std::string& chartSVG, const std::string& filename);

    /**
     * @brief 生成ASCII艺术图表（用于CLI显示）
     * @param results 诺依曼测试结果
     * @return ASCII图表字符串
     */
    static std::string generateASCIIChart(const NeumannTestResults& results);

private:
    /**
     * @brief 计算图表坐标
     */
    static std::pair<double, double> calculateChartBounds(const std::vector<double>& values);

    /**
     * @brief 生成SVG路径字符串
     */
    static std::string generateSVGPath(const std::vector<std::pair<double, double>>& points,
                                       double width, double height);
};

}  // namespace neumann