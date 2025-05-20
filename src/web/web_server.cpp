#include "web/web_server.h"
#include "core/data_manager.h"
#include "core/neumann_calculator.h"
#include "core/standard_values.h"
#include <crow.h>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace neumann {
namespace web {

// PIMPL实现类
class WebServer::Impl {
public:
  Impl(int port, const std::string &webRootDir)
      : app(), port(port), webRootDir(webRootDir) {}

  crow::SimpleApp app;
  int port;
  std::string webRootDir;
  std::thread serverThread;
};

WebServer::WebServer(int port, const std::string &webRootDir)
    : port(port), webRootDir(webRootDir), running(false),
      impl(new Impl(port, webRootDir)) {
  // 确保Web资源目录存在
  if (!fs::exists(webRootDir)) {
    std::cerr << "警告: Web资源目录不存在: " << webRootDir << std::endl;
  }

  // 初始化路由
  initializeRoutes();
}

void WebServer::start(bool background) {
  if (running) {
    std::cerr << "服务器已经在运行中" << std::endl;
    return;
  }

  // 启动服务器
  std::cout << "启动Web服务器，监听端口: " << port << std::endl;
  std::cout << "Web界面访问URL: " << getUrl() << std::endl;

  if (background) {
    // 在后台线程中启动
    impl->serverThread =
        std::thread([this]() { impl->app.port(port).multithreaded().run(); });
  } else {
    // 在当前线程中启动
    impl->app.port(port).run();
  }

  running = true;
}

void WebServer::stop() {
  if (!running) {
    return;
  }

  std::cout << "停止Web服务器" << std::endl;

  // 停止服务器
  impl->app.stop();

  // 等待后台线程结束
  if (impl->serverThread.joinable()) {
    impl->serverThread.join();
  }

  running = false;
}

bool WebServer::isRunning() const { return running; }

std::string WebServer::getUrl() const {
  return "http://localhost:" + std::to_string(port);
}

void WebServer::initializeRoutes() {
  // 静态文件服务
  CROW_ROUTE(impl->app, "/<path>")
  ([this](const std::string &path) {
    std::string filePath = impl->webRootDir + "/" + path;

    // 如果请求的是目录或空路径，则提供index.html
    if (path.empty() || fs::is_directory(filePath)) {
      filePath = impl->webRootDir + "/neumann_trend_test.html";
    }

    // 检查文件是否存在
    if (!fs::exists(filePath)) {
      return crow::response(404);
    }

    // 确定内容类型
    std::string contentType = "text/plain";
    std::string extension = fs::path(filePath).extension().string();

    if (extension == ".html") {
      contentType = "text/html";
    } else if (extension == ".css") {
      contentType = "text/css";
    } else if (extension == ".js") {
      contentType = "application/javascript";
    } else if (extension == ".json") {
      contentType = "application/json";
    }

    // 读取文件内容
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
      return crow::response(500);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // 返回文件内容
    crow::response response(content);
    response.set_header("Content-Type", contentType);
    return response;
  });

  // 默认路由重定向到index.html
  CROW_ROUTE(impl->app, "/")
  ([this]() {
    crow::response response;
    response.code = 302;
    response.set_header("Location", "/neumann_trend_test.html");
    return response;
  });

  // 注册API端点
  registerApiEndpoints();
}

void WebServer::registerApiEndpoints() {
  // API端点: 诺依曼趋势测试
  CROW_ROUTE(impl->app, "/api/neumann_test")
      .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
        return handleNeumannTestRequest(req.body);
      });

  // API端点: 获取数据集列表
  CROW_ROUTE(impl->app, "/api/datasets")
      .methods(crow::HTTPMethod::GET)(
          [this]() { return handleDataSetListRequest(); });

  // API端点: 加载数据集
  CROW_ROUTE(impl->app, "/api/dataset")
      .methods(crow::HTTPMethod::GET)([this](const crow::request &req) {
        return handleDataSetLoadRequest(req.body);
      });

  // API端点: 保存数据集
  CROW_ROUTE(impl->app, "/api/dataset")
      .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
        return handleDataSetSaveRequest(req.body);
      });
}

std::string
WebServer::handleNeumannTestRequest(const std::string &requestBody) {
  try {
    // 解析请求体
    json request = json::parse(requestBody);

    // 获取请求参数
    std::vector<double> dataPoints = request["data"].get<std::vector<double>>();
    std::vector<double> timePoints = request["time"].get<std::vector<double>>();
    double confidenceLevel = request.value("confidenceLevel", 0.95);

    // 验证输入
    if (dataPoints.size() < 4) {
      json error = {{"success", false},
                    {"error", "需要至少4个数据点才能进行诺依曼趋势测试"}};
      return error.dump();
    }

    if (timePoints.size() != dataPoints.size()) {
      json error = {{"success", false},
                    {"error", "时间点数量必须与数据点数量一致"}};
      return error.dump();
    }

    // 运行诺依曼趋势测试
    NeumannCalculator calculator(confidenceLevel);
    NeumannTestResults results = calculator.performTest(dataPoints, timePoints);

    // 构建响应
    json response = {
        {"success", true},        {"data", dataPoints},
        {"time", timePoints},     {"overallTrend", results.overallTrend},
        {"minPG", results.minPG}, {"maxPG", results.maxPG},
        {"avgPG", results.avgPG}, {"results", json::array()}};

    // 添加每个测试点的结果
    for (size_t i = 0; i < results.results.size(); ++i) {
      size_t dataIndex = i + 3; // 从第4个数据点开始计算PG值

      json result = {{"dataPoint", results.data[dataIndex]},
                     {"timePoint", results.timePoints[dataIndex]},
                     {"pgValue", results.results[i].pgValue},
                     {"wpThreshold", results.results[i].wpThreshold},
                     {"hasTrend", results.results[i].hasTrend}};

      response["results"].push_back(result);
    }

    return response.dump();
  } catch (const std::exception &e) {
    json error = {{"success", false},
                  {"error", std::string("处理请求时出错: ") + e.what()}};
    return error.dump();
  }
}

std::string WebServer::handleDataSetListRequest() {
  try {
    // 获取数据集列表
    std::vector<std::string> datasets =
        DataManager::getInstance().getDataSetNames();

    // 构建响应
    json response = {{"success", true}, {"datasets", datasets}};

    return response.dump();
  } catch (const std::exception &e) {
    json error = {{"success", false},
                  {"error", std::string("获取数据集列表时出错: ") + e.what()}};
    return error.dump();
  }
}

std::string
WebServer::handleDataSetLoadRequest(const std::string &requestBody) {
  try {
    // 解析请求体
    json request = json::parse(requestBody);

    // 获取请求参数
    std::string name = request["name"].get<std::string>();

    // 加载数据集
    DataSet dataSet = DataManager::getInstance().loadDataSet(name);

    // 构建响应
    json response = {{"success", true},
                     {"name", dataSet.name},
                     {"description", dataSet.description},
                     {"source", dataSet.source},
                     {"createdAt", dataSet.createdAt},
                     {"data", dataSet.dataPoints},
                     {"time", dataSet.timePoints}};

    return response.dump();
  } catch (const std::exception &e) {
    json error = {{"success", false},
                  {"error", std::string("加载数据集时出错: ") + e.what()}};
    return error.dump();
  }
}

std::string
WebServer::handleDataSetSaveRequest(const std::string &requestBody) {
  try {
    // 解析请求体
    json request = json::parse(requestBody);

    // 创建数据集
    DataSet dataSet;
    dataSet.name = request["name"].get<std::string>();
    dataSet.description = request.value("description", "");
    dataSet.source = request.value("source", "Web界面");
    dataSet.createdAt = request.value("createdAt", "");
    dataSet.dataPoints = request["data"].get<std::vector<double>>();
    dataSet.timePoints = request["time"].get<std::vector<double>>();

    // 如果未提供创建时间，则设置为当前时间
    if (dataSet.createdAt.empty()) {
      auto now = std::chrono::system_clock::now();
      auto timeT = std::chrono::system_clock::to_time_t(now);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
      dataSet.createdAt = ss.str();
    }

    // 保存数据集
    bool success = DataManager::getInstance().saveDataSet(dataSet);

    // 构建响应
    json response = {{"success", success}};

    if (!success) {
      response["error"] = "保存数据集失败";
    }

    return response.dump();
  } catch (const std::exception &e) {
    json error = {{"success", false},
                  {"error", std::string("保存数据集时出错: ") + e.what()}};
    return error.dump();
  }
}

} // namespace web
} // namespace neumann