@echo off
setlocal EnableDelayedExpansion

:: 初始化变量
set QUIET_MODE=0
set TOOLCHAIN=both
set ERROR_FOUND=0
set VCPKG_FOUND=0
set VCPKG_EXE=
set DEBUG_MODE=0

:: 处理命令行参数
:parse_args
if "%1"=="" goto end_parse_args

if /i "%1"=="--quiet" (
    set QUIET_MODE=1
    shift
    goto parse_args
)

if /i "%1"=="--debug" (
    set DEBUG_MODE=1
    shift
    goto parse_args
)

if /i "%1"=="--toolchain" (
    if "%2"=="msvc" (
        set TOOLCHAIN=msvc
    ) else if "%2"=="mingw" (
        set TOOLCHAIN=mingw
    ) else (
        echo [错误] 无效的工具链: %2
        exit /b 1
    )
    shift
    shift
    goto parse_args
)

echo [警告] 未知参数: %1
shift
goto parse_args
:end_parse_args

if %QUIET_MODE% EQU 0 (
    echo ===== 诺依曼趋势测试工具 - 依赖检查工具 =====
    echo.
    echo 检查模式: !TOOLCHAIN!
    if %DEBUG_MODE% EQU 1 (
        echo [调试模式已启用]
    )
    echo.
)

:: 检查基本环境
if %QUIET_MODE% EQU 0 (
    echo 系统信息:
    echo -------------------------------------
    echo 操作系统: %OS%
    systeminfo | findstr /B /C:"OS Version"
    echo.
)

:: 检查CMake
if %QUIET_MODE% EQU 0 ( echo 检查CMake... )
cmake --version >nul 2>&1
if errorlevel 1 (
    echo [错误] 未找到CMake，请安装CMake并确保其在PATH中
    set ERROR_FOUND=1
) else (
    if %QUIET_MODE% EQU 0 ( echo [成功] 找到CMake )
)
if %QUIET_MODE% EQU 0 ( echo. )

:: 检查编译器
if %QUIET_MODE% EQU 0 ( echo 检查编译器... )

if "%TOOLCHAIN%"=="msvc" (
    goto check_msvc
) else if "%TOOLCHAIN%"=="mingw" (
    goto check_mingw
) else (
    goto check_both_compilers
)

:check_msvc
if %QUIET_MODE% EQU 0 ( echo MSVC: )
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo [错误] 未找到MSVC编译器cl.exe
    echo 请安装Visual Studio或启动开发者命令提示符
    set ERROR_FOUND=1
) else (
    if %QUIET_MODE% EQU 0 ( echo [成功] 找到MSVC编译器cl.exe )
)
goto compiler_check_done

:check_mingw
if %QUIET_MODE% EQU 0 ( echo MinGW: )
where gcc.exe >nul 2>&1
if errorlevel 1 (
    echo [错误] 未找到MinGW编译器gcc.exe
    echo 请安装MinGW或将其bin目录添加到PATH环境变量
    set ERROR_FOUND=1
) else (
    if %QUIET_MODE% EQU 0 ( 
        echo [成功] 找到MinGW编译器gcc.exe
        gcc --version | findstr "gcc"
    )
)
goto compiler_check_done

:check_both_compilers
if %QUIET_MODE% EQU 0 ( echo MSVC: )
where cl.exe >nul 2>&1
if errorlevel 1 (
    if %QUIET_MODE% EQU 0 ( echo [信息] 未找到MSVC编译器cl.exe，这不是错误，除非您打算使用MSVC构建 )
) else (
    if %QUIET_MODE% EQU 0 ( echo [成功] 找到MSVC编译器cl.exe )
)

if %QUIET_MODE% EQU 0 ( echo MinGW: )
where gcc.exe >nul 2>&1
if errorlevel 1 (
    if %QUIET_MODE% EQU 0 ( echo [信息] 未找到MinGW编译器gcc.exe，这不是错误，除非您打算使用MinGW构建 )
) else (
    if %QUIET_MODE% EQU 0 (
        echo [成功] 找到MinGW编译器gcc.exe
        gcc --version | findstr "gcc"
    )
)

:compiler_check_done
if %QUIET_MODE% EQU 0 ( echo. )

:: 检查环境变量
if %QUIET_MODE% EQU 0 (
    echo 检查环境变量...
    echo PATH环境变量: (显示部分重要路径)
    echo %PATH% | findstr /i "mingw" 
    echo %PATH% | findstr /i "vcpkg"
    echo %PATH% | findstr /i "cmake"
    echo %PATH% | findstr /i "ninja"
    echo.
)

:: 尝试找到VCPKG
if %QUIET_MODE% EQU 0 ( echo 检查vcpkg... )

:: 首先检查用户是否设置了VCPKG_ROOT环境变量
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\vcpkg.exe" (
        set VCPKG_FOUND=1
        set VCPKG_EXE=%VCPKG_ROOT%\vcpkg.exe
        if %QUIET_MODE% EQU 0 ( echo [成功] 找到vcpkg: %VCPKG_EXE% )
    ) else (
        if %QUIET_MODE% EQU 0 ( echo [警告] VCPKG_ROOT环境变量已设置，但未在此路径找到vcpkg.exe: %VCPKG_ROOT% )
        if %QUIET_MODE% EQU 0 ( echo [信息] 将尝试在其他位置搜索vcpkg... )
    )
)

:: 如果未找到，检查PATH中是否有vcpkg.exe
if %VCPKG_FOUND% EQU 0 (
    where vcpkg.exe >nul 2>&1
    if not errorlevel 1 (
        for /f "tokens=*" %%i in ('where vcpkg.exe') do (
            set VCPKG_EXE=%%i
            set VCPKG_FOUND=1
            if %QUIET_MODE% EQU 0 ( echo [成功] 在PATH中找到vcpkg: !VCPKG_EXE! )
            
            :: 设置VCPKG_ROOT为vcpkg.exe所在的目录
            for %%j in ("!VCPKG_EXE!") do set VCPKG_ROOT=%%~dpj
            set VCPKG_ROOT=!VCPKG_ROOT:~0,-1!
            if %QUIET_MODE% EQU 0 ( echo [信息] 已自动设置VCPKG_ROOT为: !VCPKG_ROOT! )
        )
    )
)

:: 如果仍未找到，搜索常见位置
if %VCPKG_FOUND% EQU 0 (
    :: 检查一些可能存在vcpkg的常见位置
    set COMMON_VCPKG_PATHS=C:\vcpkg;C:\dev\vcpkg;C:\src\vcpkg;%USERPROFILE%\vcpkg;%USERPROFILE%\source\vcpkg;%USERPROFILE%\dev\vcpkg;%USERPROFILE%\Documents\vcpkg;%USERPROFILE%\git\vcpkg;%USERPROFILE%\GitHub\vcpkg
    
    for %%p in (%COMMON_VCPKG_PATHS%) do (
        if exist "%%p\vcpkg.exe" (
            set VCPKG_EXE=%%p\vcpkg.exe
            set VCPKG_ROOT=%%p
            set VCPKG_FOUND=1
            if %QUIET_MODE% EQU 0 ( echo [成功] 在常见位置找到vcpkg: !VCPKG_EXE! )
            if %QUIET_MODE% EQU 0 ( echo [信息] 已自动设置VCPKG_ROOT为: !VCPKG_ROOT! )
            goto vcpkg_found
        )
    )
    
    echo [错误] 未找到vcpkg.exe。请安装vcpkg或正确设置VCPKG_ROOT环境变量。
    echo 可从 https://github.com/microsoft/vcpkg 下载安装vcpkg。
    echo 安装后，请设置VCPKG_ROOT环境变量指向vcpkg安装目录:
    echo   set VCPKG_ROOT=C:\path\to\vcpkg
    set ERROR_FOUND=1
    goto vcpkg_check_done
)

:vcpkg_found
if %QUIET_MODE% EQU 0 ( echo [成功] 找到vcpkg: %VCPKG_EXE% )

:: 确定正确的triplet和依赖项名称
set REQUIRED_DEPS_ERROR=0
set JSON_PKG=nlohmann-json
set FTXUI_PKG=ftxui
set CROW_PKG=crow

set VCPKG_TRIPLET=
if "%TOOLCHAIN%"=="msvc" (
    set VCPKG_TRIPLET=x64-windows
) else if "%TOOLCHAIN%"=="mingw" (
    set VCPKG_TRIPLET=x64-mingw-static
)

if %QUIET_MODE% EQU 0 (
    echo 检查所需的依赖项:
    echo -------------------------------------
)

:: 检查nlohmann-json库
"%VCPKG_EXE%" list %JSON_PKG%:%VCPKG_TRIPLET% >nul 2>&1
if errorlevel 1 (
    echo [错误] 未安装%JSON_PKG%库，请使用以下命令安装:
    echo   "%VCPKG_EXE%" install %JSON_PKG%:%VCPKG_TRIPLET%
    set REQUIRED_DEPS_ERROR=1
    set ERROR_FOUND=1
) else (
    if %QUIET_MODE% EQU 0 ( echo [成功] 已安装%JSON_PKG%库 )
)

:: 检查FTXUI库
"%VCPKG_EXE%" list %FTXUI_PKG%:%VCPKG_TRIPLET% >nul 2>&1
if errorlevel 1 (
    echo [错误] 未安装%FTXUI_PKG%库，请使用以下命令安装:
    echo   "%VCPKG_EXE%" install %FTXUI_PKG%:%VCPKG_TRIPLET%
    set REQUIRED_DEPS_ERROR=1
    set ERROR_FOUND=1
) else (
    if %QUIET_MODE% EQU 0 ( echo [成功] 已安装%FTXUI_PKG%库 )
)

:: 检查Crow库
"%VCPKG_EXE%" list %CROW_PKG%:%VCPKG_TRIPLET% >nul 2>&1
if errorlevel 1 (
    echo [错误] 未安装%CROW_PKG%库，请使用以下命令安装:
    echo   "%VCPKG_EXE%" install %CROW_PKG%:%VCPKG_TRIPLET%
    set REQUIRED_DEPS_ERROR=1
    set ERROR_FOUND=1
) else (
    if %QUIET_MODE% EQU 0 ( echo [成功] 已安装%CROW_PKG%库 )
)

:: 如果设置了debug模式，输出更多环境信息
if %DEBUG_MODE% EQU 1 (
    echo.
    echo [调试信息] 环境变量:
    echo VCPKG_ROOT = %VCPKG_ROOT%
    echo VCPKG_EXE = %VCPKG_EXE%
    echo TOOLCHAIN = %TOOLCHAIN%
    echo VCPKG_TRIPLET = %VCPKG_TRIPLET%
    echo ERROR_FOUND = %ERROR_FOUND%
    echo REQUIRED_DEPS_ERROR = %REQUIRED_DEPS_ERROR%
    
    :: 列出vcpkg已安装的包
    echo.
    echo [调试信息] 已安装的vcpkg包:
    "%VCPKG_EXE%" list
    echo.
)

:vcpkg_check_done
if %QUIET_MODE% EQU 0 (
    echo.
    echo 检查完成。
    if !ERROR_FOUND! EQU 1 (
        echo [错误] 发现问题，请修复上述错误后重试。
        if !REQUIRED_DEPS_ERROR! EQU 1 (
            echo 请安装缺少的依赖库。
        )
    ) else (
        echo [成功] 所有检查通过，可以继续构建。
    )
)

exit /b !ERROR_FOUND! 