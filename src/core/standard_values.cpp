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
    // 添加标准值的完整数据（如果未找到JSON文件，则使用这些）
    // 这些数据来自Neumann Trend Test标准表

    // 0.90置信水平
    std::map<int, double> values90;
    values90[4] = 0.6675;
    values90[5] = 0.7131;
    values90[6] = 0.7835;
    values90[7] = 0.8371;
    values90[8] = 0.8796;
    values90[9] = 0.9178;
    values90[10] = 0.9318;
    values90[11] = 0.9394;
    values90[12] = 0.9471;
    values90[13] = 0.9548;
    values90[14] = 0.9625;
    values90[15] = 0.9677;
    values90[16] = 0.9705;
    values90[17] = 0.9734;
    values90[18] = 0.9763;
    values90[19] = 0.9791;
    values90[20] = 0.982;
    wpValues[0.90] = values90;

    // 0.95置信水平
    std::map<int, double> values95;
    values95[4] = 0.7805;
    values95[5] = 0.8204;
    values95[6] = 0.8902;
    values95[7] = 0.9359;
    values95[8] = 0.9483;
    values95[9] = 0.9608;
    values95[10] = 0.9685;
    values95[11] = 0.9733;
    values95[12] = 0.9782;
    values95[13] = 0.983;
    values95[14] = 0.9879;
    values95[15] = 0.9903;
    values95[16] = 0.9915;
    values95[17] = 0.9928;
    values95[18] = 0.994;
    values95[19] = 0.9953;
    values95[20] = 0.9965;
    wpValues[0.95] = values95;

    // 0.975置信水平
    std::map<int, double> values975;
    values975[4] = 0.8423;
    values975[5] = 0.8821;
    values975[6] = 0.9185;
    values975[7] = 0.9534;
    values975[8] = 0.9635;
    values975[9] = 0.9695;
    values975[10] = 0.9756;
    values975[11] = 0.9784;
    values975[12] = 0.9813;
    values975[13] = 0.9841;
    values975[14] = 0.987;
    values975[15] = 0.9886;
    values975[16] = 0.9904;
    values975[17] = 0.9923;
    values975[18] = 0.9941;
    values975[19] = 0.9959;
    values975[20] = 0.9978;
    wpValues[0.975] = values975;

    // 0.99置信水平
    std::map<int, double> values99;
    values99[4] = 0.9027;
    values99[5] = 0.9282;
    values99[6] = 0.9525;
    values99[7] = 0.9721;
    values99[8] = 0.9757;
    values99[9] = 0.9794;
    values99[10] = 0.9831;
    values99[11] = 0.9856;
    values99[12] = 0.9881;
    values99[13] = 0.9905;
    values99[14] = 0.993;
    values99[15] = 0.994;
    values99[16] = 0.995;
    values99[17] = 0.996;
    values99[18] = 0.997;
    values99[19] = 0.998;
    values99[20] = 0.999;
    wpValues[0.99] = values99;

    // 更新缓存值
    minSampleSize = 4;
    maxSampleSize = 20;
    confidenceLevels = {0.90, 0.95, 0.975, 0.99};
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
                    // 创建完整的标准值JSON - 基于Neumann Trend Test标准表
                    json completeValues;

                    // 0.90置信水平
                    json level90;
                    level90["4"] = 0.6675;
                    level90["5"] = 0.7131;
                    level90["6"] = 0.7835;
                    level90["7"] = 0.8371;
                    level90["8"] = 0.8796;
                    level90["9"] = 0.9178;
                    level90["10"] = 0.9318;
                    level90["11"] = 0.9394;
                    level90["12"] = 0.9471;
                    level90["13"] = 0.9548;
                    level90["14"] = 0.9625;
                    level90["15"] = 0.9677;
                    level90["16"] = 0.9705;
                    level90["17"] = 0.9734;
                    level90["18"] = 0.9763;
                    level90["19"] = 0.9791;
                    level90["20"] = 0.982;
                    completeValues["0.90"] = level90;

                    // 0.95置信水平
                    json level95;
                    level95["4"] = 0.7805;
                    level95["5"] = 0.8204;
                    level95["6"] = 0.8902;
                    level95["7"] = 0.9359;
                    level95["8"] = 0.9483;
                    level95["9"] = 0.9608;
                    level95["10"] = 0.9685;
                    level95["11"] = 0.9733;
                    level95["12"] = 0.9782;
                    level95["13"] = 0.983;
                    level95["14"] = 0.9879;
                    level95["15"] = 0.9903;
                    level95["16"] = 0.9915;
                    level95["17"] = 0.9928;
                    level95["18"] = 0.994;
                    level95["19"] = 0.9953;
                    level95["20"] = 0.9965;
                    completeValues["0.95"] = level95;

                    // 0.975置信水平
                    json level975;
                    level975["4"] = 0.8423;
                    level975["5"] = 0.8821;
                    level975["6"] = 0.9185;
                    level975["7"] = 0.9534;
                    level975["8"] = 0.9635;
                    level975["9"] = 0.9695;
                    level975["10"] = 0.9756;
                    level975["11"] = 0.9784;
                    level975["12"] = 0.9813;
                    level975["13"] = 0.9841;
                    level975["14"] = 0.987;
                    level975["15"] = 0.9886;
                    level975["16"] = 0.9904;
                    level975["17"] = 0.9923;
                    level975["18"] = 0.9941;
                    level975["19"] = 0.9959;
                    level975["20"] = 0.9978;
                    completeValues["0.975"] = level975;

                    // 0.99置信水平
                    json level99;
                    level99["4"] = 0.9027;
                    level99["5"] = 0.9282;
                    level99["6"] = 0.9525;
                    level99["7"] = 0.9721;
                    level99["8"] = 0.9757;
                    level99["9"] = 0.9794;
                    level99["10"] = 0.9831;
                    level99["11"] = 0.9856;
                    level99["12"] = 0.9881;
                    level99["13"] = 0.9905;
                    level99["14"] = 0.993;
                    level99["15"] = 0.994;
                    level99["16"] = 0.995;
                    level99["17"] = 0.996;
                    level99["18"] = 0.997;
                    level99["19"] = 0.998;
                    level99["20"] = 0.999;
                    completeValues["0.99"] = level99;

                    // 写入文件
                    outFile << completeValues.dump(2);  // 缩进为2空格
                    outFile.close();

                    std::cout << "已创建完整标准值文件: " << filename << std::endl;
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