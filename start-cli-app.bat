@echo off
chcp 65001 > nul
echo 正在启动诺依曼趋势测试命令行工具...

REM 确保当前目录是项目根目录
cd /d %~dp0

REM 检查并创建必要的目录和文件
if not exist "data" (
    echo 创建data目录...
    mkdir data
)

if not exist "data\standard_values.json" (
    echo 警告：标准值文件不存在，将创建空文件...
    echo {} > data\standard_values.json
)

REM 查找CLI应用程序
set CLI_APP=""

if exist "bin\neumann_cli_app.exe" (
    set CLI_APP=bin\neumann_cli_app.exe
) else if exist "build\mingw-release\bin\neumann_cli_app.exe" (
    echo 警告: 使用旧版本的构建输出路径
    set CLI_APP=build\mingw-release\bin\neumann_cli_app.exe
) else if exist "build\msvc-release\bin\neumann_cli_app.exe" (
    echo 警告: 使用旧版本的构建输出路径
    set CLI_APP=build\msvc-release\bin\neumann_cli_app.exe
) else if exist "bin\Release\neumann_cli.exe" (
    set CLI_APP=bin\Release\neumann_cli.exe
) else (
    echo 错误: 未找到命令行应用程序可执行文件
    echo 请确保已成功构建项目
    pause
    exit /b 1
)

REM 显示摘要信息
echo -----------------------------------------
echo 诺依曼趋势测试命令行工具启动摘要：
echo -----------------------------------------
echo 可执行文件：%CLI_APP%
echo 当前工作目录：%CD%
echo 数据目录：%CD%\data
echo -----------------------------------------
echo 可用命令行选项：
echo -f, --file [文件路径]    处理CSV数据文件
echo -h, --help              显示帮助信息
echo -----------------------------------------

REM 获取传递给此脚本的所有参数
set ARGS=%*

REM 启动应用程序
%CLI_APP% %ARGS%

pause 