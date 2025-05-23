@echo off
setlocal enabledelayedexpansion

REM Change to project root directory
cd /d %~dp0

REM Set environment variable to control bind address
set CROW_BINDADDR=127.0.0.1

REM Check and create necessary directories
if not exist "data" (
    mkdir data
)

if not exist "data\standard_values.json" (
    echo Warning: Standard values file not found, creating empty file...
    echo {} > data\standard_values.json
)

if not exist "web" (
    echo Warning: web directory not found, creating...
    mkdir web
)

REM Ensure web directory has HTML files
if not exist "web\neumann_trend_test.html" (
    echo Warning: HTML file missing in web directory
    
    REM Try to copy from build directory
    if exist "build\mingw-release\_deps\crow-src\examples\simple_example\index.html" (
        echo Copying basic HTML file from Crow examples...
        copy "build\mingw-release\_deps\crow-src\examples\simple_example\index.html" "web\neumann_trend_test.html"
    ) else (
        echo Creating simple placeholder HTML file...
        echo ^<!DOCTYPE html^>^<html^>^<head^>^<title^>Neumann Trend Test^</title^>^</head^>^<body^>^<h1^>Neumann Trend Test Server Running^</h1^>^</body^>^</html^> > web\neumann_trend_test.html
    )
)

REM Find web application
set WEB_APP=""

if exist "bin\neumann_web_app.exe" (
    set WEB_APP=bin\neumann_web_app.exe
) else if exist "build\mingw-release\bin\neumann_web_app.exe" (
    echo Warning: Using legacy build output path
    set WEB_APP=build\mingw-release\bin\neumann_web_app.exe
) else if exist "build\msvc-release\bin\neumann_web_app.exe" (
    echo Warning: Using legacy build output path
    set WEB_APP=build\msvc-release\bin\neumann_web_app.exe
) else if exist "bin\Release\neumann_web.exe" (
    set WEB_APP=bin\Release\neumann_web.exe
) else (
    echo Error: Web application executable not found
    echo Please ensure the project has been built successfully
    pause
    exit /b 1
)

REM Display summary information
echo -----------------------------------------
echo Neumann Trend Test Web Server Launcher
echo -----------------------------------------
echo Executable: !WEB_APP!
echo Working Directory: %CD%
echo Web Resources Directory: %CD%\web
echo Data Directory: %CD%\data
echo Bind Address: 127.0.0.1:8080
echo -----------------------------------------
echo Please visit in browser: http://localhost:8080
echo Press Ctrl+C to stop the server
echo -----------------------------------------

REM Launch web application (in current window)
"!WEB_APP!"

pause 