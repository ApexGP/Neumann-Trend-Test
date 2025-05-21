#pragma once

#include <memory>
#include <string>

namespace neumann {
namespace web {

// 前向声明Impl类
class WebServerImpl;

/**
 * @brief Web服务器类
 *
 * 提供基于HTTP的Web接口，用于在浏览器中访问诺依曼趋势测试功能
 */
class WebServer {
public:
  /**
   * @brief 构造函数
   * @param port 服务器监听端口
   * @param webRootDir Web资源文件目录
   */
  WebServer(int port = 8080, const std::string &webRootDir = "web");

  /**
   * @brief 析构函数
   */
  ~WebServer();

  /**
   * @brief 启动Web服务器
   * @param background 是否在后台运行
   */
  void start(bool background = false);

  /**
   * @brief 停止Web服务器
   */
  void stop();

  /**
   * @brief 获取服务器状态
   * @return 服务器是否运行中
   */
  bool isRunning() const;

  /**
   * @brief 获取服务器URL
   * @return 服务器访问URL
   */
  std::string getUrl() const;

private:
  // 服务器端口
  int port;

  // Web资源目录
  std::string webRootDir;

  // 服务器是否运行
  bool running;

  // 服务器实现（使用PIMPL模式以隐藏Crow依赖）
  std::unique_ptr<WebServerImpl> impl;

  // 初始化路由
  void initializeRoutes();

  // 注册API端点
  void registerApiEndpoints();

  // 处理诺依曼趋势测试请求
  std::string handleNeumannTestRequest(const std::string &requestBody);

  // 处理数据集列表请求
  std::string handleDataSetListRequest();

  // 处理数据集加载请求
  std::string handleDataSetLoadRequest(const std::string &requestBody);

  // 处理数据集保存请求
  std::string handleDataSetSaveRequest(const std::string &requestBody);
};

} // namespace web
} // namespace neumann