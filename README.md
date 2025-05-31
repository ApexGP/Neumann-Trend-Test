# 诺依曼趋势测试工具 (Neumann Trend Test Tool)

> **Language / 语言**: [English](docs/README/README_en.md) | **中文**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Xyf0606/Neumann-Trend-Test)
[![Version](https://img.shields.io/badge/version-2.9.0-blue.svg)](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

专业的诺依曼趋势测试工具，用于统计分析数据集的系统性趋势，广泛应用于药物稳定性研究和质量控制等领域。

## 📋 目录

- [✨ 功能特性](#-功能特性)
- [🎬 功能预览](#-功能预览)
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
- **批量处理**：目录级批量分析，智能文件分类，双语导出
- **可视化**：ASCII/SVG 图表，结果导出 (CSV/HTML)
- **现代 UI**：彩色终端，状态栏，表格格式化

### 🔧 v2.9.0 新特性

- **Excel 文件支持完善**：真正的 Excel 文件导入功能

  - ✅ 完整支持 .xlsx 文件格式，实现真正的 Excel 文件读取
  - ✅ 基于 ZIP 解压的 Excel 文件解析，支持复杂的 Excel 结构
  - ✅ 智能工作表检测，支持多工作表文件和工作表选择
  - ✅ 自动列类型识别，智能检测时间列和数据列
  - ✅ 共享字符串表支持，正确处理 Excel 中的文本内容
  - ✅ 数据预览功能，在导入前查看文件内容
  - ✅ 完善的错误处理，支持降级到 CSV 格式建议

- **用户体验优化**：Excel 导入流程完全重构

  - ✅ 移除硬编码的"不支持"提示，提供真正的 Excel 支持
  - ✅ 增强的文件格式检测，自动识别 .xlsx、.xls、.csv 格式
  - ✅ 智能数据验证，确保导入数据的完整性和正确性
  - ✅ 友好的错误提示，提供明确的解决建议

- **技术架构提升**：现代化的 Excel 处理引擎

  - ✅ 无需第三方 Excel 库，使用标准 C++ 实现
  - ✅ 跨平台兼容性，支持 Windows、Linux、macOS
  - ✅ 高效的内存管理，支持大型 Excel 文件处理
  - ✅ 临时文件自动清理，避免磁盘空间占用

## 🎬 功能预览

### 🚀 一键启动 - 从零到分析只需 3 秒！

_简单到连你的老板都会用_ 😎

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/startup.mp4" type="video/mp4">
</video>

![启动演示](docs/README/assets/zh_CN/startup.mp4)

<details>
<summary>📋 <strong>启动演示内容说明</strong></summary>

- 双击 `start.bat` 启动应用
- 自动检测配置并初始化
- 选择 Web 界面或 CLI 界面
- Web 界面自动在浏览器中打开

</details>

### 💻 CLI 界面 - 极客的命令行天堂

_分层菜单，一键直达，专业数据分析师的最爱_ ⚡

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/cli-interface.mp4" type="video/mp4">
</video>

![CLI界面](docs/README/assets/zh_CN/cli-interface.mp4)

<details>
<summary>📋 <strong>CLI演示内容说明</strong></summary>

- 彩色终端界面，支持中英文切换
- 实时状态栏显示当前置信度设置
- 表格化结果显示，ASCII 艺术图表
- 快捷键操作，高效数据分析

</details>

### 🌐 Web 界面 - 现代化的分析体验

_颜值即正义，让数据分析变得赏心悦目_ ✨

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/web-interface.mp4" type="video/mp4">
</video>

![Web界面](docs/README/assets/zh_CN/web-interface.mp4)

<details>
<summary>📋 <strong>Web演示内容说明</strong></summary>

- 响应式现代化 UI 设计
- 拖拽上传数据文件
- 实时图表可视化
- 交互式结果展示

</details>

### 📊 数据分析 - 从数据到洞察的魔法

_见证数据说话的神奇时刻_ 🔮

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/data-analysis.mp4" type="video/mp4">
</video>

![数据分析](docs/README/assets/zh_CN/data-analysis.mp4)

<details>
<summary>📋 <strong>分析演示内容说明</strong></summary>

- 导入 CSV 数据文件
- 实时诺伊曼趋势测试计算
- PG 值可视化图表生成
- 趋势判断结果展示

</details>

### ⚡ 批量处理 - 效率狂魔的福音

_一次处理一整个文件夹，让重复工作见鬼去吧_ 🎯

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/batch-processing.mp4" type="video/mp4">
</video>

![批量处理](docs/README/assets/zh_CN/batch-processing.mp4)

<details>
<summary>📋 <strong>批量处理演示内容说明</strong></summary>

- 选择包含多个数据文件的目录
- 并行处理多个数据集
- 进度条实时显示处理状态
- 智能文件分类：CSV 和 HTML 自动保存到专用目录
- 完整双语支持：中英文导出内容完全一致
- UTF-8 编码优化：确保中文在 Excel 中正确显示

</details>

### 🌍 国际化支持 - 全球化的专业工具

_中英文一键切换，国际范儿十足_ 🌏

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/i18n.mp4" type="video/mp4">
</video>

![国际化支持](docs/README/assets/zh_CN/i18n.mp4)

<details>
<summary>📋 <strong>国际化演示内容说明</strong></summary>

- 运行时语言切换（中文 ↔ 英文）
- 所有界面元素完整翻译
- 配置持久化保存
- 无需重启即可生效

</details>

### 🎨 数据可视化 - 让数据跃然纸上

_ASCII 艺术 + SVG 图表，双重视觉盛宴_ 📈

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/visualization.mp4" type="video/mp4">
</video>

![数据可视化](docs/README/assets/zh_CN/visualization.mp4)

<details>
<summary>📋 <strong>可视化演示内容说明</strong></summary>

- CLI 中的 ASCII 艺术图表
- Web 界面的动态 SVG 图表
- 多种图表类型（趋势图、分布图）
- 高质量导出功能

</details>

### ⚙️ 智能配置 - 双端互通

_配置记忆，一劳永逸_ 💝

<video width="100%" height="100%" autoplay loop muted>
  <source src="docs/README/assets/zh_CN/config-management.mp4" type="video/mp4">
</video>

![配置管理](docs/README/assets/zh_CN/config-management.mp4)

<details>
<summary>📋 <strong>配置管理演示内容说明</strong></summary>

- 可视化置信度设置界面
- 智能配置文件管理
- 用户配置与系统配置分离
- 配置导入导出功能

</details>

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

- **CSV 文件**：完全支持，推荐格式，兼容 Excel 导出
- **Excel 文件**：✅ 完整支持 .xlsx 格式，⚠️ .xls 格式建议转换为 .xlsx

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

| 问题               | 解决方案                                  |
| ------------------ | ----------------------------------------- |
| 找不到标准值文件   | 确保 `ref/standard_values.json` 存在      |
| Web 界面无法访问   | 检查端口占用：`netstat -an \| grep 8080`  |
| 配置无法保存       | 检查 `data/usr/` 目录权限                 |
| 彩色输出异常       | 设置 `"enableColorOutput": false`         |
| 置信度显示错误     | 更新到 v2.2.1 版本                        |
| CSV 文件中文乱码   | 使用 v2.8.0 版本，已添加 UTF-8 BOM 支持   |
| 批量处理文件找不到 | 检查 `data/csv/` 和 `data/html/` 目录权限 |
| Excel 文件无法导入 | 使用 v2.9.0 版本，已完整支持 .xlsx 格式   |

### 性能建议

- **大数据集**：使用批量处理模式
- **频繁分析**：保存常用数据集
- **Web 性能**：大数据建议使用 CLI

更多问题请查看 [FAQ](docs/FAQ.md) 或提交 [Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues)。

## �� 版本信息

**当前版本**：v2.9.0  
**发布日期**：2025-01-02

**主要更新**：

- 🔧 **完善 Excel 文件支持**：实现真正的 .xlsx 文件读取功能，支持复杂 Excel 结构
- ✨ **增强用户体验**：移除硬编码限制，提供完整的 Excel 导入流程
- 🛠️ **技术架构提升**：使用现代 C++ 实现跨平台 Excel 处理引擎

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
