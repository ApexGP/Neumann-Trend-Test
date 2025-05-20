@echo off
echo ===== 调试构建 - 诺依曼趋势测试工具 =====

:: 清理构建目录
echo 清理构建目录...
if exist "build" rmdir /s /q build
mkdir build
cd build

:: 设置CMake参数
set CMAKE_ARGS=-G "MinGW Makefiles" 

:: 如果设置了VCPKG_ROOT，添加工具链文件
if defined VCPKG_ROOT (
    echo 使用VCPKG_ROOT: %VCPKG_ROOT%
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
)

:: 调试选项
set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON --log-level=VERBOSE

echo 运行CMake配置...
echo cmake .. %CMAKE_ARGS%
cmake .. %CMAKE_ARGS%

if errorlevel 1 (
    echo CMake配置失败，错误代码: %errorlevel%
    exit /b %errorlevel%
)

echo.
echo 正在编译 - 详细模式...
cmake --build . --verbose

if errorlevel 1 (
    echo 编译失败，错误代码: %errorlevel%
    exit /b %errorlevel%
)

echo.
echo 调试构建完成。
cd .. 