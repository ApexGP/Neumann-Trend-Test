#pragma once

#include <memory>
#include <string>
#include <vector>

namespace neumann {

/**
 * @brief 用于存储诺依曼趋势测试结果的结构体
 */
struct NeumannResult {
  double pgValue;         // 计算得到的PG值
  bool hasTrend;          // 是否存在趋势
  double confidenceLevel; // 使用的置信水平
  double wpThreshold;     // 决策的阈值
};

/**
 * @brief 诺依曼趋势测试结果集合
 */
struct NeumannTestResults {
  std::vector<double> data;           // 原始数据点
  std::vector<double> timePoints;     // 时间点
  std::vector<NeumannResult> results; // 每个测试点的结果

  // 汇总统计信息
  bool overallTrend; // 整体是否存在趋势
  double minPG;      // 最小PG值
  double maxPG;      // 最大PG值
  double avgPG;      // 平均PG值
};

/**
 * @brief 诺依曼趋势测试计算器类
 *
 * 提供执行诺依曼趋势测试计算的功能
 */
class NeumannCalculator {
public:
  /**
   * @brief 构造函数
   * @param confidenceLevel 使用的置信水平 (默认0.95)
   */
  explicit NeumannCalculator(double confidenceLevel = 0.95);

  /**
   * @brief 使用默认的时间点执行诺依曼趋势测试
   * @param data 测量数据点
   * @return 诺依曼测试结果
   */
  NeumannTestResults performTest(const std::vector<double> &data);

  /**
   * @brief 使用自定义时间点执行诺依曼趋势测试
   * @param data 测量数据点
   * @param timePoints 对应的时间点
   * @return 诺依曼测试结果
   */
  NeumannTestResults performTest(const std::vector<double> &data,
                                 const std::vector<double> &timePoints);

  /**
   * @brief 设置置信水平
   * @param level 新的置信水平
   */
  void setConfidenceLevel(double level);

  /**
   * @brief 获取当前置信水平
   * @return 当前置信水平
   */
  double getConfidenceLevel() const;

private:
  /**
   * @brief 计算诺依曼趋势统计量
   * @param data 数据点向量
   * @param endIndex 结束索引（包含）
   * @return PG值
   */
  double calculatePG(const std::vector<double> &data, size_t endIndex);

  /**
   * @brief 判断是否存在趋势
   * @param pgValue 计算的PG值
   * @param sampleSize 样本数量
   * @return 是否存在趋势
   */
  bool determineTrend(double pgValue, int sampleSize);

  // 当前使用的置信水平
  double confidenceLevel;
};

} // namespace neumann