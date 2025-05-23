@echo off
chcp 65001 >nul
title Neumann Trend Test - CLI

echo Starting Neumann Trend Test Tool (CLI)...
echo.

if exist "bin\neumann_cli_app.exe" (
    cd /d "%~dp0"
    "bin\neumann_cli_app.exe" %*
) else if exist "neumann_cli_app.exe" (
    "neumann_cli_app.exe" %*
) else (
    echo Error: neumann_cli_app.exe not found
    echo Please ensure the script is in the same directory as the executable
    echo or the executable is in the bin subdirectory.
    echo.
    pause
    exit /b 1
)

if errorlevel 1 (
    echo.
    echo An error occurred during program execution
    pause
) 