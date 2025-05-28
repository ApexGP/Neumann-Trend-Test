#include "core/data_visualization.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "core/i18n.h"

namespace neumann {

std::string DataVisualization::generateTrendChart(const NeumannTestResults& results)
{
    if (results.results.empty()) {
        return "<svg><text x='10' y='20'>" + _("chart.no_data") + "</text></svg>";
    }

    const int width = 1200;  // 放大图表宽度
    const int height = 600;  // 放大图表高度
    const int margin = 80;   // 增加边距以容纳轴标签

    std::ostringstream svg;
    svg << "<svg width='" << width << "' height='" << height
        << "' xmlns='http://www.w3.org/2000/svg' style='display: block; margin: 0 auto;'>\n";

    // 添加样式定义
    svg << "<style>\n";
    svg << "  .chart-title { font-family: Arial, sans-serif; font-size: 20px; font-weight: bold; "
           "}\n";
    svg << "  .axis-label { font-family: Arial, sans-serif; font-size: 12px; }\n";
    svg << "  .axis-title { font-family: Arial, sans-serif; font-size: 14px; font-weight: bold; "
           "}\n";
    svg << "  .legend-text { font-family: Arial, sans-serif; font-size: 14px; }\n";
    svg << "</style>\n";

    // 背景
    svg << "<rect width='" << width << "' height='" << height << "' fill='white' stroke='#ddd'/>\n";

    // 标题
    svg << "<text x='" << width / 2 << "' y='30' text-anchor='middle' class='chart-title'>"
        << _("chart.title") << "</text>\n";

    // 计算坐标范围
    std::vector<double> pgValues;
    std::vector<double> thresholds;
    for (const auto& result : results.results) {
        pgValues.push_back(result.pgValue);
        thresholds.push_back(result.wpThreshold);
    }

    auto [minY, maxY] = calculateChartBounds(pgValues);
    double minThreshold = *std::min_element(thresholds.begin(), thresholds.end());
    minY = std::min(minY, minThreshold);
    maxY = std::max(maxY, *std::max_element(thresholds.begin(), thresholds.end()));

    // 添加一些边距
    double range = maxY - minY;
    minY -= range * 0.1;
    maxY += range * 0.1;

    // 绘制坐标轴
    int chartLeft = margin;
    int chartRight = width - margin;
    int chartTop = margin + 30;
    int chartBottom = height - margin;

    // Y轴
    svg << "<line x1='" << chartLeft << "' y1='" << chartTop << "' x2='" << chartLeft << "' y2='"
        << chartBottom << "' stroke='black'/>\n";

    // X轴
    svg << "<line x1='" << chartLeft << "' y1='" << chartBottom << "' x2='" << chartRight
        << "' y2='" << chartBottom << "' stroke='black'/>\n";

    // 绘制PG值线
    std::vector<std::pair<double, double>> pgPoints;
    for (size_t i = 0; i < results.results.size(); ++i) {
        double x =
            chartLeft + (double(i) / (results.results.size() - 1)) * (chartRight - chartLeft);
        double y = chartBottom -
                   ((results.results[i].pgValue - minY) / (maxY - minY)) * (chartBottom - chartTop);
        pgPoints.push_back({x, y});
    }

    // PG值线条
    svg << "<polyline points='";
    for (const auto& point : pgPoints) {
        svg << point.first << "," << point.second << " ";
    }
    svg << "' fill='none' stroke='blue' stroke-width='2'/>\n";

    // 绘制阈值线
    std::vector<std::pair<double, double>> thresholdPoints;
    for (size_t i = 0; i < results.results.size(); ++i) {
        double x =
            chartLeft + (double(i) / (results.results.size() - 1)) * (chartRight - chartLeft);
        double y = chartBottom - ((results.results[i].wpThreshold - minY) / (maxY - minY)) *
                                     (chartBottom - chartTop);
        thresholdPoints.push_back({x, y});
    }

    // 阈值线条
    svg << "<polyline points='";
    for (const auto& point : thresholdPoints) {
        svg << point.first << "," << point.second << " ";
    }
    svg << "' fill='none' stroke='red' stroke-width='2' stroke-dasharray='5,5'/>\n";

    // 数据点
    for (size_t i = 0; i < pgPoints.size(); ++i) {
        std::string color = results.results[i].hasTrend ? "red" : "green";
        svg << "<circle cx='" << pgPoints[i].first << "' cy='" << pgPoints[i].second
            << "' r='4' fill='" << color << "'/>\n";
    }

    // 图例
    svg << "<text x='" << chartRight - 150 << "' y='" << chartTop + 20 << "' class='legend-text'>"
        << _("chart.pg_values") << "</text>\n";
    svg << "<line x1='" << chartRight - 170 << "' y1='" << chartTop + 16 << "' x2='"
        << chartRight - 155 << "' y2='" << chartTop + 16 << "' stroke='blue' stroke-width='2'/>\n";

    svg << "<text x='" << chartRight - 150 << "' y='" << chartTop + 40 << "' class='legend-text'>"
        << _("chart.thresholds") << "</text>\n";
    svg << "<line x1='" << chartRight - 170 << "' y1='" << chartTop + 36 << "' x2='"
        << chartRight - 155 << "' y2='" << chartTop + 36
        << "' stroke='red' stroke-width='2' stroke-dasharray='5,5'/>\n";

    // Y轴标签
    for (int i = 0; i <= 5; ++i) {
        double value = minY + (maxY - minY) * i / 5.0;
        int y = chartBottom - i * (chartBottom - chartTop) / 5;
        svg << "<text x='" << chartLeft - 10 << "' y='" << y + 5
            << "' text-anchor='end' class='axis-label'>" << std::fixed << std::setprecision(3)
            << value << "</text>\n";
    }

    // X轴标签 - 改进间距算法
    size_t totalPoints = results.results.size();
    int maxLabels = std::min(static_cast<int>(totalPoints), 12);  // 最多显示12个标签
    int labelStep = std::max(1, static_cast<int>(totalPoints) / maxLabels);

    // 确保始终显示第一个和最后一个标签
    for (size_t i = 0; i < totalPoints; ++i) {
        bool shouldShowLabel = (i == 0) || (i == totalPoints - 1) || (i % labelStep == 0);

        if (shouldShowLabel) {
            double x = (totalPoints == 1)
                           ? (chartLeft + chartRight) / 2.0
                           : chartLeft + (double(i) / (totalPoints - 1)) * (chartRight - chartLeft);
            svg << "<text x='" << x << "' y='" << chartBottom + 20
                << "' text-anchor='middle' class='axis-label'>" << (i + 4) << "</text>\n";
        }
    }

    // X轴标题（时间点说明）
    svg << "<text x='" << (chartLeft + chartRight) / 2 << "' y='" << chartBottom + 50
        << "' text-anchor='middle' class='axis-title'>" << _("chart.time_points_unit")
        << "</text>\n";

    // Y轴标题（PG值说明）
    svg << "<text x='" << chartLeft - 50 << "' y='" << (chartTop + chartBottom) / 2
        << "' text-anchor='middle' class='axis-title' transform='rotate(-90, " << chartLeft - 50
        << ", " << (chartTop + chartBottom) / 2 << ")'>" << _("chart.pg_value_label")
        << "</text>\n";

    svg << "</svg>";
    return svg.str();
}

std::string DataVisualization::generatePGDistributionChart(const NeumannTestResults& results)
{
    if (results.results.empty()) {
        return "<svg><text x='10' y='20'>" + _("chart.no_data") + "</text></svg>";
    }

    const int width = 600;
    const int height = 400;
    const int margin = 60;

    std::ostringstream svg;
    svg << "<svg width='" << width << "' height='" << height
        << "' xmlns='http://www.w3.org/2000/svg'>\n";

    // 背景
    svg << "<rect width='" << width << "' height='" << height << "' fill='white' stroke='#ddd'/>\n";

    // 标题
    svg << "<text x='" << width / 2
        << "' y='30' text-anchor='middle' font-size='16' font-weight='bold'>"
        << _("chart.pg_distribution_title") << "</text>\n";

    // 创建直方图
    std::vector<double> pgValues;
    for (const auto& result : results.results) {
        pgValues.push_back(result.pgValue);
    }

    // 计算分布区间
    auto [minPG, maxPG] = calculateChartBounds(pgValues);
    const int bins = std::min(10, static_cast<int>(pgValues.size()));
    double binWidth = (maxPG - minPG) / bins;

    std::vector<int> histogram(bins, 0);
    for (double value : pgValues) {
        int binIndex = std::min(bins - 1, static_cast<int>((value - minPG) / binWidth));
        histogram[binIndex]++;
    }

    int maxCount = *std::max_element(histogram.begin(), histogram.end());

    // 绘制直方图
    int chartLeft = margin;
    int chartRight = width - margin;
    int chartTop = margin + 30;
    int chartBottom = height - margin;

    double barWidth = (chartRight - chartLeft) / static_cast<double>(bins);

    for (int i = 0; i < bins; ++i) {
        double barHeight = histogram[i] * (chartBottom - chartTop) / static_cast<double>(maxCount);
        double x = chartLeft + i * barWidth;
        double y = chartBottom - barHeight;

        svg << "<rect x='" << x << "' y='" << y << "' width='" << barWidth * 0.8 << "' height='"
            << barHeight << "' fill='steelblue' stroke='white'/>\n";

        // 显示计数
        if (histogram[i] > 0) {
            svg << "<text x='" << x + barWidth * 0.4 << "' y='" << y - 5
                << "' text-anchor='middle' font-size='10'>" << histogram[i] << "</text>\n";
        }
    }

    // 坐标轴
    svg << "<line x1='" << chartLeft << "' y1='" << chartBottom << "' x2='" << chartRight
        << "' y2='" << chartBottom << "' stroke='black'/>\n";
    svg << "<line x1='" << chartLeft << "' y1='" << chartTop << "' x2='" << chartLeft << "' y2='"
        << chartBottom << "' stroke='black'/>\n";

    svg << "</svg>";
    return svg.str();
}

std::string DataVisualization::generateThresholdComparisonChart(const NeumannTestResults& results)
{
    // 实现阈值比较图表
    return generateTrendChart(results);  // 临时使用趋势图
}

bool DataVisualization::saveChartToFile(const std::string& chartSVG, const std::string& filename)
{
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        file << chartSVG;
        file.close();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::string DataVisualization::generateASCIIChart(const NeumannTestResults& results)
{
    if (results.results.empty()) {
        return _("chart.no_data");
    }

    std::ostringstream chart;
    chart << "\n";
    chart << "=== " << _("chart.title") << " ===\n\n";

    // 找到PG值的范围
    std::vector<double> pgValues;
    for (const auto& result : results.results) {
        pgValues.push_back(result.pgValue);
    }

    auto [minPG, maxPG] = calculateChartBounds(pgValues);

    // ASCII图表参数 - 确保足够的显示空间
    const int chartWidth = 70;   // 图表数据区域宽度
    const int chartHeight = 15;  // 图表高度
    const int yLabelWidth = 8;   // Y轴标签宽度

    // 创建图表网格
    std::vector<std::vector<char>> grid(chartHeight, std::vector<char>(chartWidth, ' '));

    // 绘制数据点 - 改进分布算法
    for (size_t i = 0; i < results.results.size(); ++i) {
        // 计算X坐标：确保数据点均匀分布在整个图表宽度上
        int x;
        if (results.results.size() == 1) {
            x = chartWidth / 2;
        } else {
            x = (i * (chartWidth - 1)) / (results.results.size() - 1);
        }

        // 计算Y坐标：PG值映射到图表高度
        double normalizedY = (results.results[i].pgValue - minPG) / (maxPG - minPG);
        int y = chartHeight - 1 - (int) (normalizedY * (chartHeight - 1));

        // 确保坐标在有效范围内
        y = std::max(0, std::min(chartHeight - 1, y));
        x = std::max(0, std::min(chartWidth - 1, x));

        grid[y][x] = results.results[i].hasTrend ? '*' : 'o';
    }

    // 输出图表
    chart << "PG Values (o = no trend, * = trend detected)\n";

    // 顶部边界
    chart << std::fixed << std::setprecision(3) << std::setw(7) << std::right << maxPG << " ┌";
    for (int x = 0; x < chartWidth; ++x) {
        chart << "─";
    }
    chart << "┐\n";

    // 绘制图表主体
    for (int y = 0; y < chartHeight; ++y) {
        // Y轴标签
        if (y == 0) {
            chart << "        │";  // 顶部
        } else if (y == chartHeight / 2) {
            double midValue = (minPG + maxPG) / 2;
            chart << std::fixed << std::setprecision(3) << std::setw(7) << std::right << midValue
                  << " │";
        } else if (y == chartHeight - 1) {
            chart << "        │";  // 底部
        } else {
            chart << "        │";  // 中间行
        }

        // 数据区域
        for (int x = 0; x < chartWidth; ++x) {
            chart << grid[y][x];
        }
        chart << "│\n";
    }

    // 底部边界
    chart << std::fixed << std::setprecision(3) << std::setw(7) << std::right << minPG << " └";
    for (int x = 0; x < chartWidth; ++x) {
        chart << "─";
    }
    chart << "┘\n";

    // X轴标记 - 显示数据点编号
    chart << "        ";  // Y轴标签空间
    for (int x = 0; x < chartWidth; ++x) {
        bool hasLabel = false;
        for (size_t i = 0; i < results.results.size(); ++i) {
            int dataX = (results.results.size() == 1)
                            ? chartWidth / 2
                            : (i * (chartWidth - 1)) / (results.results.size() - 1);
            if (dataX == x) {
                chart << (char) ('A' + (i % 26));  // 使用字母标记数据点
                hasLabel = true;
                break;
            }
        }
        if (!hasLabel) {
            if (x == 0 || x == chartWidth - 1) {
                chart << "┼";
            } else if (x % 10 == 0) {
                chart << "┴";
            } else {
                chart << " ";
            }
        }
    }
    chart << "\n";

    // 数据点说明
    chart << "Data Points: ";
    for (size_t i = 0; i < results.results.size(); ++i) {
        chart << (char) ('A' + (i % 26)) << "=" << (i + 4);
        if (i < results.results.size() - 1) chart << ", ";
        if (i > 0 && i % 8 == 0) chart << "\n             ";  // 换行避免过长
    }
    chart << "\n";

    return chart.str();
}

std::pair<double, double> DataVisualization::calculateChartBounds(const std::vector<double>& values)
{
    if (values.empty()) {
        return {0.0, 1.0};
    }

    double minVal = *std::min_element(values.begin(), values.end());
    double maxVal = *std::max_element(values.begin(), values.end());

    // 添加一些边距
    double range = maxVal - minVal;
    if (range < 1e-10) {  // 避免除零
        minVal -= 0.1;
        maxVal += 0.1;
    }

    return {minVal, maxVal};
}

std::string DataVisualization::generateSVGPath(const std::vector<std::pair<double, double>>& points,
                                               double width, double height)
{
    if (points.empty()) {
        return "";
    }

    std::ostringstream path;
    path << "M " << points[0].first << " " << points[0].second;

    for (size_t i = 1; i < points.size(); ++i) {
        path << " L " << points[i].first << " " << points[i].second;
    }

    return path.str();
}

}  // namespace neumann