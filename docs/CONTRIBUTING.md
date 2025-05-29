# 贡献指南 (Contributing Guide)

感谢您对诺依曼趋势测试工具的关注！我们欢迎各种形式的贡献，包括代码、文档、bug 报告和功能建议。

## 📋 目录

- [如何贡献](#如何贡献)
- [开发环境设置](#开发环境设置)
- [代码贡献流程](#代码贡献流程)
- [编码规范](#编码规范)
- [测试要求](#测试要求)
- [文档贡献](#文档贡献)
- [问题报告](#问题报告)

## 如何贡献

### 🐛 报告问题

如果您发现了 bug 或有改进建议：

1. **检查现有 Issues**：避免重复报告
2. **使用 Issue 模板**：提供详细信息
3. **包含重现步骤**：帮助我们快速定位问题
4. **提供环境信息**：操作系统、版本等

### 💡 功能建议

对于新功能或改进建议：

1. **查看项目路线图**：了解当前开发方向
2. **在 Discussions 中讨论**：获得社区反馈
3. **详细描述用例**：说明功能的实际应用场景
4. **考虑实现复杂度**：评估开发工作量

### 📝 文档改进

文档贡献同样重要：

- 修正错误或不准确的信息
- 改进表达和可读性
- 添加使用示例和最佳实践
- 翻译文档（目前支持中文和英文）

## 开发环境设置

### 前置条件

- **Git**: 版本控制
- **C++17 编译器**: GCC 7+, Clang 5+, MSVC 2017+
- **CMake**: 3.15+
- **vcpkg**: Windows 平台推荐

### 克隆仓库

```bash
git clone https://github.com/Xyf0606/Neumann-Trend-Test.git
cd Neumann-Trend-Test
```

### 安装依赖

**Windows (vcpkg):**

```cmd
vcpkg install nlohmann-json:x64-windows ftxui:x64-windows crow:x64-windows
```

**Linux (Ubuntu):**

```bash
sudo apt install build-essential cmake nlohmann-json3-dev
# 手动构建 FTXUI 和 Crow，参考 docs/BUILD.md
```

**macOS (Homebrew):**

```bash
brew install cmake nlohmann-json
# 手动构建 FTXUI 和 Crow，参考 docs/BUILD.md
```

### 构建项目

```bash
# Debug 版本（开发推荐）
cmake --preset mingw-debug    # Windows MinGW
cmake --preset linux-debug    # Linux
cmake --build --preset mingw-debug

# 运行测试
ctest --preset mingw-debug --verbose
```

### 开发工具配置

**VSCode 设置：**

- 项目已包含 `.vscode/` 配置
- 安装推荐插件：C/C++, CMake Tools, clang-format

**clangd 配置：**

- 使用项目根目录的 `.clangd` 配置
- 自动生成 `compile_commands.json`

## 代码贡献流程

### 1. Fork 和分支

```bash
# Fork 仓库到您的 GitHub 账户
# 然后克隆您的 fork

git clone https://github.com/YOUR_USERNAME/Neumann-Trend-Test.git
cd Neumann-Trend-Test

# 添加上游仓库
git remote add upstream https://github.com/Xyf0606/Neumann-Trend-Test.git

# 创建功能分支
git checkout -b feature/your-feature-name
git checkout -b bugfix/issue-number
```

### 2. 开发过程

**提交指南：**

- 使用清晰的提交信息
- 每个提交解决一个问题
- 提交前运行测试和代码格式化

**提交信息格式：**

```
type(scope): brief description

Detailed explanation if needed

Fixes #issue_number
```

**类型说明：**

- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档更新
- `style`: 代码格式化
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建和工具

**示例：**

```
feat(core): add custom confidence level validation

Add comprehensive validation for user-imported confidence levels,
including range checks and W(P) value reasonableness tests.

Fixes #42
```

### 3. 代码质量检查

**运行测试：**

```bash
# 单元测试
ctest --preset mingw-debug

# 特定测试
ctest --preset mingw-debug -R neumann_test
```

**代码格式化：**

```bash
# 格式化所有文件
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# 检查格式
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
```

**静态分析（可选）：**

```bash
# 使用 clang-tidy
clang-tidy src/**/*.cpp -- -Iinclude -std=c++17
```

### 4. 提交 Pull Request

**PR 标题格式：**

```
[Type] Brief description of changes
```

**PR 描述模板：**

```markdown
## 变更概述

简要描述您的更改内容

## 变更类型

- [ ] Bug 修复
- [ ] 新功能
- [ ] 文档更新
- [ ] 性能优化
- [ ] 代码重构

## 测试

- [ ] 添加了新的测试用例
- [ ] 现有测试通过
- [ ] 手动测试完成

## 相关 Issue

Fixes #issue_number

## 检查清单

- [ ] 代码遵循项目规范
- [ ] 提交信息清晰明确
- [ ] 文档已更新（如需要）
- [ ] 测试覆盖新代码
```

## 编码规范

### C++ 编码标准

**命名约定：**

```cpp
// 类名：PascalCase
class NeumannCalculator {};

// 函数/变量：camelCase
double calculatePGValue();
int sampleSize;

// 常量：UPPER_CASE
const int MAX_SAMPLE_SIZE = 60;

// 命名空间：小写
namespace neumann::core {}
```

**代码风格：**

- 使用 4 空格缩进（不使用 Tab）
- 行长度限制 100 字符
- 使用 `.clang-format` 配置
- 头文件使用 `#pragma once`

**现代 C++ 特性：**

```cpp
// 使用 auto 进行类型推导
auto result = calculatePGValue(data);

// 使用智能指针
std::unique_ptr<Calculator> calc = std::make_unique<Calculator>();

// 使用范围 for 循环
for (const auto& value : dataPoints) {
    // ...
}

// 使用 nullptr 而不是 NULL
Calculator* calc = nullptr;
```

### 文件组织

**目录结构：**

```
include/
  ├── core/          # 核心功能头文件
  ├── cli/           # CLI 相关头文件
  └── web/           # Web 服务器头文件
src/
  ├── core/          # 核心功能实现
  ├── cli/           # CLI 实现
  └── web/           # Web 服务器实现
tests/               # 测试文件
docs/                # 文档
```

**头文件结构：**

```cpp
#pragma once

// 系统头文件
#include <vector>
#include <string>

// 第三方库
#include <nlohmann/json.hpp>

// 项目头文件
#include "core/i18n.h"

namespace neumann {

class MyClass {
    // 公共接口在前
public:
    MyClass();

    // 私有成员在后
private:
    void privateMethod();
    int memberVariable;
};

}  // namespace neumann
```

## 测试要求

### 单元测试

使用 Catch2 框架编写测试：

```cpp
#include <catch2/catch_test_macros.hpp>
#include "core/neumann_calculator.h"

TEST_CASE("NeumannCalculator basic functionality", "[neumann]") {
    neumann::NeumannCalculator calc(0.95);

    SECTION("handles minimum data points") {
        std::vector<double> data = {1.0, 2.0, 3.0, 4.0};
        auto results = calc.performTest(data);

        REQUIRE(results.results.size() == 1);
        REQUIRE(results.data.size() == 4);
    }

    SECTION("detects trend correctly") {
        std::vector<double> trendData = {10.0, 9.0, 8.0, 7.0, 6.0};
        auto results = calc.performTest(trendData);

        REQUIRE(results.overallTrend == true);
    }
}
```

**测试覆盖要求：**

- 新功能必须有对应测试
- 测试覆盖率目标 ≥ 80%
- 包含边界条件测试
- 包含错误处理测试

### 集成测试

测试完整的工作流程：

```cpp
TEST_CASE("Full workflow integration", "[integration]") {
    // 数据导入 → 计算 → 结果验证
}
```

## 文档贡献

### 文档类型

1. **代码文档**：函数和类的注释
2. **用户文档**：README、FAQ、使用指南
3. **开发文档**：BUILD.md、CONTRIBUTING.md
4. **API 文档**：自动生成（Doxygen）

### 文档规范

**代码注释：**

```cpp
/**
 * @brief 计算诺依曼趋势测试的 PG 值
 *
 * 基于输入数据计算 PG 统计量，用于判断是否存在显著趋势。
 *
 * @param data 输入数据点向量
 * @param endIndex 计算终止位置（包含）
 * @return 计算得到的 PG 值
 *
 * @throws std::invalid_argument 如果数据点不足或索引超出范围
 *
 * @see performTest() 完整的趋势测试流程
 * @since v2.0.0
 */
double calculatePG(const std::vector<double>& data, size_t endIndex);
```

**Markdown 文档：**

- 使用清晰的标题层次
- 包含代码示例
- 添加目录导航
- 保持更新和准确性

## 问题报告

### Bug 报告模板

```markdown
## Bug 描述

简要描述遇到的问题

## 重现步骤

1. 启动程序
2. 选择...
3. 输入...
4. 看到错误

## 预期行为

描述您期望发生的情况

## 实际行为

描述实际发生的情况

## 环境信息

- 操作系统：Windows 10 / Ubuntu 20.04 / macOS 11
- 程序版本：v2.2.1
- 编译器：MinGW 8.1.0 / GCC 9.4.0
- 其他相关信息

## 错误日志
```

粘贴相关错误信息

```

## 附加信息
任何其他有助于问题解决的信息
```

### 功能请求模板

```markdown
## 功能描述

清晰描述您希望添加的功能

## 用例场景

说明这个功能的实际应用场景

## 建议的解决方案

描述您认为应该如何实现

## 可选方案

描述您考虑过的其他解决方案

## 额外信息

任何其他相关信息
```

## 发布流程

### 版本号规则

遵循[语义化版本控制](https://semver.org/)：

- `MAJOR.MINOR.PATCH`
- 不兼容的 API 修改：增加 MAJOR
- 向下兼容的功能性新增：增加 MINOR
- 向下兼容的问题修正：增加 PATCH

### 发布检查清单

- [ ] 所有测试通过
- [ ] 文档更新
- [ ] CHANGELOG.md 更新
- [ ] 版本号更新
- [ ] 创建发布包
- [ ] GitHub Release

---

## 联系方式

- **GitHub Issues**: 技术问题和 bug 报告
- **GitHub Discussions**: 功能讨论和社区交流
- **Email**: 维护者邮箱（紧急事项）

感谢您的贡献！您的参与让这个项目变得更好。🎉
