#include "core/data_manager.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>

#include "core/config.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace neumann {

DataManager::DataManager()
{
    // 从配置获取数据目录
    auto &config = Config::getInstance();
    dataDir = config.getDataDirectory();

    // 创建数据目录（如果不存在）
    if (!fs::exists(dataDir)) {
        fs::create_directories(dataDir);
    }
}

DataManager &DataManager::getInstance()
{
    static DataManager instance;
    return instance;
}

DataSet DataManager::importFromCSV(const std::string &filename, bool hasHeader)
{
    DataSet dataSet;

    // 从文件名提取数据集名称
    fs::path filePath(filename);
    dataSet.name = filePath.stem().string();
    dataSet.source = filename;

    // 获取当前时间作为创建时间
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    dataSet.createdAt = ss.str();

    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开CSV文件: " << filename << std::endl;
            return dataSet;
        }

        std::string line;

        // 跳过表头（如果有）
        if (hasHeader && std::getline(file, line)) {
            // 表头可以用作描述
            dataSet.description = line;
        }

        // 读取数据
        while (std::getline(file, line)) {
            std::stringstream lineStream(line);
            std::string cell;

            // 假设CSV格式为: 时间点,数据点
            if (std::getline(lineStream, cell, ',')) {
                try {
                    double timePoint = std::stod(cell);
                    dataSet.timePoints.push_back(timePoint);
                }
                catch (const std::exception &e) {
                    std::cerr << "解析时间点出错: " << cell << std::endl;
                    continue;
                }
            }

            if (std::getline(lineStream, cell, ',')) {
                try {
                    double dataPoint = std::stod(cell);
                    dataSet.dataPoints.push_back(dataPoint);
                }
                catch (const std::exception &e) {
                    std::cerr << "解析数据点出错: " << cell << std::endl;
                    continue;
                }
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "导入CSV文件时出错: " << e.what() << std::endl;
    }

    return dataSet;
}

bool DataManager::exportToCSV(const DataSet &dataSet, const std::string &filename)
{
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法创建CSV文件: " << filename << std::endl;
            return false;
        }

        // 写入表头
        file << "Time,Value" << std::endl;

        // 写入数据
        for (size_t i = 0; i < dataSet.dataPoints.size() && i < dataSet.timePoints.size(); ++i) {
            file << dataSet.timePoints[i] << "," << dataSet.dataPoints[i] << std::endl;
        }

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "导出到CSV文件时出错: " << e.what() << std::endl;
        return false;
    }
}

bool DataManager::saveDataSet(const DataSet &dataSet)
{
    if (dataSet.name.empty()) {
        std::cerr << "数据集名称不能为空" << std::endl;
        return false;
    }

    try {
        // 构建JSON对象
        json j;
        j["name"] = dataSet.name;
        j["description"] = dataSet.description;
        j["source"] = dataSet.source;
        j["createdAt"] = dataSet.createdAt;
        j["timePoints"] = dataSet.timePoints;
        j["dataPoints"] = dataSet.dataPoints;

        // 保存到文件
        std::string filePath = dataDir + "/" + dataSet.name + ".json";
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法创建数据集文件: " << filePath << std::endl;
            return false;
        }

        file << std::setw(4) << j << std::endl;

        // 添加到缓存
        loadedDataSets[dataSet.name] = dataSet;

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "保存数据集时出错: " << e.what() << std::endl;
        return false;
    }
}

DataSet DataManager::loadDataSet(const std::string &name)
{
    // 检查缓存
    auto it = loadedDataSets.find(name);
    if (it != loadedDataSets.end()) {
        return it->second;
    }

    DataSet dataSet;
    dataSet.name = name;

    try {
        // 从文件加载
        std::string filePath = dataDir + "/" + name + ".json";
        if (!fs::exists(filePath)) {
            std::cerr << "数据集文件不存在: " << filePath << std::endl;
            return dataSet;
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法打开数据集文件: " << filePath << std::endl;
            return dataSet;
        }

        json j;
        file >> j;

        // 解析JSON
        dataSet.name = j["name"];
        dataSet.description = j["description"];
        dataSet.source = j["source"];
        dataSet.createdAt = j["createdAt"];
        dataSet.timePoints = j["timePoints"].get<std::vector<double>>();
        dataSet.dataPoints = j["dataPoints"].get<std::vector<double>>();

        // 添加到缓存
        loadedDataSets[name] = dataSet;

        return dataSet;
    }
    catch (const std::exception &e) {
        std::cerr << "加载数据集时出错: " << e.what() << std::endl;
        return dataSet;
    }
}

std::vector<std::string> DataManager::getDataSetNames()
{
    std::vector<std::string> names;

    // 定义需要过滤的系统文件
    std::set<std::string> systemFiles = {"standard_values", "translations", "config"};

    try {
        for (const auto &entry : fs::directory_iterator(dataDir)) {
            if (entry.path().extension() == ".json") {
                std::string filename = entry.path().stem().string();

                // 过滤系统文件
                if (systemFiles.find(filename) == systemFiles.end()) {
                    names.push_back(filename);
                }
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "获取数据集名称时出错: " << e.what() << std::endl;
    }

    return names;
}

bool DataManager::deleteDataSet(const std::string &name)
{
    try {
        std::string filePath = dataDir + "/" + name + ".json";

        // 从文件系统删除
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }

        // 从缓存中删除
        loadedDataSets.erase(name);

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "删除数据集时出错: " << e.what() << std::endl;
        return false;
    }
}

}  // namespace neumann