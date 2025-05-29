# 构建指南

本文档提供详细的项目构建说明，适用于不同平台和编译器。

## 📋 目录

- [系统要求](#系统要求)
- [依赖安装](#依赖安装)
- [Windows 构建](#windows-构建)
- [Linux 构建](#linux-构建)
- [macOS 构建](#macos-构建)
- [构建选项](#构建选项)
- [故障排除](#故障排除)

## 系统要求

### 编译器支持

- **GCC**: 7.0+ (支持 C++17)
- **Clang**: 5.0+ (支持 C++17)
- **MSVC**: 2017+ (Visual Studio 15.7+)
- **MinGW**: 7.0+ (推荐 MinGW-w64)

### 构建工具

- **CMake**: 3.15+
- **Ninja**: 推荐（可选，提升构建速度）
- **vcpkg**: Windows 平台强烈推荐

## 依赖安装

### 核心依赖库

- [nlohmann/json](https://github.com/nlohmann/json) - JSON 处理
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - 终端 UI
- [Crow](https://github.com/CrowCpp/Crow) - Web 服务器

### 可选依赖

- [Catch2](https://github.com/catchorg/Catch2) - 单元测试框架

## Windows 构建

### 方法一：使用 vcpkg（推荐）

1. **安装 vcpkg**

   ```cmd
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   set VCPKG_ROOT=C:\vcpkg
   ```

2. **安装依赖包**

   ```cmd
   # MSVC 编译器
   vcpkg install nlohmann-json:x64-windows ftxui:x64-windows crow:x64-windows

   # MinGW 编译器
   vcpkg install nlohmann-json:x64-mingw-static ftxui:x64-mingw-static crow:x64-mingw-static
   ```

3. **构建项目**

   ```cmd
   # 检查依赖
   check-deps.bat --toolchain mingw

   # 构建项目
   build.bat mingw release
   build.bat msvc debug
   ```

### 方法二：手动安装依赖

1. **下载预编译库**

   - 从各项目的 Release 页面下载预编译版本
   - 或使用包管理器如 Conan

2. **设置环境变量**

   ```cmd
   set CMAKE_PREFIX_PATH=C:\path\to\libraries
   ```

3. **构建**
   ```cmd
   cmake --preset mingw-release
   cmake --build --preset mingw-release
   ```

## Linux 构建

### Ubuntu/Debian

1. **安装系统依赖**

   ```bash
   sudo apt update
   sudo apt install build-essential cmake ninja-build
   sudo apt install nlohmann-json3-dev
   ```

2. **手动构建 FTXUI**

   ```bash
   git clone https://github.com/ArthurSonzogni/FTXUI.git
   cd FTXUI
   mkdir build && cd build
   cmake .. -DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_DOCS=OFF
   make -j$(nproc)
   sudo make install
   ```

3. **手动构建 Crow**

   ```bash
   git clone https://github.com/CrowCpp/Crow.git
   cd Crow
   mkdir build && cd build
   cmake .. -DCROW_BUILD_EXAMPLES=OFF
   make -j$(nproc)
   sudo make install
   ```

4. **构建项目**
   ```bash
   ./build.sh
   ./build.sh debug    # Debug 版本
   ```

### Fedora/CentOS

```bash
# Fedora
sudo dnf install gcc-c++ cmake ninja-build nlohmann-json-devel

# CentOS/RHEL (需要 EPEL)
sudo yum install epel-release
sudo yum install gcc-c++ cmake3 ninja-build
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake ninja nlohmann-json
```

## macOS 构建

### 使用 Homebrew

1. **安装依赖**

   ```bash
   brew install cmake ninja nlohmann-json
   ```

2. **手动构建其他依赖**

   ```bash
   # FTXUI
   git clone https://github.com/ArthurSonzogni/FTXUI.git
   cd FTXUI && mkdir build && cd build
   cmake .. -DFTXUI_BUILD_EXAMPLES=OFF
   make -j$(sysctl -n hw.ncpu) && sudo make install

   # Crow
   git clone https://github.com/CrowCpp/Crow.git
   cd Crow && mkdir build && cd build
   cmake .. && make -j$(sysctl -n hw.ncpu) && sudo make install
   ```

3. **构建项目**
   ```bash
   ./build.sh
   ```

## 构建选项

### CMake 预设

项目提供了多个 CMake 预设，位于 `CMakePresets.json`：

- `mingw-release` - MinGW Release 版本
- `mingw-debug` - MinGW Debug 版本
- `msvc-release` - MSVC Release 版本
- `msvc-debug` - MSVC Debug 版本
- `linux-release` - Linux Release 版本
- `linux-debug` - Linux Debug 版本

### 自定义构建

```bash
# 手动配置
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

# 构建
cmake --build . --parallel

# 安装
cmake --build . --target install
```

### 构建选项说明

| 选项                | 说明            | 默认值       |
| ------------------- | --------------- | ------------ |
| `CMAKE_BUILD_TYPE`  | 构建类型        | Release      |
| `BUILD_TESTING`     | 构建测试        | ON           |
| `ENABLE_WEB_SERVER` | 启用 Web 服务器 | ON           |
| `STATIC_LINKING`    | 静态链接        | ON (Windows) |

## 故障排除

### 常见编译错误

1. **找不到依赖库**

   ```
   错误：Could not find package nlohmann_json
   解决：确保已正确安装依赖或设置 CMAKE_PREFIX_PATH
   ```

2. **C++17 支持错误**

   ```
   错误：This file requires compiler and library support for C++17
   解决：升级编译器版本或检查 CMake 设置
   ```

3. **vcpkg 集成问题**
   ```
   错误：vcpkg toolchain file not found
   解决：设置 VCPKG_ROOT 环境变量或手动指定工具链文件
   ```

### 性能优化

1. **使用 Ninja 构建器**

   ```bash
   cmake -G Ninja ..
   ```

2. **并行构建**

   ```bash
   cmake --build . --parallel $(nproc)
   ```

3. **使用 ccache（Linux/macOS）**
   ```bash
   sudo apt install ccache  # Ubuntu
   export CC="ccache gcc"
   export CXX="ccache g++"
   ```

### 调试构建问题

1. **详细输出**

   ```bash
   cmake --build . --verbose
   ```

2. **检查依赖**

   ```bash
   ./check-deps.bat --debug    # Windows
   ldd bin/neumann_cli_app     # Linux
   otool -L bin/neumann_cli_app # macOS
   ```

3. **CMake 调试**
   ```bash
   cmake .. --debug-output
   ```

## 贡献者构建

### 开发环境设置

1. **克隆仓库**

   ```bash
   git clone https://github.com/Xyf0606/Neumann-Trend-Test.git
   cd Neumann-Trend-Test
   ```

2. **安装 Git hooks**

   ```bash
   cp tools/pre-commit .git/hooks/
   chmod +x .git/hooks/pre-commit
   ```

3. **配置开发构建**
   ```bash
   cmake --preset mingw-debug
   cmake --build --preset mingw-debug
   ```

### 代码格式化

项目使用 clang-format 进行代码格式化：

```bash
# 格式化所有文件
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# 检查格式
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
```

### 运行测试

```bash
# 构建测试
cmake --build . --target tests

# 运行测试
ctest --preset mingw-debug --verbose
```

---

如有其他构建问题，请提交 [Issue](https://github.com/Xyf0606/Neumann-Trend-Test/issues)。
