# 诺依曼趋势测试工具 (Neumann Trend Test Tool)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Xyf0606/Neumann-Trend-Test)
[![Version](https://img.shields.io/badge/version-2.2.1-blue.svg)](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

诺依曼趋势测试(Neumann Trend Test)是一种统计方法，用于评估数据集是否存在系统性趋势。在药物稳定性研究和质量控制等领域广泛应用。

## 📋 项目概述

本项目提供完整的诺依曼趋势测试功能，包括：

- **🌐 双语支持**：完整的中文/英文界面支持，智能配置管理
- **📊 数据管理**：支持 CSV 文件导入，数据集保存和加载
- **🧮 趋势分析**：基于标准 W(P)值表执行诺依曼趋势测试
- **🖥️ 统一界面**：CLI 命令行界面集成 Web 服务器启动功能
- **📈 结果可视化**：ASCII 图表、SVG 图表导出和统计分析
- **⚡ 批量处理**：支持目录批量处理和多格式数据导入
- **🎨 现代 UI**：彩色终端输出、进度指示器和表格格式化
- **⚙️ 智能配置**：置信度可视化设置、状态栏显示和持久化配置
- **🔧 智能管理**：用户配置隔离、权限友好、自动文件迁移

## ✨ 核心功能特点

### 🔬 统计分析能力

- **支持置信度**：95%, 99%, 99.9%
- **样本范围**：4-60 个数据点（扩展支持）
- **多重测试**：从第 4 个数据点开始递增计算
- **趋势判断**：基于标准 W(P)阈值的统计显著性检验
- **精确显示**：置信度显示精度修复，正确显示小数位数

### 📁 数据处理能力

- **格式支持**：CSV 文件（Excel 支持框架已就绪）
- **批量处理**：整个目录的数据文件批量分析
- **数据验证**：自动检测和清理无效数据
- **数据集管理**：保存、加载和管理分析数据集

### 🎯 用户体验

- **双语界面**：中文/英文运行时切换，完整国际化支持
- **彩色输出**：16 色 ANSI 支持，状态指示清晰
- **进度显示**：批量处理实时进度和加载动画
- **表格格式**：自动列宽调整和颜色编码
- **错误处理**：分类错误代码和友好的解决建议
- **状态栏显示**：实时显示当前置信度水平和应用状态
- **智能启动**：翻译系统优先初始化，确保启动消息正确显示

### 🔧 智能配置管理（v2.2.1 新特性）

- **配置隔离**：用户私有配置与系统默认配置完全分离
- **智能加载**：用户私有 > 系统默认 > 开发环境 > 内置默认的优先级
- **权限友好**：无权限时自动降级到只读模式，不影响使用
- **自动迁移**：标准值文件从旧位置自动迁移到用户目录
- **文件管理**：用户可定制文件集中到 `data/usr/` 目录管理

### 🌍 完整国际化（v2.2.1 新特性）

- **全模块支持**：所有核心模块完全支持多语言
- **翻译完整性**：标准值管理、配置系统、错误处理等模块无硬编码
- **格式统一**：错误消息、成功提示、警告信息统一使用翻译键
- **运行时切换**：支持应用运行时动态切换语言

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
   - 下载最新的`NTT-x86_64-windows`压缩包
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
├── bin/                               # 可执行文件
│   ├── neumann_cli_app.exe            # CLI端程序
│   └── neumann_web_app.exe            # Web端程序
├── lib/                               # 运行库文件
│
├── config/                            # 配置文件
│   ├── config.json                    # 应用配置
│   └── translations.json              # 多语言翻译
├── data/
│   ├── usr/                           # 用户系统文件目录
│   │   ├── config_private.json        # 用户私有配置文件
│   │   └── standard_values.json       # 用户标准值文件
│   ├── sample/                        # 示例数据
│   │   ├── TestSuite/                 # 测试数据集
│   │   └── ConfidenceLevel/           # 自定义置信度示例
│   ├── svg/                           # SVG导出目录
│   │
│   ├── user_test1.json                # 用户测试数据
│   └── user_test2.json                # 用户测试数据
├── web/                               # Web端资源文件
│
├── ref/
│   └── standard_values.json           # 系统默认标准值
├── start.bat                          # 启动脚本
└── README.md                          # 使用说明
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
- **🧮 智能趋势判断**：新增药物稳定性专用的趋势检测算法，更准确地判断末端连续趋势
- **🔧 界面优化**：修复菜单对齐问题，完善双语支持，提升用户体验

### v2.2.1 新功能亮点

- **🔧 智能配置管理**：用户私有配置与系统默认配置完全分离，权限友好
- **🌍 完整国际化**：标准值模块完全翻译，消除所有硬编码文本
- **🐛 精度修复**：置信度显示精度修复，正确显示小数位数（如 0.990 而不是 0.9）
- **🚀 启动优化**：翻译系统优先初始化，确保启动消息正确显示
- **📁 文件管理**：用户可定制文件集中到 `data/usr/` 目录管理
- **🔄 自动迁移**：标准值文件从旧位置自动迁移到用户目录

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

### 智能配置管理系统

v2.2.1 版本引入了智能配置管理系统，提供更好的用户配置隔离和权限管理：

#### 配置文件层次结构

1. **用户私有配置** (`data/usr/config_private.json`) - 最高优先级

   - 用户个人设置，优先加载
   - 自动从系统配置创建
   - 支持用户自定义修改

2. **系统默认配置** (`config/config.json`) - 备用配置

   - 软件默认设置
   - 权限不足时的只读模式
   - 软件更新时保持不变

3. **开发环境配置** (`config/config.json`) - 开发模式
   - 开发时的临时配置
   - 仅在开发环境中使用

#### 智能加载流程

```
启动应用 → 检查用户私有配置 → 存在且有效 → 加载成功 ✓
         ↓
         检查系统默认配置 → 存在 → 创建用户配置 → 加载成功 ✓
         ↓                      ↓
         检查开发配置 → 加载 ✓    创建失败 → 只读模式 ⚠️
         ↓
         使用内置默认 ⚠️
```

#### 配置权限管理

- **可写模式**：用户私有配置文件，支持保存设置
- **只读模式**：系统配置文件，设置修改不会保存
- **权限检测**：自动检测用户权限，优雅降级

#### 配置目录结构

```
data/
├── usr/                           # 用户系统文件目录
│   ├── config_private.json        # 用户私有配置文件
│   └── standard_values.json       # 用户标准值文件
│
├── sample/                        # 示例数据
│   ├── TestSuite/                 # 测试数据集
│   └── ConfidenceLevel/           # 自定义置信度示例
├── svg/                           # SVG导出目录
├── user_test1.json                # 用户测试数据
└── user_test2.json                # 用户测试数据

config/
├── config.json                    # 系统默认配置
└── translations.json              # 用户翻译文件

ref/
└── standard_values.json           # 系统默认标准值
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
- **智能趋势检测**：v2.2.0 新增末端连续趋势点检测算法，专为药物稳定性研究优化
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

7. **趋势判断结果异常**

   ```bash
   # v2.2.0版本采用了新的智能趋势判断算法
   # 如果发现结果与预期不符，请检查：
   # 1. 数据是否为药物稳定性相关的时序数据
   # 2. 数据点数量是否足够（建议≥4个点）
   # 3. 是否存在末端连续的异常值
   ```

8. **翻译文件损坏**

   ```bash
   # 如果界面显示翻译键而非实际文本
   # 检查翻译文件是否正确
   ls -la config/translations.json
   # 重新下载或重置翻译文件
   ```

9. **配置文件权限问题（v2.2.1）**

   ```bash
   # 用户私有配置无法创建或保存
   # 检查用户目录权限
   ls -la data/usr/
   # 确保有足够的磁盘空间和写入权限
   # 应用会自动降级到只读模式并提示用户
   ```

10. **置信度显示精度异常（v2.2.1 已修复）**

    ```bash
    # 如果置信度显示为 0.9 而不是 0.990
    # 请更新到 v2.2.1 版本，该问题已修复
    # 或检查是否使用了旧版本的显示格式
    ```

11. **标准值模块翻译显示异常（v2.2.1 已修复）**

    ```bash
    # 如果自定义置信度导入过程中显示硬编码中文
    # 请更新到 v2.2.1 版本，已完整国际化
    # 检查语言设置是否正确生效
    ```

12. **程序启动时部分信息显示英文（v2.2.1 已修复）**

    ```bash
    # 如果程序启动时配置管理消息显示英文
    # v2.2.1 已调整启动顺序，翻译系统优先初始化
    # 重新启动应用即可看到正确的翻译文本
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

## 📦 版本信息

**当前版本**：v2.2.1

**主要更新**：

- 🔧 智能配置管理系统（用户配置隔离、权限友好）
- 🌍 完整国际化支持（标准值模块完全翻译）
- 🐛 置信度显示精度修复
- 🚀 程序启动翻译系统优化

**完整更新历史**：请查看 [CHANGELOG.md](CHANGELOG.md) 获取详细的版本更新记录。

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE) - 详情请查看 LICENSE 文件。

## 🙏 致谢

- [诺依曼趋势测试方法](https://en.wikipedia.org/wiki/Neumann_trend_test) 的统计学基础
- [nlohmann/json](https://github.com/nlohmann/json) 提供的优秀 JSON 库
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) 现代化的终端 UI 框架
- [Crow](https://github.com/CrowCpp/Crow) 轻量级的 C++ Web 框架

---

**📧 联系方式**：如有问题或建议，请通过[GitHub Issues](https://github.com/Xyf0606/Neumann-Trend-Test/issues)联系我。
