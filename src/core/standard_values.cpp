#include "core/standard_values.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace neumann {

StandardValues::StandardValues() : minSampleSize(std::numeric_limits<int>::max()), maxSampleSize(0)
{
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

StandardValues &StandardValues::getInstance()
{
    static StandardValues instance;
    return instance;
}

bool StandardValues::loadFromFile(const std::string &filename)
{
    try {
        // 检查文件是否存在
        std::ifstream checkFile(filename);
        if (!checkFile.is_open()) {
            std::cerr << "无法打开标准值文件: " << filename << std::endl;

            // 尝试创建标准值文件
            try {
                std::cout << "尝试创建标准值文件..." << std::endl;
                std::ofstream outFile(filename);
                if (outFile.is_open()) {
                    // 创建基本的标准值JSON
                    json basicValues;

                    // 添加0.95置信水平的默认值
                    json level95;
                    level95["4"] = 0.7805;
                    level95["5"] = 0.8204;
                    level95["6"] = 0.8902;
                    level95["7"] = 0.9359;

                    basicValues["0.95"] = level95;

                    // 写入文件
                    outFile << basicValues.dump(2);  // 缩进为2空格
                    outFile.close();

                    std::cout << "已创建基本标准值文件: " << filename << std::endl;
                } else {
                    std::cerr << "无法创建标准值文件。将使用内置默认值。" << std::endl;
                    return false;
                }
            }
            catch (const std::exception &e) {
                std::cerr << "创建标准值文件失败: " << e.what() << "。将使用内置默认值。"
                          << std::endl;
                return false;
            }

            // 重新尝试打开文件
            checkFile = std::ifstream(filename);
            if (!checkFile.is_open()) {
                std::cerr << "仍然无法打开标准值文件。将使用内置默认值。" << std::endl;
                return false;
            }
        }

        // 打开并读取JSON文件
        std::ifstream file(filename);
        json data;

        try {
            file >> data;
        }
        catch (const json::parse_error &e) {
            std::cerr << "标准值文件JSON解析错误: " << e.what() << std::endl;
            std::cerr << "将使用内置默认值。" << std::endl;
            return false;
        }

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

        std::cout << "成功加载标准值文件: " << filename << std::endl;
        std::cout << "  支持的置信水平: " << confidenceLevels.size() << " 个" << std::endl;
        std::cout << "  样本大小范围: " << minSampleSize << "-" << maxSampleSize << std::endl;

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "加载标准值文件时出错: " << e.what() << std::endl;
        std::cerr << "将使用内置默认值。" << std::endl;
        return false;
    }
}

double StandardValues::getWPValue(int sampleSize, double confidenceLevel) const
{
    // 检查置信水平是否存在
    auto levelIt = wpValues.find(confidenceLevel);
    if (levelIt == wpValues.end()) {
        // 尝试找到最接近的置信水平
        double closestLevel = 0.95;  // 默认
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
            return -1.0;  // 找不到任何有效的置信水平
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

int StandardValues::getMinSampleSize() const
{
    return minSampleSize;
}

int StandardValues::getMaxSampleSize() const
{
    return maxSampleSize;
}

std::vector<double> StandardValues::getSupportedConfidenceLevels() const
{
    return confidenceLevels;
}

}  // namespace neumann