#include "core/standard_values.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace neumann {

StandardValues::StandardValues()
    : minSampleSize(std::numeric_limits<int>::max()), maxSampleSize(0) {
  // 添加标准值的基本数据（如果未找到JSON文件，则使用这些）
  // 这些数据来自Neumann Trend Test标准表，置信水平0.95
  std::map<int, double> values95;
  values95[4] = 0.7805;
  values95[5] = 0.8204;
  values95[6] = 0.8902;
  values95[7] = 0.9359;
  wpValues[0.95] = values95;

  // 更新缓存值
  minSampleSize = 4;
  maxSampleSize = 7;
  confidenceLevels.push_back(0.95);
}

StandardValues &StandardValues::getInstance() {
  static StandardValues instance;
  return instance;
}

bool StandardValues::loadFromFile(const std::string &filename) {
  try {
    // 打开并读取JSON文件
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "无法打开标准值文件: " << filename << std::endl;
      return false;
    }

    json data;
    file >> data;

    // 清除现有数据
    wpValues.clear();
    confidenceLevels.clear();

    minSampleSize = std::numeric_limits<int>::max();
    maxSampleSize = 0;

    // 解析JSON数据
    for (auto &confidenceLevel : data.items()) {
      double level = std::stod(confidenceLevel.key());
      confidenceLevels.push_back(level);

      std::map<int, double> levelValues;
      for (auto &sampleSize : confidenceLevel.value().items()) {
        int size = std::stoi(sampleSize.key());
        double value = sampleSize.value().get<double>();

        levelValues[size] = value;

        // 更新最小/最大样本大小
        minSampleSize = std::min(minSampleSize, size);
        maxSampleSize = std::max(maxSampleSize, size);
      }

      wpValues[level] = levelValues;
    }

    // 排序置信水平
    std::sort(confidenceLevels.begin(), confidenceLevels.end());

    return true;
  } catch (const std::exception &e) {
    std::cerr << "加载标准值文件时出错: " << e.what() << std::endl;
    return false;
  }
}

double StandardValues::getWPValue(int sampleSize,
                                  double confidenceLevel) const {
  // 检查置信水平是否存在
  auto levelIt = wpValues.find(confidenceLevel);
  if (levelIt == wpValues.end()) {
    // 尝试找到最接近的置信水平
    double closestLevel = 0.95; // 默认
    double minDiff = std::numeric_limits<double>::max();

    for (double level : confidenceLevels) {
      double diff = std::abs(level - confidenceLevel);
      if (diff < minDiff) {
        minDiff = diff;
        closestLevel = level;
      }
    }

    levelIt = wpValues.find(closestLevel);
    if (levelIt == wpValues.end()) {
      return -1.0; // 找不到任何有效的置信水平
    }
  }

  // 检查样本大小是否存在
  auto sizeIt = levelIt->second.find(sampleSize);
  if (sizeIt == levelIt->second.end()) {
    // 找不到精确匹配，返回-1.0
    return -1.0;
  }

  return sizeIt->second;
}

int StandardValues::getMinSampleSize() const { return minSampleSize; }

int StandardValues::getMaxSampleSize() const { return maxSampleSize; }

std::vector<double> StandardValues::getSupportedConfidenceLevels() const {
  return confidenceLevels;
}

} // namespace neumann