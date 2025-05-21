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

## 下载预编译版本

对于不想自行构建的用户，可以直接下载预编译的发布版：

1. 访问 [项目发布页](https://github.com/Xyf0606/Neumann-Trend-Test/releases)
2. 下载最新的 `NeumannTrendTest-v1.2.0.zip` 文件
3. 解压后运行 `start-web-app.bat` 即可使用

## 从源码编译 | Build from source

本项目依赖以下库：

- nlohmann/json: 用于 JSON 处理
- FTXUI: 用于终端 UI
- Crow: 用于 Web 服务器
- Catch2: 用于单元测试（可选，仅测试需要）

### Windows 平台依赖安装

Windows 平台**强烈推荐**使用 vcpkg 包管理器安装所有依赖：

1. **安装 vcpkg**

   ```cmd
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. **设置环境变量（可选）**

   推荐设置 `VCPKG_ROOT` 环境变量指向 vcpkg 安装目录，但现在构建脚本能够自动搜索常见安装位置：

   ```cmd
   # 临时设置（当前命令行会话）
   set VCPKG_ROOT=C:\path\to\vcpkg

   # 或永久设置（通过系统属性）
   setx VCPKG_ROOT C:\path\to\vcpkg
   ```

   > **注意**：如果不设置环境变量，构建系统会自动尝试在常见位置查找 vcpkg，但为了稳定性，仍建议手动设置。

3. **安装依赖**

   **使用 MSVC 编译器时**：

   ```cmd
   vcpkg install nlohmann-json:x64-windows
   vcpkg install ftxui:x64-windows
   vcpkg install crow:x64-windows
   vcpkg install catch2:x64-windows
   ```

   **使用 MinGW 编译器时**：

   ```cmd
   vcpkg install nlohmann-json:x64-mingw-static
   vcpkg install ftxui:x64-mingw-static
   vcpkg install crow:x64-mingw-static
   vcpkg install catch2:x64-mingw-static
   ```

### Linux 平台依赖安装

1. **安装 nlohmann/json**

   Ubuntu/Debian:

   ```bash
   sudo apt install nlohmann-json3-dev
   ```

   Fedora:

   ```bash
   sudo dnf install nlohmann-json-devel
   ```

   Arch Linux:

   ```bash
   sudo pacman -S nlohmann-json
   ```

2. **安装 FTXUI**

   FTXUI 通常需要手动构建：

   ```bash
   git clone https://github.com/ArthurSonzogni/FTXUI
   cd FTXUI
   mkdir build && cd build
   cmake .. -DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_DOCS=OFF
   make -j$(nproc)
   sudo make install
   ```

3. **安装 Crow**

   Crow 通常需要手动构建：

   ```bash
   git clone https://github.com/CrowCpp/Crow
   cd Crow
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   sudo make install
   ```

4. **安装 Catch2** (可选)

   Ubuntu/Debian:

   ```bash
   sudo apt install catch2
   ```

   Fedora:

   ```bash
   sudo dnf install catch-devel
   ```

   Arch Linux:

   ```bash
   sudo pacman -S catch2
   ```

   或手动构建：

   ```bash
   git clone https://github.com/catchorg/Catch2.git
   cd Catch2
   mkdir build && cd build
   cmake .. -DBUILD_TESTING=OFF
   make -j$(nproc)
   sudo make install
   ```

### macOS 平台依赖安装

使用 Homebrew 安装：

```bash
brew install nlohmann-json
brew install catch2
# FTXUI 和 Crow 需手动构建，方法同 Linux
```

## 构建说明

确保已安装所有依赖库后，可以进行项目构建。

### Windows

项目在 Windows 平台上支持两种编译器：MSVC（Visual Studio）和 MinGW（GCC）。
构建脚本会提供交互式选择，或者可以通过参数直接指定编译器。

```cmd
# 交互式选择构建工具链（推荐初次构建时使用）
build.bat

# 使用MinGW编译器构建Release版本
build.bat mingw

# 使用Visual Studio (MSVC)编译器构建Release版本
build.bat msvc

# 构建Debug版本
build.bat mingw debug
build.bat msvc debug

# 清理后重新构建
build.bat clean mingw
```

### Linux/macOS

```bash
# 构建Release版本
./build.sh

# 构建Debug版本
./build.sh debug

# 清理后重新构建
./build.sh clean
```

### 检查依赖项

如果构建失败，可以使用依赖检查工具查看依赖项是否正确安装：

```cmd
# Windows
check-deps.bat

# 只检查特定工具链的依赖
check-deps.bat --toolchain msvc
check-deps.bat --toolchain mingw

# 启用调试模式显示更多信息
check-deps.bat --debug
```

这将显示系统中的依赖项安装状态和环境配置。检查工具现在能够自动搜索多个位置查找 vcpkg 和相关依赖库。

### 高级构建配置

项目使用 CMake 预设(CMake Presets)进行构建配置，详细预设配置可在`CMakePresets.json`文件中查看。对于更高级的控制，可以直接使用 CMake 命令：

```bash
# 手动配置
cmake --preset=mingw-release

# 手动构建
cmake --build --preset=mingw-release
```

## 最新构建系统改进

本项目最近对构建系统进行了以下改进：

1. **智能依赖项检测**：

   - 自动搜索多个位置查找 vcpkg，即使环境变量未设置
   - 提供更详细的错误信息和安装建议
   - 智能检测特定工具链所需的 triplet
   - 添加`--debug`模式显示详细信息

2. **工具链集成**：

   - 更好地支持 MSVC 和 MinGW 工具链切换
   - 优化 Ninja 构建系统集成
   - 自动检测并适应不同环境

3. **构建脚本优化**：

   - 交互式工具链选择
   - 命令行参数支持
   - 直观的构建状态反馈
   - 输出结构优化，二进制文件统一存放在根目录的 bin 文件夹

4. **CMake 配置改进**：

   - 更灵活的 vcpkg 工具链集成
   - 添加额外的库路径查找逻辑
   - 改进 triplet 配置
   - 统一输出目录结构，便于直接调用生成的程序

5. **开发工具集成**：
   - 优化 clangd 配置，提供更准确的代码智能提示
   - 自动配置索引路径，支持多种构建目录结构
   - 改进跨平台 IDE 支持，统一开发体验
   - VSCode 集成增强，提供开箱即用的开发环境

这些改进使得项目更容易在不同环境下构建，特别是对于 Windows 用户来说，不再需要手动修改构建文件或环境变量来适应不同的工具链。

## IDE 开发环境设置

本项目提供了开箱即用的 IDE 开发环境配置，特别优化了 VS Code 和其他支持 clangd 的编辑器：

### VS Code 配置

项目已预配置 VS Code 开发环境：

1. **安装推荐扩展**：

   - C/C++ Extension Pack
   - clangd (推荐使用 clangd 而非原生 C/C++扩展提供智能提示)

2. **clangd 配置**：

   - 项目根目录的`.clangd`文件已优化配置，提供准确的代码导航和补全
   - 自动适配不同构建环境(MSVC/MinGW)的包含路径
   - 支持跨平台开发环境

3. **使用方法**：
   - 首次打开项目时，VS Code 会提示安装推荐扩展
   - 运行构建脚本生成`compile_commands.json`
   - clangd 会自动索引项目，提供代码导航和智能提示

### 其他编辑器

对于其他支持 clangd 的编辑器(如 CLion、Vim、Emacs)：

1. 确保编辑器已配置使用 clangd 语言服务器
2. 运行构建脚本生成`compile_commands.json`
3. 使用项目根目录的`.clangd`配置文件

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

> **注意**: 所有可执行文件现在都会直接生成在项目根目录的 bin 文件夹中，无需再从 build 目录寻找。

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
