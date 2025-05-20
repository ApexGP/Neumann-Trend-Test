@echo off
echo ===== 构建诺依曼趋势测试工具 =====

:: 设置默认构建工具链
set BUILD_SYSTEM=Visual Studio 17 2022
set BUILD_ARCH=-A x64
set USE_MINGW=0
set CONFIG=Release
set CLEAN_BUILD=0
set USE_VCPKG=0
set INSTALL_DEPS=0

:: 检查命令行参数
if "%1"=="clean" (
    set CLEAN_BUILD=1
    echo 清理构建目录...
    if exist "build" rmdir /s /q build
    shift
)

if "%1"=="mingw" (
    set BUILD_SYSTEM=MinGW Makefiles
    set BUILD_ARCH=
    set USE_MINGW=1
    set CONFIG=
    echo 使用MinGW工具链构建...
    set NEUMANN_USE_MINGW=1
    shift
) else (
    echo 使用MSVC工具链构建...
)

:: 检查是否使用vcpkg和是否安装依赖
if "%1"=="vcpkg" (
    set USE_VCPKG=1
    echo 使用vcpkg管理依赖...
    shift
) else if "%1"=="install-deps" (
    set INSTALL_DEPS=1
    echo 将安装项目依赖...
    shift
)

:: 如果指定安装依赖或使用vcpkg且安装依赖
if "%INSTALL_DEPS%"=="1" (
    :: 检查VCPKG_ROOT环境变量
    if defined VCPKG_ROOT (
        echo 使用vcpkg安装依赖...
        echo 正在安装FTXUI库...
        call "%VCPKG_ROOT%\vcpkg" install ftxui:x64-windows
        if errorlevel 1 (
            echo 警告: FTXUI安装失败，将尝试从GitHub下载。
        ) else (
            echo FTXUI安装成功。
        )
        
        echo 正在安装Crow库...
        call "%VCPKG_ROOT%\vcpkg" install crow:x64-windows
        if errorlevel 1 (
            echo 警告: Crow安装失败，将尝试从GitHub下载。
        ) else (
            echo Crow安装成功。
        )
    ) else (
        echo 错误: 未设置VCPKG_ROOT环境变量，无法安装依赖。
        echo 请设置VCPKG_ROOT环境变量或手动安装依赖。
        exit /b 1
    )
)

if not exist "build" mkdir build
cd build

:: 构建命令
set CMAKE_CMD=cmake .. -G "%BUILD_SYSTEM%" %BUILD_ARCH%

:: 如果使用vcpkg，添加vcpkg工具链文件
if "%USE_VCPKG%"=="1" (
    :: 检查VCPKG_ROOT环境变量
    if defined VCPKG_ROOT (
        echo 使用VCPKG_ROOT环境变量: %VCPKG_ROOT%
        set CMAKE_CMD=%CMAKE_CMD% -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
        
        :: 如果启用vcpkg但没有显式安装依赖，自动安装
        if "%INSTALL_DEPS%"=="0" (
            echo 检查是否需要安装依赖...
            call "%VCPKG_ROOT%\vcpkg" list | findstr "ftxui" > nul
            if errorlevel 1 (
                echo FTXUI未安装，正在安装...
                call "%VCPKG_ROOT%\vcpkg" install ftxui:x64-windows
            ) else (
                echo FTXUI已安装。
            )
            
            call "%VCPKG_ROOT%\vcpkg" list | findstr "crow" > nul
            if errorlevel 1 (
                echo Crow未安装，正在安装...
                call "%VCPKG_ROOT%\vcpkg" install crow:x64-windows
            ) else (
                echo Crow已安装。
            )
        )
    ) else (
        echo 警告: 未找到VCPKG_ROOT环境变量，请确保已正确安装vcpkg或手动安装依赖
    )
)

echo 配置项目...
%CMAKE_CMD% %~1 %~2 %~3 %~4

echo 编译项目...
cmake --build . --config Release

echo 运行测试...
ctest -C Release

echo.
if "%USE_MINGW%"=="1" (
    echo 构建完成。可执行文件位于 build\bin 目录中。
) else (
    echo 构建完成。可执行文件位于 build\bin\%CONFIG% 目录中。
)
echo - neumann_cli_app.exe: 命令行界面应用
echo - neumann_web_app.exe: Web界面应用
echo.
echo 用法:
echo - build.bat        : 使用MSVC构建
echo - build.bat mingw  : 使用MinGW构建
echo - build.bat vcpkg  : 使用MSVC构建，通过vcpkg管理依赖
echo - build.bat mingw vcpkg : 使用MinGW构建，通过vcpkg管理依赖
echo - build.bat install-deps : 安装项目依赖
echo - build.bat clean [mingw] [vcpkg]: 清理后重新构建
echo.

cd ..