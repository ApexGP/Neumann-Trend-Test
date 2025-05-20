# 诺依曼趋势测试工具 (Neumann Trend Test Tool)

诺依曼趋势测试(Neumann Trend Test)是一种统计方法，用于评估数据集是否存在系统性趋势。在药物稳定性研究和质量控制等领域广泛应用。

## 项目概述

本项目提供完整的诺依曼趋势测试功能，包括：

- 数据管理：支持从 CSV 文件导入数据，保存和加载数据集
- 趋势分析：基于标准 W(P)值表执行诺依曼趋势测试
- 双重界面：同时提供命令行界面和 Web 界面
- 结果可视化：以表格和图表形式展示测试结果

## 功能特点

- 支持多种置信水平(0.90, 0.95, 0.975, 0.99)
- 支持样本大小从 4 到 20 的范围
- 可导入 CSV 数据文件进行批量处理
- 可视化结果展示趋势判断
- 支持导出结果到文本或 CSV 文件

## 构建说明

### 依赖项

- C++17 兼容的编译器
- CMake 3.10 或更高版本
- 以下依赖库会自动下载（如果未安装）：
  - nlohmann/json: 用于 JSON 处理
  - Crow: 用于 Web 服务器
  - FTXUI: 用于终端 UI
  - Catch2: 用于单元测试

### 构建步骤

#### Windows

Windows 平台提供两种构建方式：使用 MSVC 或 MinGW。

```cmd
# 使用 MSVC 构建 (默认)
build.bat

# 使用 MinGW 构建
build.bat mingw
```

#### Linux/MacOS

```bash
# 创建构建目录并构建项目
./build.sh
```

### 高级构建配置

#### 自定义 MinGW 工具链

在 Windows 上使用 MinGW 构建时，系统会按以下顺序查找 MinGW 安装路径：

1. 环境变量 `MINGW_ROOT`
2. Scoop 安装路径 (`%SCOOP%/apps/mingw/current`)
3. 常见安装路径 (`C:/MinGW`, `C:/msys64/mingw64`, 等)

您也可以手动指定 MinGW 工具链配置：

```cmd
# 使用自定义 MinGW 工具链文件
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake ..
```

#### 配置 clangd 语言服务器

项目会自动生成 `compile_commands.json` 文件，支持 clangd 语言服务器。在 VSCode 中，您可以：

1. 安装 clangd 扩展
2. 确保 `build` 目录中的 `compile_commands.json` 文件存在
3. 若使用 MinGW，请在 `settings.json` 中添加:

```json
{
  "clangd.arguments": [
    "--background-index",
    "--compile-commands-dir=${workspaceFolder}/build",
    "--query-driver=<MinGW路径>/bin/g++.exe"
  ]
}
```

## 使用说明

### 命令行界面

```bash
# 启动交互式终端界面
./bin/neumann_cli_app

# 直接处理CSV文件
./bin/neumann_cli_app -f data.csv

# 查看帮助
./bin/neumann_cli_app -h
```

### Web 界面

```bash
# 启动Web服务器（默认端口8080）
./bin/neumann_web_app

# 指定端口和Web资源目录
./bin/neumann_web_app -p 3000 -d /path/to/web

# 查看帮助
./bin/neumann_web_app -h
```

启动 Web 服务器后，在浏览器中访问 http://localhost:8080 使用 Web 界面。

## 数据格式

CSV 数据文件格式示例:

```
Time,Value
0,6147338
1,6147785
2,6154157
5,6141375
10,6127575
18,6118354
24,6090916
```

## 参考资料

标准 W(P)值表参考文献:

- Sachs, L. (1984)
- Neumann, J., et al. "Distribution of the ratio of the mean square successive difference to the variance." The Annals of Mathematical Statistics, 1941.

## 许可证

MIT License
