# 诺依曼趋势测试工具 (Neumann Trend Test Tool)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Xyf0606/Neumann-Trend-Test)
[![Version](https://img.shields.io/badge/version-2.7.1-blue.svg)](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

专业的诺依曼趋势测试工具，用于统计分析数据集的系统性趋势，广泛应用于药物稳定性研究和质量控制等领域。

## 📋 目录

- [✨ 功能特性](#-功能特性)
- [🚀 快速开始](#-快速开始)
- [📖 使用指南](#-使用指南)
- [🛠️ 从源码构建](#️-从源码构建)
- [🔧 配置说明](#-配置说明)
- [📊 数据格式](#-数据格式)
- [🐛 故障排除](#-故障排除)
- [📦 版本信息](#-版本信息)
- [🤝 贡献指南](#-贡献指南)
- [📄 许可证](#-许可证)

## ✨ 功能特性

### 🔬 核心功能

- **统计分析**：支持 95%, 99%, 99.9% 置信度，4-60 个数据点
- **双语界面**：完整的中文/英文支持，运行时切换
- **多平台**：Windows, Linux, macOS 原生支持
- **双界面**：命令行 + Web 界面，满足不同使用需求

### 🎯 用户体验

- **智能配置**：用户配置隔离，权限友好管理
- **批量处理**：目录级批量分析，进度实时显示
- **可视化**：ASCII/SVG 图表，结果导出 (CSV/HTML)
- **现代 UI**：彩色终端，状态栏，表格格式化

### 🔧 v2.7.1 新特性

- **国际化系统完善**：Web 服务器启动模块全面国际化

  - ✅ 彻底消除 `web_main.cpp` 中的硬编码文本
  - ✅ 统一 Web 和 CLI 模块的翻译机制
  - ✅ 完善 `getTextf` 模板函数支持
  - ✅ 新增 36 个翻译键，覆盖服务器启动全流程

- **模板系统优化**：修复格式化文本处理机制

  - ✅ 解决 `getTextf` 模板函数链接错误
  - ✅ 添加 `std::string` 非引用版本模板实例化
  - ✅ 支持类型安全的多参数格式化
  - ✅ 避免手动字符串拼接，重用现有代码

### 🔧 v2.3.0 新特性

- **静态编译优化**：彻底解决动态库依赖问题

  - ✅ 消除 `libwinpthread-1.dll` 依赖
  - ✅ 完全静态链接 MinGW 运行时
  - ✅ 可在任何 Windows 机器上运行
  - ✅ 无需额外安装任何运行库

- **国际化系统完善**：修复 Web 界面翻译显示问题

  - ✅ 消除所有硬编码中英文文本
  - ✅ 修复翻译键映射错误（如 `result.test_results`）
  - ✅ 新增 75+ 个翻译键，覆盖所有功能模块
  - ✅ 支持实时语言切换，优化翻译 API 性能

- **用户体验优化**：智能启动流程，降低使用门槛
  - ✅ CLI 应用自动启动 Web 服务器并打开浏览器
  - ✅ 提供 Web/CLI 界面选择，默认推荐 Web 界面
  - ✅ 优化用户引导流程，改进错误处理
  - ✅ 清晰的失败反馈和解决建议

## 🚀 快速开始

### 方法一：下载预编译版本（推荐）

1. **下载发布包**

   - 访问[Releases 页面](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
   - 下载最新的`NTT-x86_64-windows`压缩包
   - ⚠️ 目前不提供`linux`包，请自行构建
   - 解压文件到任意位置

2. **启动应用**

   ```bash
   # Windows
   start.bat

   # Linux/macOS
   ./start.sh
   ```

3. **开始使用**
   - CLI 界面：直接进行数据分析
   - Web 界面：在 CLI 中选择"启动 Web 服务器"

### 方法二：Docker（即将支持）

```bash
docker run -p 8080:8080 neumann-trend-test:latest
```

## 📖 使用指南

### 基本工作流程

1. **数据准备**

   ```csv
   时间,数值
   0,99.95
   1,99.88
   2,99.82
   3,99.76
   ```

2. **运行分析**

   - **交互式**：启动应用 → 选择"运行新的诺依曼趋势测试"
   - **批量处理**：选择"批量数据处理" → 指定目录
   - **命令行**：`./bin/neumann_cli_app.exe -f data.csv`

3. **查看结果**
   - 趋势判断：是否存在显著趋势
   - PG 值分析：每个测试点的统计值
   - 可视化图表：趋势图和分布图

### Web 界面使用

1. 在 CLI 中选择"启动 Web 服务器"
2. 浏览器访问 `http://localhost:8080`
3. 上传数据文件或手动输入数据
4. 实时查看分析结果和图表

### 配置管理

- **置信度设置**：设置 → 置信度配置 → 选择或自定义
- **语言切换**：设置 → 语言设置 → 中文/English
- **配置文件**：自动保存到 `data/usr/config_private.json`

## 🛠️ 从源码构建

### 系统要求

- C++17 兼容编译器
- CMake 3.15+
- vcpkg (Windows 推荐)

### Windows 构建

```bash
# 1. 安装依赖
vcpkg install nlohmann-json:x64-windows ftxui:x64-windows crow:x64-windows

# 2. 构建项目
build.bat mingw release    # MinGW
build.bat msvc release     # MSVC
```

### Linux/macOS 构建

```bash
# 1. 安装依赖
sudo apt install nlohmann-json3-dev    # Ubuntu
brew install nlohmann-json             # macOS

# 2. 手动构建FTXUI和Crow（参考官方文档）

# 3. 构建项目
./build.sh
```

详细构建说明请参考 [构建文档](docs/BUILD.md)。

## 🔧 配置说明

### 配置文件位置

- **用户配置**：`data/usr/config_private.json` （优先）
- **系统配置**：`config/config.json` （备用）

### 主要配置项

```json
{
  "language": "zh", // 界面语言
  "defaultConfidenceLevel": 0.95, // 默认置信度
  "dataDirectory": "data", // 数据目录
  "defaultWebPort": 8080, // Web端口
  "enableColorOutput": true, // 彩色输出
  "maxDataPoints": 1000 // 数据点限制
}
```

### 智能配置系统

- **配置隔离**：用户配置与系统配置分离
- **权限检测**：无权限时自动降级到只读模式
- **自动迁移**：旧版本配置自动升级

## 📊 数据格式

### 支持格式

- **CSV 文件**：推荐格式，兼容 Excel 导出
- **Excel 文件**：框架已就绪，即将支持

### 数据要求

- **最少数据点**：4 个（统计检验最低要求）
- **最多数据点**：60 个（基于标准值表覆盖范围）
- **数据类型**：数值型（整数/小数）
- **时间列**：可选，默认使用递增序列

### 示例数据

```csv
时间,药物含量
0,100.00
3,99.85
6,99.72
9,99.58
12,99.43
```

## 🐛 故障排除

### 常见问题

| 问题             | 解决方案                                 |
| ---------------- | ---------------------------------------- |
| 找不到标准值文件 | 确保 `ref/standard_values.json` 存在     |
| Web 界面无法访问 | 检查端口占用：`netstat -an \| grep 8080` |
| 配置无法保存     | 检查 `data/usr/` 目录权限                |
| 彩色输出异常     | 设置 `"enableColorOutput": false`        |
| 置信度显示错误   | 更新到 v2.2.1 版本                       |

### 性能建议

- **大数据集**：使用批量处理模式
- **频繁分析**：保存常用数据集
- **Web 性能**：大数据建议使用 CLI

更多问题请查看 [FAQ](docs/FAQ.md) 或提交 [Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues)。

## 📦 版本信息

**当前版本**：v2.7.1  
**发布日期**：2025-05-30

**主要更新**：

- 🌍 完善国际化系统，消除 Web 服务器启动模块硬编码文本
- ✨ 优化 getTextf 模板函数，修复链接错误和类型支持
- 📝 新增 36 个翻译键，统一 Web 和 CLI 模块翻译机制
- 🔧 提升代码质量，避免重复实现，重用现有格式化系统

**更新历史**：[CHANGELOG.md](CHANGELOG.md)

## 🤝 贡献指南

我欢迎各种形式的贡献！

### 如何贡献

1. **报告问题**：[提交 Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues)
2. **功能建议**：描述需求和使用场景
3. **代码贡献**：Fork → 开发 → 测试 → Pull Request

### 开发环境

```bash
git clone https://github.com/Xyf0606/Neumann-Trend-Test.git
cd Neumann-Trend-Test
./build.sh debug
ctest --preset linux-debug
```

### 贡献规范

- 遵循现有代码风格
- 添加适当的测试用例
- 更新相关文档
- 确保 CI 通过

详细指南：[CONTRIBUTING.md](docs/CONTRIBUTING.md)

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE)。

---

<div align="center">

**⭐ 如果这个项目对您有帮助，请给我一个 Star！⭐**

[🏠 主页](https://github.com/Xyf0606/Neumann-Trend-Test) •
[📖 文档](docs/) •
[🐛 报告问题](https://github.com/Xyf0606/Neumann-Trend-Test/issues) •
[💬 讨论](https://github.com/Xyf0606/Neumann-Trend-Test/discussions)

</div>
