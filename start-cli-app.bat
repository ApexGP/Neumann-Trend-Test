@echo off
setlocal enabledelayedexpansion

REM Change to project root directory
cd /d %~dp0

REM Check and create necessary directories
if not exist "data" (
    mkdir data
)

if not exist "data\standard_values.json" (
    echo Warning: Standard values file not found, creating empty file...
    echo {} > data\standard_values.json
)

REM Find CLI application
set CLI_APP=""

if exist "bin\neumann_cli_app.exe" (
    set CLI_APP=bin\neumann_cli_app.exe
) else if exist "build\mingw-release\bin\neumann_cli_app.exe" (
    echo Warning: Using legacy build output path
    set CLI_APP=build\mingw-release\bin\neumann_cli_app.exe
) else if exist "build\msvc-release\bin\neumann_cli_app.exe" (
    echo Warning: Using legacy build output path
    set CLI_APP=build\msvc-release\bin\neumann_cli_app.exe
) else if exist "bin\Release\neumann_cli.exe" (
    set CLI_APP=bin\Release\neumann_cli.exe
) else (
    echo Error: CLI application executable not found
    echo Please ensure the project has been built successfully
    pause
    exit /b 1
)

REM Display summary information
echo -----------------------------------------
echo Neumann Trend Test CLI Tool Launcher
echo -----------------------------------------
echo Executable: !CLI_APP!
echo Working Directory: %CD%
echo Data Directory: %CD%\data
echo -----------------------------------------
echo Available command line options:
echo -f, --file [filepath]    Process CSV data file
echo -h, --help              Show help information
echo -----------------------------------------

REM Get all arguments passed to this script
set ARGS=%*

REM Launch application
"!CLI_APP!" %ARGS%

pause 