#include "core/standard_values.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <sstream>

#include "core/i18n.h"

using json = nlohmann::json;

namespace neumann {

// 简单的字符串替换函数
std::string replace_placeholder(const std::string &text, const std::string &placeholder,
                                const std::string &replacement)
{
    std::string result = text;
    size_t pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.length(), replacement);
    }
    return result;
}

template <typename T>
std::string format_single(const std::string &format, const T &value)
{
    std::ostringstream oss;
    oss << value;
    return replace_placeholder(format, "{}", oss.str());
}

template <typename T1, typename T2>
std::string format_two(const std::string &format, const T1 &value1, const T2 &value2)
{
    std::ostringstream oss1, oss2;
    oss1 << value1;
    oss2 << value2;
    std::string result = format;
    // 替换第一个 {}
    size_t pos = result.find("{}");
    if (pos != std::string::npos) {
        result.replace(pos, 2, oss1.str());
    }
    // 替换第二个 {}
    pos = result.find("{}");
    if (pos != std::string::npos) {
        result.replace(pos, 2, oss2.str());
    }
    return result;
}

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
            std::cerr << _("standard_values.file_open_error") << ": " << filename << std::endl;

            // 尝试创建标准值文件
            try {
                std::cout << _("standard_values.create_file_attempt") << std::endl;
                std::ofstream outFile(filename);
                if (outFile.is_open()) {
                    // 创建完整的标准值JSON - 基于Neumann Trend Test标准表
                    json completeValues;

                    // 定义标准表数据 - 按样本数排序
                    std::vector<std::pair<int, std::vector<double>>> standardTableData = {
                        {4, {0.7805, 0.6256, 0.5898}},  {5, {0.8204, 0.5779, 0.4161}},
                        {6, {0.8902, 0.5615, 0.3634}},  {7, {0.9359, 0.6140, 0.3695}},
                        {8, {0.9825, 0.6628, 0.4036}},  {9, {1.0244, 0.7088, 0.4420}},
                        {10, {1.0623, 0.7518, 0.4816}}, {11, {1.0965, 0.7915, 0.5197}},
                        {12, {1.1276, 0.8280, 0.5557}}, {13, {1.1558, 0.8618, 0.5898}},
                        {14, {1.1816, 0.8931, 0.6223}}, {15, {1.2053, 0.9221, 0.6532}},
                        {16, {1.2272, 0.9491, 0.6826}}, {17, {1.2473, 0.9743, 0.7104}},
                        {18, {1.2660, 0.9979, 0.7368}}, {19, {1.2834, 1.0199, 0.7617}},
                        {20, {1.2996, 1.0406, 0.7852}}, {21, {1.3148, 1.0601, 0.8073}},
                        {22, {1.3290, 1.0785, 0.8283}}, {23, {1.3425, 1.0958, 0.8481}},
                        {24, {1.3552, 1.1122, 0.8668}}, {25, {1.3671, 1.1278, 0.8846}},
                        {26, {1.3785, 1.1426, 0.9017}}, {27, {1.3892, 1.1567, 0.9182}},
                        {28, {1.3994, 1.1702, 0.9341}}, {29, {1.4091, 1.1830, 0.9496}},
                        {30, {1.4183, 1.1951, 0.9645}}, {31, {1.4270, 1.2067, 0.9789}},
                        {32, {1.4354, 1.2177, 0.9925}}, {33, {1.4434, 1.2283, 1.0055}},
                        {34, {1.4511, 1.2386, 1.0180}}, {35, {1.4585, 1.2485, 1.0300}},
                        {36, {1.4656, 1.2581, 1.0416}}, {37, {1.4726, 1.2673, 1.0529}},
                        {38, {1.4793, 1.2763, 1.0639}}, {39, {1.4858, 1.2850, 1.0746}},
                        {40, {1.4921, 1.2934, 1.0850}}, {41, {1.4982, 1.3017, 1.0950}},
                        {42, {1.5041, 1.3096, 1.1048}}, {43, {1.5098, 1.3172, 1.1142}},
                        {44, {1.5154, 1.3246, 1.1233}}, {45, {1.5206, 1.3317, 1.1320}},
                        {46, {1.5257, 1.3387, 1.1404}}, {47, {1.5305, 1.3453, 1.1484}},
                        {48, {1.5351, 1.3515, 1.1561}}, {49, {1.5395, 1.3573, 1.1635}},
                        {50, {1.5437, 1.3629, 1.1705}}, {51, {1.5477, 1.3683, 1.1774}},
                        {52, {1.5518, 1.3738, 1.1843}}, {53, {1.5557, 1.3792, 1.1910}},
                        {54, {1.5596, 1.3846, 1.1976}}, {55, {1.5634, 1.3899, 1.2041}},
                        {56, {1.5670, 1.3949, 1.2104}}, {57, {1.5707, 1.3999, 1.2166}},
                        {58, {1.5743, 1.4048, 1.2227}}, {59, {1.5779, 1.4096, 1.2288}},
                        {60, {1.5814, 1.4144, 1.2349}}};

                    // 置信度数组 [P(95%), P(99%), P(99.9%)]
                    std::vector<std::string> confidenceLevels = {"0.95", "0.99", "0.999"};

                    // 按置信度创建JSON对象 - 保证顺序
                    for (size_t confIdx = 0; confIdx < confidenceLevels.size(); ++confIdx) {
                        json levelData;

                        // 按样本数排序添加数据
                        for (const auto &entry : standardTableData) {
                            int sampleSize = entry.first;
                            double value = entry.second[confIdx];
                            levelData[std::to_string(sampleSize)] = value;
                        }

                        completeValues[confidenceLevels[confIdx]] = levelData;
                    }

                    // 写入文件
                    outFile << completeValues.dump(2);  // 缩进为2空格
                    outFile.close();

                    std::cout << _("standard_values.create_file_success") << ": " << filename
                              << std::endl;
                } else {
                    std::cerr << _("standard_values.file_create_error") << std::endl;
                    return false;
                }
            }
            catch (const std::exception &e) {
                std::cerr << _("standard_values.file_create_failed") << ": " << e.what()
                          << std::endl;
                return false;
            }

            // 重新尝试打开文件
            checkFile = std::ifstream(filename);
            if (!checkFile.is_open()) {
                std::cerr << _("standard_values.file_still_cannot_open") << std::endl;
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
            std::cerr << _("standard_values.json_parse_error") << ": " << e.what() << std::endl;
            std::cerr << _("standard_values.fallback_to_default") << std::endl;
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

        std::cout << _("standard_values.load_success") << ": " << filename << std::endl;
        std::cout << "  " << _("standard_values.supported_confidence_levels") << ": "
                  << confidenceLevels.size() << " " << _("ui.count_unit") << std::endl;
        std::cout << "  " << _("standard_values.sample_size_range") << ": " << minSampleSize << "-"
                  << maxSampleSize << std::endl;

        // 记录当前文件路径
        currentFilePath = filename;

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << _("standard_values.load_error") << ": " << e.what() << std::endl;
        std::cerr << _("standard_values.fallback_to_default") << std::endl;
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

bool StandardValues::importCustomConfidenceLevel(double confidenceLevel,
                                                 const std::string &filename)
{
    try {
        std::map<int, double> customValues;

        // 检查文件扩展名
        std::string extension = filename.substr(filename.find_last_of('.'));
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == ".json") {
            // 解析JSON格式
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << _("standard_values.file_cannot_open") << ": " << filename << std::endl;
                return false;
            }

            json data;
            try {
                file >> data;
            }
            catch (const json::parse_error &e) {
                std::cerr << _("standard_values.json_error") << ": " << e.what() << std::endl;
                return false;
            }

            // 解析JSON数据
            for (auto &item : data.items()) {
                try {
                    int sampleSize = std::stoi(item.key());
                    double wpValue = item.value().get<double>();
                    customValues[sampleSize] = wpValue;
                }
                catch (const std::exception &e) {
                    std::cerr << _("standard_values.parse_item_failed") << ": " << item.key()
                              << " -> " << item.value().dump() << std::endl;
                    return false;
                }
            }

        } else if (extension == ".csv") {
            // 解析CSV格式
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << _("standard_values.file_cannot_open") << ": " << filename << std::endl;
                return false;
            }

            std::string line;
            bool isFirstLine = true;

            while (std::getline(file, line)) {
                // 跳过可能的表头
                if (isFirstLine &&
                    (line.find("sample") != std::string::npos ||
                     line.find("Sample") != std::string::npos || line.find("n") == 0)) {
                    isFirstLine = false;
                    continue;
                }
                isFirstLine = false;

                std::stringstream ss(line);
                std::string sampleSizeStr, wpValueStr;

                if (std::getline(ss, sampleSizeStr, ',') && std::getline(ss, wpValueStr)) {
                    try {
                        int sampleSize = std::stoi(sampleSizeStr);
                        double wpValue = std::stod(wpValueStr);
                        customValues[sampleSize] = wpValue;
                    }
                    catch (const std::exception &e) {
                        std::cerr << _("standard_values.csv_parse_failed") << ": " << line
                                  << std::endl;
                        return false;
                    }
                }
            }

        } else {
            std::cerr << _("standard_values.unsupported_format") << ": " << extension << std::endl;
            std::cerr << _("standard_values.supported_formats") << std::endl;
            return false;
        }

        // 验证数据完整性
        if (!validateCustomValues(customValues)) {
            std::cerr << _("standard_values.validation_failed") << std::endl;
            return false;
        }

        // 检查置信度是否已存在
        if (wpValues.find(confidenceLevel) != wpValues.end()) {
            std::cout << _("standard_values.confidence_exists_warning") << ": " << confidenceLevel
                      << std::endl;
        }

        // 添加到标准值表
        wpValues[confidenceLevel] = customValues;

        // 更新置信度列表
        auto it = std::find(confidenceLevels.begin(), confidenceLevels.end(), confidenceLevel);
        if (it == confidenceLevels.end()) {
            confidenceLevels.push_back(confidenceLevel);
            std::sort(confidenceLevels.begin(), confidenceLevels.end());
        }

        std::cout << _("standard_values.import_success") << ": " << confidenceLevel << std::endl;
        std::cout << _("standard_values.import_data_count") << ": " << customValues.size()
                  << std::endl;

        // 自动保存到当前标准值文件
        if (!currentFilePath.empty()) {
            if (saveToFile(currentFilePath)) {
                std::cout << _("standard_values.save_success") << std::endl;
            } else {
                std::cout << _("standard_values.save_warning") << std::endl;
            }
        } else {
            std::cout << _("standard_values.no_file_path_warning") << std::endl;
        }

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << _("standard_values.import_failed") << ": " << e.what() << std::endl;
        return false;
    }
}

bool StandardValues::removeConfidenceLevel(double confidenceLevel)
{
    // 不允许删除内置的标准置信度
    if (confidenceLevel == 0.95 || confidenceLevel == 0.99 || confidenceLevel == 0.999) {
        std::cerr << _("standard_values.cannot_delete_builtin") << ": " << confidenceLevel
                  << std::endl;
        return false;
    }

    auto it = wpValues.find(confidenceLevel);
    if (it == wpValues.end()) {
        std::cerr << _("standard_values.confidence_not_exists") << ": " << confidenceLevel
                  << std::endl;
        return false;
    }

    // 从标准值表中移除
    wpValues.erase(it);

    // 从置信度列表中移除
    auto levelIt = std::find(confidenceLevels.begin(), confidenceLevels.end(), confidenceLevel);
    if (levelIt != confidenceLevels.end()) {
        confidenceLevels.erase(levelIt);
    }

    std::cout << _("standard_values.remove_success") << ": " << confidenceLevel << std::endl;

    // 自动保存到当前标准值文件
    if (!currentFilePath.empty()) {
        if (saveToFile(currentFilePath)) {
            std::cout << _("standard_values.remove_save_success") << std::endl;
        } else {
            std::cout << _("standard_values.remove_save_warning") << std::endl;
        }
    }

    return true;
}

bool StandardValues::validateCustomValues(const std::map<int, double> &values) const
{
    // 检查是否为空
    if (values.empty()) {
        std::cerr << _("standard_values.empty_table") << std::endl;
        return false;
    }

    // 检查样本数范围
    int minSize = values.begin()->first;
    int maxSize = values.rbegin()->first;

    if (minSize < 4) {
        std::cerr << _("standard_values.min_sample_size_error") << ": " << minSize << std::endl;
        return false;
    }

    if (maxSize > 100) {  // 设置一个合理的上限
        std::cerr << _("standard_values.max_sample_size_error") << ": " << maxSize << std::endl;
        return false;
    }

    // 检查是否包含关键样本数（至少要有4-20的数据）
    std::vector<int> requiredSizes = {4, 5, 6, 7, 8, 9, 10, 15, 20};
    for (int size : requiredSizes) {
        if (values.find(size) == values.end()) {
            std::cerr << _("standard_values.missing_required_size") << ": " << size << std::endl;
            return false;
        }
    }

    // 检查W(P)值的合理性
    for (const auto &pair : values) {
        double wpValue = pair.second;
        if (wpValue <= 0.0 || wpValue > 10.0) {  // W(P)值应该在合理范围内
            std::cerr << _("standard_values.unreasonable_wp") << ": " << pair.first << " -> "
                      << wpValue << std::endl;
            return false;
        }
    }

    // 检查W(P)值的单调性（一般情况下应该随样本数增加而增加）
    double prevValue = 0.0;
    for (const auto &pair : values) {
        if (pair.second < prevValue * 0.8) {  // 允许一定的波动
            std::cerr << _("standard_values.wp_sequence_warning") << ": " << pair.first << " -> "
                      << pair.second << " vs " << prevValue << std::endl;
            // 这里只是警告，不阻止导入
        }
        prevValue = pair.second;
    }

    std::cout << _("standard_values.validation_passed") << std::endl;
    std::cout << _("standard_values.validation_range") << ": " << minSize << "-" << maxSize
              << std::endl;
    std::cout << _("standard_values.validation_count") << ": " << values.size() << std::endl;

    return true;
}

bool StandardValues::saveToFile(const std::string &filename)
{
    try {
        // 创建JSON对象
        json data;

        // 按置信度排序并写入数据
        for (double confidenceLevel : confidenceLevels) {
            auto it = wpValues.find(confidenceLevel);
            if (it != wpValues.end()) {
                json levelData;

                // 按样本数排序写入数据
                for (const auto &pair : it->second) {
                    levelData[std::to_string(pair.first)] = pair.second;
                }

                data[std::to_string(confidenceLevel)] = levelData;
            }
        }

        // 写入文件
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << _("standard_values.cannot_create_output_file") << ": " << filename
                      << std::endl;
            return false;
        }

        file << data.dump(2);  // 缩进为2空格
        file.close();

        std::cout << _("standard_values.file_save_success") << ": " << filename << std::endl;

        // 更新当前文件路径
        currentFilePath = filename;

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << _("standard_values.save_file_failed") << ": " << e.what() << std::endl;
        return false;
    }
}

void StandardValues::setUserFilePath(const std::string &filePath)
{
    currentFilePath = filePath;
}

}  // namespace neumann