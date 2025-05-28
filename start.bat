@echo off
chcp 65001 >nul
title Neumann Trend Test Tool

echo ===== 诺依曼趋势测试工具 =====
echo Neumann Trend Test Tool
echo.
echo 启动CLI应用程序，您可以在程序内选择启动Web服务器
echo Starting CLI application, you can choose to start web server within the program
echo.

if exist "bin\neumann_cli_app.exe" (
    cd /d "%~dp0"
    "bin\neumann_cli_app.exe" %*
) else if exist "neumann_cli_app.exe" (
    "neumann_cli_app.exe" %*
) else (
    echo Error: neumann_cli_app.exe not found
    echo 错误: 未找到 neumann_cli_app.exe
    echo Please ensure the script is in the same directory as the executable
    echo 请确保脚本与可执行文件在同一目录中
    echo or the executable is in the bin subdirectory.
    echo 或可执行文件在bin子目录中。
    echo.
    pause
    exit /b 1
)

if errorlevel 1 (
    echo.
    echo An error occurred during program execution
    echo 程序执行过程中发生错误
    pause
) 