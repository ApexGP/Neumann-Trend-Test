#include "core/neumann_calculator.h"
#include "core/standard_values.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

using namespace neumann;

TEST_CASE("Standard W(P) values are loaded correctly", "[standard_values]") {
  auto &standard_values = StandardValues::getInstance();

  SECTION("Default confidence level (0.95)") {
    REQUIRE(standard_values.getWPValue(4) == Catch::Approx(0.7805));
    REQUIRE(standard_values.getWPValue(5) == Catch::Approx(0.8204));
    REQUIRE(standard_values.getWPValue(6) == Catch::Approx(0.8902));
    REQUIRE(standard_values.getWPValue(7) == Catch::Approx(0.9359));
  }

  SECTION("Custom confidence level (0.99)") {
    REQUIRE(standard_values.getWPValue(4, 0.99) == Catch::Approx(0.9027));
    REQUIRE(standard_values.getWPValue(5, 0.99) == Catch::Approx(0.9282));
  }

  SECTION("Invalid sample size") {
    REQUIRE(standard_values.getWPValue(3) == -1.0);
    REQUIRE(standard_values.getWPValue(25) == -1.0);
  }
}

TEST_CASE("Neumann trend test calculation", "[neumann_calculator]") {
  NeumannCalculator calculator;

  SECTION("Simple increasing trend") {
    std::vector<double> data = {100, 110, 120, 130, 140, 150};
    auto results = calculator.performTest(data);

    // 从第四个数据点开始判断趋势
    REQUIRE(results.results.size() == 3);

    // 确保趋势判断符合预期
    REQUIRE(results.overallTrend);

    // PG值应该很小，小于标准W(P)值
    REQUIRE(results.results[0].pgValue < results.results[0].wpThreshold);
  }

  SECTION("No trend (random data)") {
    std::vector<double> data = {100, 105, 102, 108, 103, 106};
    auto results = calculator.performTest(data);

    // 从第四个数据点开始判断趋势
    REQUIRE(results.results.size() == 3);

    // 检查是否未发现趋势
    REQUIRE_FALSE(results.overallTrend);

    // PG值应该较大，大于标准W(P)值
    REQUIRE(results.results[0].pgValue > results.results[0].wpThreshold);
  }

  SECTION("Custom time points") {
    std::vector<double> data = {100, 110, 120, 130, 140, 150};
    std::vector<double> timePoints = {0, 2, 5, 10, 15, 20};

    auto results = calculator.performTest(data, timePoints);

    // 检查时间点是否被正确保存
    REQUIRE(results.timePoints == timePoints);
  }
}

TEST_CASE("Edge cases for Neumann calculator", "[neumann_calculator]") {
  NeumannCalculator calculator;

  SECTION("Less than 4 data points") {
    std::vector<double> data = {100, 110, 120};
    auto results = calculator.performTest(data);

    // 结果中不应有任何测试点
    REQUIRE(results.results.empty());
  }

  SECTION("Different confidence levels") {
    std::vector<double> data = {100, 110, 120, 130, 140, 150};

    // 默认置信水平 (0.95)
    auto results1 = calculator.performTest(data);

    // 更高置信水平 (0.99)
    calculator.setConfidenceLevel(0.99);
    auto results2 = calculator.performTest(data);

    // 更高置信水平的W(P)值应该更大
    REQUIRE(results2.results[0].wpThreshold > results1.results[0].wpThreshold);
  }
}