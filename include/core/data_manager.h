#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace neumann {

/**
 * @brief 数据集合结构体
 */
struct DataSet {
  std::string name;               // 数据集名称
  std::vector<double> dataPoints; // 数据点
  std::vector<double> timePoints; // 时间点
  std::string description;        // 描述信息
  std::string source;             // 数据来源
  std::string createdAt;          // 创建时间
};

/**
 * @brief 数据管理器类
 *
 * 负责数据的导入、导出和管理
 */
class DataManager {
public:
  /**
   * @brief 获取DataManager单例实例
   * @return DataManager的共享实例
   */
  static DataManager &getInstance();

  /**
   * @brief 从CSV文件导入数据
   * @param filename CSV文件路径
   * @param hasHeader 文件是否包含表头
   * @return 导入的数据集
   */
  DataSet importFromCSV(const std::string &filename, bool hasHeader = true);

  /**
   * @brief 导出数据到CSV文件
   * @param dataSet 要导出的数据集
   * @param filename 目标CSV文件路径
   * @return 是否成功导出
   */
  bool exportToCSV(const DataSet &dataSet, const std::string &filename);

  /**
   * @brief 保存数据集
   * @param dataSet 要保存的数据集
   * @return 是否成功保存
   */
  bool saveDataSet(const DataSet &dataSet);

  /**
   * @brief 加载数据集
   * @param name 数据集名称
   * @return 加载的数据集
   */
  DataSet loadDataSet(const std::string &name);

  /**
   * @brief 获取所有已保存的数据集名称
   * @return 数据集名称列表
   */
  std::vector<std::string> getDataSetNames();

  /**
   * @brief 删除数据集
   * @param name 要删除的数据集名称
   * @return 是否成功删除
   */
  bool deleteDataSet(const std::string &name);

private:
  // 私有构造函数，防止外部实例化
  DataManager();

  // 禁用拷贝和赋值
  DataManager(const DataManager &) = delete;
  DataManager &operator=(const DataManager &) = delete;

  // 保存路径
  std::string dataDir;

  // 缓存已加载的数据集
  std::map<std::string, DataSet> loadedDataSets;
};

} // namespace neumann