#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace neumann {

/**
 * @brief 诺依曼趋势测试的标准值管理类
 *
 * 管理诺依曼趋势测试的标准W(P)值，支持从文件加载和访问
 */
class StandardValues
{
public:
    /**
   * @brief 获取StandardValues单例实例
   * @return StandardValues的共享实例
   */
    static StandardValues &getInstance();

    /**
   * @brief 从JSON文件加载标准值
   * @param filename JSON文件路径
   * @return 是否成功加载
   */
    bool loadFromFile(const std::string &filename);

    /**
   * @brief 保存当前标准值到JSON文件
   * @param filename JSON文件路径
   * @return 是否成功保存
   */
    bool saveToFile(const std::string &filename);

    /**
   * @brief 获取指定样本数和置信水平的W(P)值
   * @param sampleSize 样本数量
   * @param confidenceLevel 置信水平 (0.95, 0.99等)
   * @return 对应的W(P)值，如果不存在返回-1.0
   */
    double getWPValue(int sampleSize, double confidenceLevel = 0.95) const;

    /**
   * @brief 获取支持的最小样本数
   * @return 最小样本数
   */
    int getMinSampleSize() const;

    /**
   * @brief 获取支持的最大样本数
   * @return 最大样本数
   */
    int getMaxSampleSize() const;

    /**
   * @brief 获取所有支持的置信水平
   * @return 包含所有支持的置信水平的向量
   */
    std::vector<double> getSupportedConfidenceLevels() const;

    /**
   * @brief 导入自定义置信度的标准值表
   * @param confidenceLevel 置信度水平
   * @param filename 包含标准值的文件路径（JSON或CSV格式）
   * @return 是否成功导入
   */
    bool importCustomConfidenceLevel(double confidenceLevel, const std::string &filename);

    /**
   * @brief 移除指定的置信度水平
   * @param confidenceLevel 要移除的置信度水平
   * @return 是否成功移除
   */
    bool removeConfidenceLevel(double confidenceLevel);

    /**
   * @brief 验证自定义标准值表的完整性
   * @param values 标准值映射（样本数 -> W(P)值）
   * @return 是否通过验证
   */
    bool validateCustomValues(const std::map<int, double> &values) const;

    /**
   * @brief 设置用户标准值文件路径（用于保存自定义标准值）
   * @param filePath 用户标准值文件的完整路径
   */
    void setUserFilePath(const std::string &filePath);

private:
    // 私有构造函数，防止外部实例化
    StandardValues();

    // 禁用拷贝和赋值
    StandardValues(const StandardValues &) = delete;
    StandardValues &operator=(const StandardValues &) = delete;

    // 标准值数据结构: map<置信水平, map<样本数, W(P)值>>
    std::map<double, std::map<int, double>> wpValues;

    // 缓存最小和最大样本数
    int minSampleSize;
    int maxSampleSize;

    // 缓存支持的置信水平
    std::vector<double> confidenceLevels;

    // 当前标准值文件路径
    std::string currentFilePath;
};

}  // namespace neumann