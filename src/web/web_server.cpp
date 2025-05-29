#include "web/web_server.h"

// 标准库头文件
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

// 第三方库
#define CROW_MAIN
#include <crow.h>

#include <nlohmann/json.hpp>

// 项目头文件
#include "core/batch_processor.h"
#include "core/config.h"
#include "core/data_manager.h"
#include "core/data_visualization.h"
#include "core/excel_reader.h"
#include "core/i18n.h"
#include "core/neumann_calculator.h"
#include "core/standard_values.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace neumann { namespace web {

// PIMPL实现类
class WebServerImpl
{
public:
    WebServerImpl(int port, const std::string &webRootDir)
        : app(), port(port), webRootDir(webRootDir)
    {
    }

    crow::SimpleApp app;
    int port;
    std::string webRootDir;
    std::thread serverThread;
};

WebServer::WebServer(int port, const std::string &webRootDir)
    : port(port), webRootDir(webRootDir), running(false), impl(new WebServerImpl(port, webRootDir))
{
    // 确保Web资源目录存在
    if (!fs::exists(webRootDir)) {
        std::cerr << "警告: Web资源目录不存在: " << webRootDir << std::endl;
        try {
            fs::create_directory(webRootDir);
            std::cout << "已创建Web资源目录: " << webRootDir << std::endl;
        }
        catch (const std::exception &e) {
            std::cerr << "错误: 无法创建Web资源目录: " << e.what() << std::endl;
        }
    }

    // 检查是否有索引页面文件
    std::string indexFile = webRootDir + "/neumann_trend_test.html";
    if (!fs::exists(indexFile)) {
        std::cerr << "警告: 未找到Web界面文件: " << indexFile << std::endl;
    }

    // 初始化路由
    initializeRoutes();
}

WebServer::~WebServer()
{
    if (running) {
        stop();
    }
}

void WebServer::start(bool background)
{
    if (running) {
        std::cerr << "服务器已经在运行中" << std::endl;
        return;
    }

    std::cout << "启动Web服务器，监听端口: " << port << std::endl;
    std::cout << "Web界面访问URL: " << getUrl() << std::endl;

    if (background) {
        impl->serverThread = std::thread([this]() { impl->app.port(port).multithreaded().run(); });
    } else {
        impl->app.port(port).run();
    }

    running = true;
}

void WebServer::stop()
{
    if (!running) {
        return;
    }

    std::cout << "停止Web服务器" << std::endl;
    impl->app.stop();

    if (impl->serverThread.joinable()) {
        impl->serverThread.join();
    }

    running = false;
}

bool WebServer::isRunning() const
{
    return running;
}

std::string WebServer::getUrl() const
{
    return "http://localhost:" + std::to_string(port);
}

void WebServer::initializeRoutes()
{
    // 先注册API端点
    registerApiEndpoints();

    // 默认路由
    CROW_ROUTE(impl->app, "/")
    ([this]() {
        crow::response response;
        response.code = 302;
        response.set_header("Location", "/neumann_trend_test.html");
        return response;
    });

    // 静态文件服务（放在最后，避免拦截API请求）
    CROW_ROUTE(impl->app, "/<path>")
    ([this](const std::string &path) {
        std::string filePath = impl->webRootDir + "/" + path;

        if (path.empty() || fs::is_directory(filePath)) {
            filePath = impl->webRootDir + "/neumann_trend_test.html";
        }

        if (!fs::exists(filePath)) {
            return crow::response(404);
        }

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

        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            return crow::response(500);
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        crow::response response(content);
        response.set_header("Content-Type", contentType);
        return response;
    });
}

void WebServer::registerApiEndpoints()
{
    // 测试端点
    CROW_ROUTE(impl->app, "/api/test")
    ([this]() {
        json response = {{"success", true}, {"message", "API正常工作"}};
        return response.dump();
    });

    // 核心测试API
    CROW_ROUTE(impl->app, "/api/neumann_test")
        .methods("POST"_method)(
            [this](const crow::request &req) { return handleNeumannTestRequest(req.body); });

    // 数据集管理
    CROW_ROUTE(impl->app, "/api/datasets")
    ([this]() { return handleDataSetListRequest(); });

    CROW_ROUTE(impl->app, "/api/dataset").methods("POST"_method)([this](const crow::request &req) {
        return handleDataSetSaveRequest(req.body);
    });

    CROW_ROUTE(impl->app, "/api/dataset/<string>")
    ([this](const std::string &name) { return handleDataSetLoadRequestByName(name); });

    CROW_ROUTE(impl->app, "/api/dataset/delete/<string>")
    ([this](const std::string &name) {
        json request = {{"name", name}};
        return handleDataSetDeleteRequest(request.dump());
    });

    // 样本数据
    CROW_ROUTE(impl->app, "/api/sample_data")
    ([this]() { return handleSampleDataListRequest(); });

    CROW_ROUTE(impl->app, "/api/sample_data/load")
        .methods("POST"_method)(
            [this](const crow::request &req) { return handleSampleDataLoadRequest(req.body); });

    // 配置管理
    CROW_ROUTE(impl->app, "/api/config")
    ([this]() { return handleConfigGetRequest(); });

    CROW_ROUTE(impl->app, "/api/config/update")
        .methods("PUT"_method)(
            [this](const crow::request &req) { return handleConfigUpdateRequest(req.body); });

    // 语言管理
    CROW_ROUTE(impl->app, "/api/language/<string>")
        .methods("POST"_method)(
            [this](const std::string &language) { return handleLanguageChangeRequest(language); });

    // 置信度管理
    CROW_ROUTE(impl->app, "/api/confidence_level/<string>")
        .methods("POST"_method)(
            [this](const std::string &level) { return handleConfidenceLevelChangeRequest(level); });

    // 标准值
    CROW_ROUTE(impl->app, "/api/standard_values")
    ([this]() { return handleStandardValuesGetRequest(); });

    // 翻译
    CROW_ROUTE(impl->app, "/api/translations/<string>")
    ([this](const std::string &language) { return handleTranslationsGetRequest(language); });

    // 统计
    CROW_ROUTE(impl->app, "/api/statistics")
    ([this]() { return handleStatisticsRequest(); });
}

// 简化的API处理函数

std::string WebServer::handleNeumannTestRequest(const std::string &requestBody)
{
    try {
        json request = json::parse(requestBody);
        std::vector<double> dataPoints = request["data"].get<std::vector<double>>();
        std::vector<double> timePoints = request["time"].get<std::vector<double>>();
        double confidenceLevel = request.value("confidenceLevel", 0.95);

        if (dataPoints.size() < 4) {
            json error = {{"success", false}, {"error", "需要至少4个数据点"}};
            return error.dump();
        }

        if (timePoints.size() != dataPoints.size()) {
            json error = {{"success", false}, {"error", "时间点数量必须与数据点数量一致"}};
            return error.dump();
        }

        NeumannCalculator calculator(confidenceLevel);
        NeumannTestResults results = calculator.performTest(dataPoints, timePoints);

        json response = {{"success", true},        {"data", dataPoints},
                         {"time", timePoints},     {"overallTrend", results.overallTrend},
                         {"minPG", results.minPG}, {"maxPG", results.maxPG},
                         {"avgPG", results.avgPG}, {"results", json::array()}};

        for (size_t i = 0; i < results.results.size(); ++i) {
            size_t dataIndex = i + 3;
            json result = {{"dataPoint", results.data[dataIndex]},
                           {"timePoint", results.timePoints[dataIndex]},
                           {"pgValue", results.results[i].pgValue},
                           {"wpThreshold", results.results[i].wpThreshold},
                           {"hasTrend", results.results[i].hasTrend}};
            response["results"].push_back(result);
        }

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("处理请求时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleDataSetListRequest()
{
    try {
        std::vector<std::string> datasets = DataManager::getInstance().getDataSetNames();
        json response = {{"success", true}, {"datasets", datasets}};
        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false},
                      {"error", std::string("获取数据集列表时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleDataSetLoadRequestByName(const std::string &name)
{
    try {
        DataSet dataSet = DataManager::getInstance().loadDataSet(name);

        json response = {{"success", true},
                         {"name", dataSet.name},
                         {"description", dataSet.description},
                         {"source", dataSet.source},
                         {"createdAt", dataSet.createdAt},
                         {"data", dataSet.dataPoints},
                         {"time", dataSet.timePoints}};

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("加载数据集时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleDataSetSaveRequest(const std::string &requestBody)
{
    try {
        json request = json::parse(requestBody);
        DataSet dataSet;
        dataSet.name = request["name"].get<std::string>();
        dataSet.description = request.value("description", "");
        dataSet.source = request.value("source", "Web界面");
        dataSet.createdAt = request.value("createdAt", "");
        dataSet.dataPoints = request["data"].get<std::vector<double>>();
        dataSet.timePoints = request["time"].get<std::vector<double>>();

        if (dataSet.createdAt.empty()) {
            auto now = std::chrono::system_clock::now();
            auto timeT = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
            dataSet.createdAt = ss.str();
        }

        bool success = DataManager::getInstance().saveDataSet(dataSet);
        json response = {{"success", success}};
        if (!success) {
            response["error"] = "保存数据集失败";
        }

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("保存数据集时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleDataSetDeleteRequest(const std::string &requestBody)
{
    try {
        json request = json::parse(requestBody);
        std::string name = request["name"].get<std::string>();
        bool success = DataManager::getInstance().deleteDataSet(name);

        json response = {{"success", success}};
        if (!success) {
            response["error"] = "删除数据集失败";
        }

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("删除数据集时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleSampleDataListRequest()
{
    try {
        std::vector<std::string> sampleFiles;
        std::string sampleDir = "data/sample/TestSuite";

        if (fs::exists(sampleDir) && fs::is_directory(sampleDir)) {
            for (const auto &entry : fs::directory_iterator(sampleDir)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".csv" || ext == ".txt") {
                        sampleFiles.push_back(entry.path().filename().string());
                    }
                }
            }
        }

        json response = {{"success", true}, {"files", sampleFiles}};
        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false},
                      {"error", std::string("获取样本数据列表时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleConfigGetRequest()
{
    try {
        auto &config = Config::getInstance();
        auto &i18n = I18n::getInstance();

        json response = {
            {"success", true},
            {"config",
             {{"language", i18n.getCurrentLanguage() == Language::CHINESE ? "zh" : "en"},
              {"defaultConfidenceLevel", config.getDefaultConfidenceLevel()},
              {"showWelcomeMessage", config.getShowWelcomeMessage()},
              {"enableColorOutput", config.getEnableColorOutput()},
              {"maxDataPoints", config.getMaxDataPoints()},
              {"autoSaveResults", config.getAutoSaveResults()},
              {"defaultWebPort", config.getDefaultWebPort()}}}};

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("获取配置时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleConfigUpdateRequest(const std::string &requestBody)
{
    try {
        json request = json::parse(requestBody);
        auto &config = Config::getInstance();

        if (request.contains("defaultConfidenceLevel")) {
            config.setDefaultConfidenceLevel(request["defaultConfidenceLevel"]);
        }
        if (request.contains("showWelcomeMessage")) {
            config.setShowWelcomeMessage(request["showWelcomeMessage"]);
        }
        if (request.contains("enableColorOutput")) {
            config.setEnableColorOutput(request["enableColorOutput"]);
        }
        if (request.contains("maxDataPoints")) {
            config.setMaxDataPoints(request["maxDataPoints"]);
        }
        if (request.contains("autoSaveResults")) {
            config.setAutoSaveResults(request["autoSaveResults"]);
        }

        bool success = config.saveConfiguration();
        json response = {{"success", success}};
        if (!success) {
            response["error"] = "保存配置失败";
        }

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("更新配置时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleTranslationsGetRequest(const std::string &language)
{
    try {
        auto &i18n = I18n::getInstance();
        Language originalLang = i18n.getCurrentLanguage();
        Language requestedLang = (language == "zh") ? Language::CHINESE : Language::ENGLISH;

        // 临时切换语言获取翻译
        i18n.setLanguage(requestedLang);

        json translations;

        // 应用标题和描述
        translations["app.title"] = i18n.getText("app.title");
        translations["app.description"] = i18n.getText("app.description");

        // 标签页
        translations["tab.test"] = (language == "zh") ? "测试" : "Test";
        translations["tab.data"] = (language == "zh") ? "数据" : "Data";
        translations["tab.config"] = (language == "zh") ? "设置" : "Settings";

        // 输入标签
        translations["input.data"] = (language == "zh") ? "数据点" : "Data Points";
        translations["input.confidence"] = (language == "zh") ? "置信水平" : "Confidence Level";
        translations["input.default_time"] =
            (language == "zh") ? "使用默认时间点" : "Use Default Time Points";
        translations["input.time"] = (language == "zh") ? "时间点" : "Time Points";

        // 动作按钮
        translations["action.run_test"] = (language == "zh") ? "运行测试" : "Run Test";
        translations["action.save_dataset"] = (language == "zh") ? "保存数据集" : "Save Dataset";
        translations["action.load"] = (language == "zh") ? "加载" : "Load";
        translations["action.delete"] = (language == "zh") ? "删除" : "Delete";
        translations["action.load_sample"] =
            (language == "zh") ? "加载样本数据" : "Load Sample Data";
        translations["action.save_config"] = (language == "zh") ? "保存配置" : "Save Config";
        translations["action.reload_standards"] =
            (language == "zh") ? "重新加载标准值" : "Reload Standards";

        // 数据管理
        translations["data.name"] = (language == "zh") ? "数据集名称" : "Dataset Name";
        translations["data.saved"] = (language == "zh") ? "已保存的数据集" : "Saved Datasets";
        translations["data.sample"] = (language == "zh") ? "样本数据" : "Sample Data";
        translations["data.select"] = (language == "zh") ? "选择数据集..." : "Select dataset...";
        translations["data.sample.select"] =
            (language == "zh") ? "选择样本数据..." : "Select sample data...";

        // 配置
        translations["config.default_confidence"] =
            (language == "zh") ? "默认置信水平" : "Default Confidence Level";
        translations["config.auto_save"] =
            (language == "zh") ? "自动保存结果" : "Auto Save Results";
        translations["config.color_output"] =
            (language == "zh") ? "启用彩色输出" : "Enable Color Output";
        translations["config.standard_values"] =
            (language == "zh") ? "标准值文件" : "Standard Values File";

        // 结果
        translations["result.title"] = (language == "zh") ? "测试结果" : "Test Results";
        translations["result.description"] =
            (language == "zh") ? "诺依曼趋势测试分析结果将在这里显示"
                               : "Neumann trend test analysis results will be displayed here";
        translations["status.loading"] = (language == "zh") ? "正在计算..." : "Loading...";
        translations["current.confidence"] = (language == "zh") ? "置信水平" : "Confidence Level";

        // 测试结果相关翻译
        translations["result.conclusion"] = (language == "zh") ? "测试结论：" : "Test Conclusion:";
        translations["result.trend_detected"] =
            (language == "zh") ? "检测到趋势" : "Trend Detected";
        translations["result.no_trend"] = (language == "zh") ? "无明显趋势" : "No Trend";
        translations["result.test_points"] = (language == "zh") ? "测试点数" : "Test Points";
        translations["result.min_pg"] = (language == "zh") ? "最小PG值" : "Min PG Value";
        translations["result.max_pg"] = (language == "zh") ? "最大PG值" : "Max PG Value";
        translations["result.avg_pg"] = (language == "zh") ? "平均PG值" : "Average PG Value";
        translations["result.chart_title"] =
            (language == "zh") ? "PG值趋势图" : "PG Value Trend Chart";
        translations["result.details_title"] = (language == "zh") ? "详细结果" : "Detailed Results";

        // 表格标题
        translations["table.data_point"] = (language == "zh") ? "数据点" : "Data Point";
        translations["table.time_point"] = (language == "zh") ? "时间点" : "Time Point";
        translations["table.pg_value"] = (language == "zh") ? "PG值" : "PG Value";
        translations["table.threshold"] = (language == "zh") ? "W(P)阈值" : "W(P) Threshold";
        translations["table.trend_judgment"] = (language == "zh") ? "趋势判断" : "Trend Judgment";
        translations["table.has_trend"] = (language == "zh") ? "有趋势" : "Has Trend";
        translations["table.no_trend"] = (language == "zh") ? "无趋势" : "No Trend";

        // 图表标签
        translations["chart.pg_value"] = (language == "zh") ? "PG值" : "PG Value";
        translations["chart.threshold"] = (language == "zh") ? "阈值" : "Threshold";

        // 错误消息
        translations["error.no_data"] =
            (language == "zh") ? "请输入数据点" : "Please enter data points";
        translations["error.min_points"] =
            (language == "zh") ? "至少需要4个数据点才能进行诺依曼趋势测试"
                               : "At least 4 data points are required for Neumann trend test";
        translations["error.time_mismatch"] =
            (language == "zh") ? "时间点数量必须与数据点数量一致"
                               : "Number of time points must match number of data points";
        translations["error.test_failed"] = (language == "zh") ? "测试失败" : "Test failed";
        translations["error.network"] = (language == "zh") ? "网络错误" : "Network error";

        // 欢迎信息
        translations["welcome.title"] = (language == "zh") ? "欢迎使用诺依曼趋势测试工具"
                                                           : "Welcome to Neumann Trend Test Tool";
        translations["welcome.description"] =
            (language == "zh")
                ? "请在左侧输入数据并运行测试以查看分析结果"
                : "Please enter data on the left and run the test to view analysis results";

        // 占位符文本
        if (language == "zh") {
            translations["input.data.placeholder"] =
                "每行一个数值或CSV格式 (时间,数值)\n例如:\n100\n110\n120\n或\n0,100\n1,110\n2,120";
            translations["input.time.placeholder"] = "每行一个时间值\n例如:\n0\n1\n2\n3";
            translations["data.name.placeholder"] = "输入数据集名称";
        } else {
            translations["input.data.placeholder"] =
                "One value per line or CSV format "
                "(time,value)\nExample:\n100\n110\n120\nor\n0,100\n1,110\n2,120";
            translations["input.time.placeholder"] =
                "One time value per line\nExample:\n0\n1\n2\n3";
            translations["data.name.placeholder"] = "Enter dataset name";
        }

        // 数据管理消息
        translations["message.dataset_name_required"] =
            (language == "zh") ? "请输入数据集名称" : "Please enter dataset name";
        translations["message.data_required"] =
            (language == "zh") ? "请输入数据" : "Please enter data";
        translations["message.dataset_saved"] =
            (language == "zh") ? "数据集保存成功" : "Dataset saved successfully";
        translations["message.save_failed"] = (language == "zh") ? "保存失败" : "Save failed";
        translations["message.select_dataset"] =
            (language == "zh") ? "请选择要加载的数据集" : "Please select a dataset to load";
        translations["message.dataset_loaded"] =
            (language == "zh") ? "加载成功" : "Loaded successfully";
        translations["message.load_failed"] = (language == "zh") ? "加载失败" : "Load failed";
        translations["message.select_dataset_delete"] =
            (language == "zh") ? "请选择要删除的数据集" : "Please select a dataset to delete";
        translations["message.confirm_delete"] =
            (language == "zh") ? "确定要删除数据集" : "Are you sure you want to delete dataset";
        translations["message.delete_warning"] =
            (language == "zh") ? "此操作无法撤销" : "This operation cannot be undone";
        translations["message.dataset_deleted"] =
            (language == "zh") ? "删除成功" : "Deleted successfully";
        translations["message.delete_failed"] = (language == "zh") ? "删除失败" : "Delete failed";

        // 配置消息
        translations["message.config_updated"] =
            (language == "zh") ? "配置已更新" : "Configuration updated";
        translations["message.config_update_failed"] =
            (language == "zh") ? "配置更新失败" : "Configuration update failed";
        translations["message.unknown_error"] = (language == "zh") ? "未知错误" : "Unknown error";
        translations["message.feature_not_implemented"] =
            (language == "zh") ? "功能尚未实现" : "Feature not implemented yet";

        // 样本数据消息
        translations["message.select_sample"] =
            (language == "zh") ? "请选择要加载的样本数据" : "Please select sample data to load";
        translations["message.sample_loaded"] =
            (language == "zh") ? "样本数据加载成功" : "Sample data loaded successfully";

        // 恢复原始语言
        i18n.setLanguage(originalLang);

        json response = {{"success", true}, {"language", language}, {"translations", translations}};
        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("获取翻译时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleStatisticsRequest()
{
    try {
        auto &dataManager = DataManager::getInstance();
        auto datasetNames = dataManager.getDataSetNames();

        int totalDatasets = datasetNames.size();
        int datasetsWithTrend = 0;
        double totalDataPoints = 0;
        double totalPGValue = 0;
        int totalTests = 0;

        for (const auto &name : datasetNames) {
            try {
                auto dataset = dataManager.loadDataSet(name);

                if (dataset.dataPoints.size() >= 4) {
                    NeumannCalculator calculator(0.95);
                    auto results = calculator.performTest(dataset.dataPoints, dataset.timePoints);

                    if (results.overallTrend) {
                        datasetsWithTrend++;
                    }

                    totalDataPoints += dataset.dataPoints.size();
                    totalPGValue += results.avgPG;
                    totalTests++;
                }
            }
            catch (...) {
                // 跳过无法处理的数据集
            }
        }

        json response = {{"success", true},
                         {"statistics",
                          {{"totalDatasets", totalDatasets},
                           {"datasetsWithTrend", datasetsWithTrend},
                           {"avgDataPoints", totalTests > 0 ? totalDataPoints / totalTests : 0},
                           {"avgPGValue", totalTests > 0 ? totalPGValue / totalTests : 0}}}};

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false},
                      {"error", std::string("获取统计信息时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleSampleDataLoadRequest(const std::string &requestBody)
{
    try {
        json request = json::parse(requestBody);
        std::string filename = request["filename"].get<std::string>();
        std::string sampleDir = "data/sample/TestSuite";
        std::string filePath = sampleDir + "/" + filename;

        if (!fs::exists(filePath)) {
            json error = {{"success", false}, {"error", "样本文件不存在"}};
            return error.dump();
        }

        // 使用ExcelReader读取文件
        ExcelReader reader;
        DataSet dataSet;

        try {
            // 直接使用importFromExcel方法，它会自动处理CSV文件
            dataSet = reader.importFromExcel(filePath, "", true);

            if (dataSet.dataPoints.empty()) {
                json error = {{"success", false}, {"error", "无法读取文件数据"}};
                return error.dump();
            }

            // 如果时间点为空，生成默认时间点
            if (dataSet.timePoints.empty()) {
                for (size_t i = 0; i < dataSet.dataPoints.size(); ++i) {
                    dataSet.timePoints.push_back(static_cast<double>(i));
                }
            }
        }
        catch (const std::exception &e) {
            json error = {{"success", false}, {"error", std::string("读取文件失败: ") + e.what()}};
            return error.dump();
        }

        json response = {{"success", true},
                         {"filename", filename},
                         {"data", dataSet.dataPoints},
                         {"time", dataSet.timePoints}};

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false},
                      {"error", std::string("加载样本数据时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleLanguageChangeRequest(const std::string &language)
{
    try {
        auto &i18n = I18n::getInstance();
        auto &config = Config::getInstance();

        Language newLang = (language == "zh") ? Language::CHINESE : Language::ENGLISH;
        i18n.setLanguage(newLang);
        config.setLanguage(newLang);

        bool success = config.saveConfiguration();

        json response = {{"success", success}, {"language", language}};
        if (!success) {
            response["error"] = "保存语言设置失败";
        }

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("设置语言时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleConfidenceLevelChangeRequest(const std::string &level)
{
    try {
        double confidenceLevel = std::stod(level);
        auto &config = Config::getInstance();

        config.setDefaultConfidenceLevel(confidenceLevel);
        bool success = config.saveConfiguration();

        json response = {{"success", success}, {"confidenceLevel", confidenceLevel}};
        if (!success) {
            response["error"] = "保存置信度设置失败";
        }

        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("设置置信度时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleStandardValuesGetRequest()
{
    try {
        auto &standardValues = StandardValues::getInstance();
        auto supportedLevels = standardValues.getSupportedConfidenceLevels();

        json response = {{"success", true}, {"confidenceLevels", supportedLevels}};
        return response.dump();
    }
    catch (const std::exception &e) {
        json error = {{"success", false}, {"error", std::string("获取标准值时出错: ") + e.what()}};
        return error.dump();
    }
}

std::string WebServer::handleCustomConfidenceImportRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleCustomConfidenceListRequest()
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleCustomConfidenceDeleteRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleChartDataRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleExportSVGRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleBatchProcessRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleBatchStatusRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleCSVImportRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

std::string WebServer::handleFileUploadRequest(const std::string & /*requestBody*/)
{
    json response = {{"success", false}, {"error", "功能尚未实现"}};
    return response.dump();
}

}}  // namespace neumann::web