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
- [静态编译配置](#静态编译配置)

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

## 静态编译配置

从 v2.3.0 开始，项目默认启用静态编译，彻底解决 Windows 运行库依赖问题。

### MinGW 静态链接配置

#### 自动配置（推荐）

项目的 CMakeLists.txt 已自动配置 MinGW 静态链接：

```cmake
# MinGW特定的链接选项（已在项目中配置）
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -Wl,-Bstatic -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# 强制静态链接所有库
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(BUILD_SHARED_LIBS OFF)
```

#### 使用 vcpkg 静态三元组

确保使用静态链接的 vcpkg 三元组：

```bash
# 安装静态库依赖
vcpkg install nlohmann-json:x64-mingw-static ftxui:x64-mingw-static crow:x64-mingw-static

# 或设置环境变量
set VCPKG_DEFAULT_TRIPLET=x64-mingw-static
```

### 验证静态编译

#### Windows 验证方法

1. **使用 dumpbin 检查依赖**（需要 Visual Studio）：

   ```cmd
   dumpbin /dependents bin\neumann_cli_app.exe
   ```

2. **使用 objdump 检查**（MinGW 环境）：

   ```bash
   objdump -p bin/neumann_cli_app.exe | grep "DLL Name"
   ```

3. **期望的依赖结果**：

   ```
   正确的静态编译应只显示系统库：
   ✅ KERNEL32.dll
   ✅ api-ms-win-crt-*.dll
   ✅ WS2_32.dll
   ✅ WSOCK32.dll

   不应出现这些依赖：
   ❌ libwinpthread-1.dll
   ❌ libgcc_s_seh-1.dll
   ❌ libstdc++-6.dll
   ```

#### Linux 验证方法

```bash
# 检查动态库依赖
ldd bin/neumann_cli_app

# 静态编译应显示最少依赖
```

### 故障排除

#### 常见静态链接问题

1. **pthread 库仍为动态链接**

   ```cmake
   # 解决方案：确保使用正确的pthread链接标志
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -pthread -Wl,-Bstatic")
   ```

2. **vcpkg 三元组错误**

   ```bash
   # 检查当前三元组
   vcpkg list

   # 重新安装正确的静态三元组
   vcpkg remove nlohmann-json ftxui crow --triplet x64-mingw-dynamic
   vcpkg install nlohmann-json:x64-mingw-static ftxui:x64-mingw-static crow:x64-mingw-static
   ```

3. **链接顺序问题**
   ```cmake
   # 确保在app/CMakeLists.txt中有正确的库链接顺序
   target_link_libraries(neumann_cli_app
       neumann_cli
       neumann_core
       ${STATIC_LIBRARIES}
       ws2_32 wsock32 winpthread  # Windows静态系统库
   )
   ```

#### 高级静态链接技巧

1. **完全静态链接**（消除所有动态依赖）：

   ```cmake
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")
   ```

2. **选择性静态链接**：

   ```cmake
   # 只静态链接特定库
   target_link_libraries(target_name
       -static-libgcc
       -static-libstdc++
       -Wl,-Bstatic -lpthread
       -Wl,-Bdynamic -lkernel32
   )
   ```

3. **检查未解决的符号**：

   ```bash
   # MinGW
   nm -u bin/neumann_cli_app.exe | grep " U "

   # 或使用objdump
   objdump -t bin/neumann_cli_app.exe | grep "*UND*"
   ```

### 不同平台的静态编译说明

#### Windows (MinGW)

- 默认启用完全静态链接
- 生成的可执行文件可在任何 Windows 机器运行
- 无需安装 MinGW 运行时

#### Windows (MSVC)

- 默认使用静态运行时链接
- 可能仍需要 Visual C++运行时
- 建议使用 MinGW 进行真正的静态编译

#### Linux

- 静态链接 glibc 可能导致兼容性问题
- 推荐只静态链接第三方库
- 保持系统库动态链接

#### macOS

- 系统限制静态链接系统库
- 只能静态链接第三方依赖
- 使用 bundle 方式分发依赖

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
