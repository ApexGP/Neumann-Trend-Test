@echo off
echo ===== 诺依曼趋势测试工具构建系统 =====
echo.

:: 初始化默认值
set CLEAN_BUILD=0
set VERBOSE=0
set TOOL_CHAIN=
set BUILD_TYPE=release
set SKIP_INTERACTIVE=0

:: 处理命令行参数
:parse_args
if "%1"=="" goto end_parse_args

if /i "%1"=="clean" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)

if /i "%1"=="verbose" (
    set VERBOSE=1
    shift
    goto parse_args
)

if /i "%1"=="msvc" (
    set TOOL_CHAIN=msvc
    set SKIP_INTERACTIVE=1
    shift
    goto parse_args
)

if /i "%1"=="mingw" (
    set TOOL_CHAIN=mingw
    set SKIP_INTERACTIVE=1
    shift
    goto parse_args
)

if /i "%1"=="debug" (
    set BUILD_TYPE=debug
    shift
    goto parse_args
)

if /i "%1"=="release" (
    set BUILD_TYPE=release
    shift
    goto parse_args
)

echo [警告] 未知参数: %1
shift
goto parse_args

:end_parse_args

:: 判断是否需要进行工具链交互式选择
if "%TOOL_CHAIN%"=="" (
    if "%SKIP_INTERACTIVE%"=="0" (
        goto interactive_select
    )
)

goto start_build

:: 交互式选择构建工具链
:interactive_select
echo 请选择构建工具链:
echo 1) MSVC (Visual Studio)
echo 2) MinGW (GCC)
echo.
set /p CHOICE="请输入你的选择 (1/2): "

if "%CHOICE%"=="1" (
    set TOOL_CHAIN=msvc
    echo 你选择了MSVC工具链
) else if "%CHOICE%"=="2" (
    set TOOL_CHAIN=mingw
    echo 你选择了MinGW工具链
) else (
    echo [错误] 无效的选择: %CHOICE%
    exit /b 1
)

echo.

:: 检查对应编译器是否可用
if "%TOOL_CHAIN%"=="msvc" (
    where cl.exe >nul 2>&1
    if errorlevel 1 (
        echo [错误] 未找到MSVC编译器(cl.exe)
        echo 请安装Visual Studio或确保已在开发者命令提示符中运行此脚本
        exit /b 1
    )
) else if "%TOOL_CHAIN%"=="mingw" (
    where gcc.exe >nul 2>&1
    if errorlevel 1 (
        echo [错误] 未找到MinGW编译器(gcc.exe)
        echo 请安装MinGW或确保将其bin目录添加到PATH环境变量中
        exit /b 1
    )
)

:start_build
:: 设置CMake预设名称
set PRESET=%TOOL_CHAIN%-%BUILD_TYPE%

:: 输出配置信息
echo.
echo 构建配置:
echo - 工具链: %TOOL_CHAIN%
echo - 构建类型: %BUILD_TYPE%
echo - CMake预设: %PRESET%

:: 检查依赖 (单独调用依赖检查脚本)
echo 检查依赖项...
call check-deps.bat --quiet --toolchain %TOOL_CHAIN%
if errorlevel 1 (
    echo.
    echo [错误] 依赖检查失败，请安装缺少的依赖项后再试
    exit /b 1
)
echo [成功] 所有依赖检查通过

:: 如果需要清理构建目录
if "%CLEAN_BUILD%"=="1" (
    echo 清理构建目录: build/%PRESET%
    if exist "build/%PRESET%" rmdir /s /q "build/%PRESET%"
)

:: 创建构建目录
if not exist "build/%PRESET%" mkdir "build/%PRESET%"

:: 检查 Ninja 是否可用 (对MinGW构建有帮助)
if "%TOOL_CHAIN%"=="mingw" (
    where ninja.exe >nul 2>&1
    if errorlevel 1 (
        echo [警告] 未找到Ninja构建工具，构建可能较慢
        echo 建议通过以下方式安装Ninja:
        echo   scoop install ninja
        echo   或
        echo   choco install ninja
    )
)

:: 配置项目
echo.
echo 配置项目...
if "%VERBOSE%"=="1" (
    cmake --preset=%PRESET% --log-level=VERBOSE
) else (
    cmake --preset=%PRESET%
)

if errorlevel 1 (
    echo.
    echo [错误] 配置失败
    exit /b 1
)

:: 构建项目
echo.
echo 编译项目...
if "%VERBOSE%"=="1" (
    cmake --build --preset=%PRESET% --verbose
) else (
    cmake --build --preset=%PRESET%
)

if errorlevel 1 (
    echo.
    echo [错误] 构建失败
    exit /b 1
)

:: 运行测试
echo.
echo 运行测试...
ctest --preset=%PRESET%

:: 显示成功信息
echo.
echo 构建完成。可执行文件位于 bin 目录中。
echo - neumann_cli_app.exe: 命令行界面应用
echo - neumann_web_app.exe: Web界面应用
echo.
echo 用法:
echo - build.bat                     : 交互式选择构建工具链 (默认Release)
echo - build.bat mingw               : 使用MinGW构建Release版本
echo - build.bat msvc                : 使用MSVC构建Release版本
echo - build.bat [toolchain] debug   : 构建Debug版本
echo - build.bat clean [其他选项]     : 清理后重新构建
echo - build.bat verbose [其他选项]   : 显示详细构建信息
echo. 