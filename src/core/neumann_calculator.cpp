#include "core/neumann_calculator.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "core/standard_values.h"

namespace neumann {

NeumannCalculator::NeumannCalculator(double confidenceLevel) : confidenceLevel(confidenceLevel) {}

NeumannTestResults NeumannCalculator::performTest(const std::vector<double> &data)
{
    // 创建默认时间点 (0, 1, 2, ...)
    std::vector<double> timePoints(data.size());
    std::iota(timePoints.begin(), timePoints.end(), 0.0);

    return performTest(data, timePoints);
}

NeumannTestResults NeumannCalculator::performTest(const std::vector<double> &data,
                                                  const std::vector<double> &timePoints)
{
    NeumannTestResults results;
    results.data = data;
    results.timePoints = timePoints;

    // 最少需要4个数据点才能进行测试
    if (data.size() < 4 || data.size() != timePoints.size()) {
        return results;
    }

    double sumPG = 0.0;
    double minPG = std::numeric_limits<double>::max();
    double maxPG = std::numeric_limits<double>::lowest();
    bool hasTrend = false;

    // 对每个可能的子集计算PG值和判断是否有趋势
    for (size_t i = 3; i < data.size(); ++i) {
        double pgValue = calculatePG(data, i);
        bool trend = determineTrend(pgValue, i + 1);

        // 更新统计信息
        sumPG += pgValue;
        minPG = std::min(minPG, pgValue);
        maxPG = std::max(maxPG, pgValue);

        // 如果有一个测试点判断存在趋势，整体就视为存在趋势
        if (trend) {
            hasTrend = true;
        }

        // 获取对应样本数量的标准阈值
        double wpThreshold = StandardValues::getInstance().getWPValue(i + 1, confidenceLevel);

        // 添加到结果中
        NeumannResult result;
        result.pgValue = pgValue;
        result.hasTrend = trend;
        result.confidenceLevel = confidenceLevel;
        result.wpThreshold = wpThreshold;

        results.results.push_back(result);
    }

    // 更新汇总统计信息
    results.overallTrend = hasTrend;
    results.minPG = minPG;
    results.maxPG = maxPG;
    results.avgPG = sumPG / results.results.size();

    return results;
}

void NeumannCalculator::setConfidenceLevel(double level)
{
    confidenceLevel = level;
}

double NeumannCalculator::getConfidenceLevel() const
{
    return confidenceLevel;
}

double NeumannCalculator::calculatePG(const std::vector<double> &data, size_t endIndex)
{
    // 确保不会超出范围
    if (endIndex >= data.size() || endIndex < 3) {
        return 0.0;
    }

    double sum_area = 0.0;
    double sum_numer = 0.0;
    double sum_denomin = 0.0;

    // 计算数据子集的和和平均值
    for (size_t j = 0; j <= endIndex; ++j) {
        sum_area += data[j];
    }
    double avg = sum_area / (endIndex + 1);

    // 计算分子和分母
    for (size_t k = 0; k <= endIndex; ++k) {
        if (k < endIndex) {
            sum_numer += std::pow(data[k] - data[k + 1], 2);
        }
        sum_denomin += std::pow(data[k] - avg, 2);
    }

    // 防止除以零
    if (sum_denomin == 0.0) {
        return 0.0;
    }

    // 计算PG值
    return sum_numer / sum_denomin;
}

bool NeumannCalculator::determineTrend(double pgValue, int sampleSize)
{
    // 获取对应的标准W(P)值
    double wpValue = StandardValues::getInstance().getWPValue(sampleSize, confidenceLevel);

    // 如果PG <= WP，则判断为存在趋势
    return (pgValue <= wpValue);
}

}  // namespace neumann