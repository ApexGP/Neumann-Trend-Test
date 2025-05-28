# 诺依曼趋势测试工具 (Neumann Trend Test Tool)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Xyf0606/Neumann-Trend-Test)
[![Version](https://img.shields.io/badge/version-2.2.0-blue.svg)](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

诺依曼趋势测试(Neumann Trend Test)是一种统计方法，用于评估数据集是否存在系统性趋势。在药物稳定性研究和质量控制等领域广泛应用。

## 📋 项目概述

本项目提供完整的诺依曼趋势测试功能，包括：

- **🌐 双语支持**：完整的中文/英文界面支持
- **📊 数据管理**：支持 CSV 文件导入，数据集保存和加载
- **🧮 趋势分析**：基于标准 W(P)值表执行诺依曼趋势测试
- **🖥️ 统一界面**：CLI 命令行界面集成 Web 服务器启动功能
- **📈 结果可视化**：ASCII 图表、SVG 图表导出和统计分析
- **⚡ 批量处理**：支持目录批量处理和多格式数据导入
- **🎨 现代 UI**：彩色终端输出、进度指示器和表格格式化
- **⚙️ 智能配置**：置信度可视化设置、状态栏显示和持久化配置

## ✨ 核心功能特点

### 🔬 统计分析能力

- **支持置信度**：95%, 99%, 99.9%
- **样本范围**：4-60 个数据点（扩展支持）
- **多重测试**：从第 4 个数据点开始递增计算
- **趋势判断**：基于标准 W(P)阈值的统计显著性检验

### 📁 数据处理能力

- **格式支持**：CSV 文件（Excel 支持框架已就绪）
- **批量处理**：整个目录的数据文件批量分析
- **数据验证**：自动检测和清理无效数据
- **数据集管理**：保存、加载和管理分析数据集

### 🎯 用户体验

- **双语界面**：中文/英文运行时切换
- **彩色输出**：16 色 ANSI 支持，状态指示清晰
- **进度显示**：批量处理实时进度和加载动画
- **表格格式**：自动列宽调整和颜色编码
- **错误处理**：分类错误代码和友好的解决建议
- **状态栏显示**：实时显示当前置信度水平和应用状态

### 📊 可视化和导出

- **ASCII 图表**：CLI 内实时趋势图显示
- **SVG 导出**：高质量矢量图表生成
- **统计汇总**：多数据集统计分析
- **结果导出**：CSV 和 HTML 格式批量报告

## 🚀 快速开始

### 预编译版本（推荐）

为方便不想自行编译的用户，我提供了预编译的发布版本：

1. **下载预编译版本**

   - 访问[项目发布页](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
   - 下载最新的`NTT-x86_64-windows-v2.2.0.zip`文件
   - ⚠️ 目前不提供`linux`包，请自行构建
   - 解压文件到任意位置

2. **启动应用**

   ```cmd
   # Windows
   start.bat

   # Linux/macOS
   ./start.sh
   ```

   启动后，您可以在 CLI 界面中选择：

   - 使用命令行功能进行数据分析
   - 启动 Web 服务器，通过浏览器访问 Web 界面

3. **系统要求**
   - Windows 7/8/10/11 64 位
   - linux 发行版
   - 无需额外安装依赖（静态链接）

### 发布包内容

```
Neumann-Trend-Test/
├── bin/                    # 可执行文件
│   └── neumann_cli_app.exe # 统一应用程序
├── config/                 # 配置文件
│   ├── config.json         # 应用配置
│   └── translations.json   # 多语言翻译
├── data/
│   ├── sample/             # 示例数据
│   │   ├── TestSuite/      # 测试数据集
│   │   └── ConfidenceLevel/# 自定义置信度示例
│   └── svg/                # svg默认导出路径
│
├── ref/                    # 参考文件
│   ├── standard_values.json                   # 标准值表
│   ├── Table_of_Standard_W(P)_for_Neumann-Trend-Test.md
│   └── Table_of_Standard_W(P)_for_Neumann-Trend-Test.pdf
├── web/                    # Web界面资源
├── start.bat               # Windows启动脚本
├── start.sh                # Linux/macOS启动脚本
└── README.md               # 使用说明
```

## 🛠️ 从源码构建

### 系统要求

- **C++17**兼容的编译器（GCC 7+, Clang 5+, MSVC 2017+）
- **CMake** 3.15+
- **vcpkg**包管理器（Windows 推荐）

### 依赖库

- [nlohmann/json](https://github.com/nlohmann/json) - JSON 处理
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - 终端 UI
- [Crow](https://github.com/CrowCpp/Crow) - Web 服务器
- [Catch2](https://github.com/catchorg/Catch2) - 单元测试（可选）

### Windows 构建

1. **安装 vcpkg**

   ```cmd
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   set VCPKG_ROOT=%CD%
   ```

2. **安装依赖**

   ```cmd
   # MSVC编译器
   vcpkg install nlohmann-json:x64-windows ftxui:x64-windows crow:x64-windows

   # MinGW编译器
   vcpkg install nlohmann-json:x64-mingw-static ftxui:x64-mingw-static crow:x64-mingw-static
   ```

3. **构建项目**

   ```cmd
   # 交互式选择编译器
   build.bat

   # 直接指定编译器
   build.bat mingw release
   build.bat msvc debug
   ```

### Linux/macOS 构建

1. **安装依赖**

   ```bash
   # Ubuntu/Debian
   sudo apt install nlohmann-json3-dev

   # Fedora
   sudo dnf install nlohmann-json-devel

   # macOS
   brew install nlohmann-json
   ```

2. **手动构建 FTXUI 和 Crow**

   ```bash
   # FTXUI
   git clone https://github.com/ArthurSonzogni/FTXUI
   cd FTXUI && mkdir build && cd build
   cmake .. -DFTXUI_BUILD_EXAMPLES=OFF
   make -j$(nproc) && sudo make install

   # Crow
   git clone https://github.com/CrowCpp/Crow
   cd Crow && mkdir build && cd build
   cmake .. && make -j$(nproc) && sudo make install
   ```

3. **构建项目**
   ```bash
   ./build.sh           # Release版本
   ./build.sh debug     # Debug版本
   ./build.sh clean     # 清理重构建
   ```

## 📖 使用指南

### 统一界面快速入门

1. **启动应用**

   ```bash
   # Windows
   start.bat

   # Linux/macOS
   ./start.sh

   # 或直接运行可执行文件
   ./bin/neumann_cli_app.exe
   ```

2. **主要功能**

   - **运行新的诺依曼趋势测试** - 手动输入数据进行测试
   - **从 CSV 导入数据** - 批量导入外部数据文件
   - **加载样本数据** - 使用内置示例数据学习
   - **批量数据处理** - 处理整个目录的数据文件
   - **启动 Web 服务器** - 在 CLI 中启动 Web 界面，通过浏览器访问
   - **高级功能** - 数据可视化和统计分析
   - **设置** - 语言和置信度配置

### v2.2.0 新功能亮点

- **🌐 统一启动方式**：CLI 应用集成 Web 服务器启动功能，简化使用流程
- **🎯 智能置信度配置**：可视化设置界面，支持预设值（90%, 95%, 99%）和自定义输入
- **📊 实时状态栏**：右上角显示当前置信度水平和应用状态
- **⚡ 简化测试流程**：自动使用配置的置信度，减少重复输入
- **🎨 美化结果显示**：改进的边框设计和彩色输出
- **💾 配置持久化**：设置自动保存，重启后保持用户偏好

3. **命令行参数**

   ```bash
   # 直接处理文件
   ./bin/neumann_cli_app.exe -f data/sample/TestSuite/demo.csv

   # 显示帮助
   ./bin/neumann_cli_app.exe --help
   ```

### Web 界面使用

1. **启动 Web 服务器**

   - 在 CLI 主菜单中选择"启动 Web 服务器"
   - 配置端口号（默认 8080）和 Web 资源目录
   - 服务器将在后台运行，您可以继续使用 CLI 功能

2. **访问界面**

   - 打开浏览器访问显示的 URL（如 `http://localhost:8080`）
   - 在线输入数据或加载数据集
   - 实时查看测试结果和图表

3. **管理 Web 服务器**
   - 再次选择"启动 Web 服务器"可以停止当前服务器
   - 服务器在 CLI 应用退出时自动停止

### 批量处理工作流

1. **准备数据目录**

   ```
   data/batch/
   ├── experiment1.csv
   ├── experiment2.csv
   └── experiment3.csv
   ```

2. **运行批量处理**

   - 在 CLI 中选择"批量数据处理"
   - 选择"处理目录中的所有数据文件"
   - 输入目录路径：`data/batch/`
   - 查看处理进度和结果摘要

3. **导出批量结果**
   - 选择导出格式（CSV 或 HTML）
   - 指定输出文件名
   - 获得完整的批量处理报告

## 🔧 配置选项

编辑`config/config.json`文件自定义应用行为：

```json
{
  "language": "zh", // 界面语言: "zh"(中文) 或 "en"(英文)
  "defaultConfidenceLevel": 0.95, // 默认置信度: 0.90, 0.95, 0.99 或自定义值
  "dataDirectory": "data", // 数据目录路径
  "webRootDirectory": "web", // Web资源目录
  "defaultWebPort": 8080, // Web服务器默认端口
  "showWelcomeMessage": true, // 是否显示欢迎消息
  "enableColorOutput": true, // 是否启用彩色输出
  "maxDataPoints": 1000, // 最大数据点数限制
  "autoSaveResults": true // 是否自动保存结果
}
```

### 置信度配置说明

v2.1.0 版本新增了可视化的置信度配置界面：

- **预设值选择**：90%, 95%, 99% 三个常用置信度水平
- **自定义输入**：支持 0.0-1.0 范围内的任意置信度值
- **默认推荐**：95% 置信度被标记为推荐默认值
- **实时显示**：当前置信度在状态栏中实时显示
- **持久保存**：设置会自动保存到配置文件中

## 📊 数据格式要求

### CSV 文件格式

```csv
时间,数值
0,99.95
1,99.88
2,99.82
...
```

### 数据要求

- **最少数据点**：4 个（诺依曼测试最低要求）
- **最多数据点**：60 个（基于标准值表）
- **数据类型**：数值型，支持整数和小数
- **时间点**：可选，未提供时使用默认递增序列

## 🎯 技术特性

### 架构设计

- **模块化设计**：核心算法、CLI 界面、Web 服务分离
- **单例模式**：配置管理、国际化、错误处理全局一致
- **RAII 模式**：自动资源管理，内存安全

### 性能优化

- **静态链接**：无运行时依赖，单文件部署
- **高效算法**：O(n²)时间复杂度的 PG 值计算
- **内存优化**：数据流式处理，支持大型数据集

### 跨平台兼容

- **Windows**：原生 ANSI 颜色支持，UTF-8 编码
- **Linux/macOS**：完整 POSIX 兼容，终端颜色自适应
- **Web 标准**：HTML5 + JavaScript，现代浏览器支持

## 🐛 故障排除

### 常见问题

1. **编译错误：找不到头文件**

   ```bash
   # 检查vcpkg安装
   vcpkg list
   # 确保设置了VCPKG_ROOT环境变量
   echo $VCPKG_ROOT
   ```

2. **运行时错误：标准值文件未找到**

   ```bash
   # 确保ref/standard_values.json存在
   ls ref/standard_values.json
   # 检查工作目录是否正确
   pwd
   ```

3. **Web 界面无法访问**

   ```bash
   # 检查端口是否被占用
   netstat -an | grep 8080
   # 尝试使用不同端口
   ./bin/neumann_web_app.exe --port 8081
   ```

4. **彩色输出不显示**

   ```bash
   # Windows: 确保使用Windows 10+的现代终端
   # 或在config.json中设置 "enableColorOutput": false
   ```

5. **置信度设置无法保存**

   ```bash
   # 确保config目录存在且有写入权限
   ls -la config/
   # 检查配置文件是否可写
   ls -la config/config.json
   ```

6. **状态栏显示异常**
   ```bash
   # 检查终端宽度设置
   echo $COLUMNS  # Linux/macOS
   # 或尝试调整终端窗口大小
   ```

### 性能建议

- **大数据集（>40 数据点）**：使用批量处理模式
- **频繁测试**：保存常用数据集避免重复输入
- **Web 界面**：大数据集建议使用 CLI 获得更好性能

## 🤝 贡献指南

欢迎提交问题报告和功能请求！

1. **报告 Bug**：[GitHub Issues](https://github.com/Xyf0606/Neumann-Trend-Test/issues)
2. **功能请求**：描述期望功能和使用场景
3. **代码贡献**：Fork 项目，创建 feature 分支，提交 Pull Request

### 开发环境设置

```bash
# 克隆仓库
git clone https://github.com/Xyf0606/Neumann-Trend-Test.git
cd Neumann-Trend-Test

# 安装开发依赖
./check-deps.bat  # Windows
./build.sh        # Linux/macOS

# 运行测试
ctest --preset mingw-debug
```

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE) - 详情请查看 LICENSE 文件。

## 🙏 致谢

- [诺依曼趋势测试方法](https://en.wikipedia.org/wiki/Neumann_trend_test) 的统计学基础
- [nlohmann/json](https://github.com/nlohmann/json) 提供的优秀 JSON 库
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) 现代化的终端 UI 框架
- [Crow](https://github.com/CrowCpp/Crow) 轻量级的 C++ Web 框架

---

**📧 联系方式**：如有问题或建议，请通过[GitHub Issues](https://github.com/Xyf0606/Neumann-Trend-Test/issues)联系我。
