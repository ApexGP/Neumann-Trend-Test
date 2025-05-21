@echo off
chcp 65001 > nul
echo 正在启动诺依曼趋势测试Web服务器...

REM 确保当前目录是项目根目录
cd /d %~dp0

REM 设置环境变量以控制绑定地址
set CROW_BINDADDR=127.0.0.1

REM 检查并创建必要的目录和文件
if not exist "data" (
    echo 创建data目录...
    mkdir data
)

if not exist "data\standard_values.json" (
    echo 警告：标准值文件不存在，将创建空文件...
    echo {} > data\standard_values.json
)

if not exist "web" (
    echo 警告：web目录不存在，将创建...
    mkdir web
)

REM 确保web目录中有网页文件
if not exist "web\neumann_trend_test.html" (
    echo 警告：web目录中缺少HTML文件
    
    REM 尝试从构建目录复制
    if exist "build\mingw-release\_deps\crow-src\examples\simple_example\index.html" (
        echo 正在从Crow示例复制基本HTML文件作为替代...
        copy "build\mingw-release\_deps\crow-src\examples\simple_example\index.html" "web\neumann_trend_test.html"
    ) else (
        echo 无法找到HTML文件，创建简单的占位文件...
        echo ^<!DOCTYPE html^>^<html^>^<head^>^<title^>诺依曼趋势测试^</title^>^</head^>^<body^>^<h1^>诺依曼趋势测试服务器正在运行^</h1^>^</body^>^</html^> > web\neumann_trend_test.html
    )
)

REM 查找构建目录中的web应用程序
set WEB_APP=""

if exist "build\mingw-release\bin\neumann_web_app.exe" (
    set WEB_APP=build\mingw-release\bin\neumann_web_app.exe
) else if exist "build\msvc-release\bin\neumann_web_app.exe" (
    set WEB_APP=build\msvc-release\bin\neumann_web_app.exe
) else if exist "bin\Release\neumann_web.exe" (
    set WEB_APP=bin\Release\neumann_web.exe
) else (
    echo 错误: 未找到Web应用程序可执行文件
    echo 请确保已成功构建项目
    pause
    exit /b 1
)

REM 显示摘要信息
echo -----------------------------------------
echo 诺依曼趋势测试Web服务器启动摘要：
echo -----------------------------------------
echo 可执行文件：%WEB_APP%
echo 当前工作目录：%CD%
echo Web资源目录：%CD%\web
echo 数据目录：%CD%\data
echo 绑定地址：127.0.0.1:8080
echo -----------------------------------------
echo 请使用浏览器访问：http://localhost:8080
echo 按Ctrl+C可停止服务器
echo -----------------------------------------

REM 启动Web应用程序（在当前窗口）
%WEB_APP%

pause 