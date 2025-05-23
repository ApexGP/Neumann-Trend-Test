@echo off
echo ===== 诺依曼趋势测试工具 - 发布包生成器 =====
echo.

set RELEASE_DIR=release
set BUILD_SUCCESS=0

echo 检查构建产物...
if not exist "bin\neumann_cli_app.exe" (
    echo [错误] 未找到 neumann_cli_app.exe，请先构建项目
    echo 请运行: build.bat
    exit /b 1
)

if not exist "bin\neumann_web_app.exe" (
    echo [错误] 未找到 neumann_web_app.exe，请先构建项目
    echo 请运行: build.bat
    exit /b 1
)

echo [成功] 构建产物检查通过

echo.
echo 清理并创建发布目录结构...
if exist "%RELEASE_DIR%" rmdir /s /q "%RELEASE_DIR%"

mkdir "%RELEASE_DIR%"
mkdir "%RELEASE_DIR%\bin"
mkdir "%RELEASE_DIR%\lib"
mkdir "%RELEASE_DIR%\web"
mkdir "%RELEASE_DIR%\config"
mkdir "%RELEASE_DIR%\data"
mkdir "%RELEASE_DIR%\ref"

echo [成功] 发布目录结构已创建

echo.
echo 复制可执行文件...
copy "bin\*.exe" "%RELEASE_DIR%\bin\" >nul
if errorlevel 1 (
    echo [错误] 复制可执行文件失败
    exit /b 1
)
echo [成功] 可执行文件已复制

echo.
echo 复制运行库...
if exist "lib" (
    xcopy "lib\*" "%RELEASE_DIR%\lib\" /s /e /q >nul
    echo [成功] 运行库已复制
) else (
    echo [信息] 未找到lib文件夹，跳过
)

echo.
echo 复制Web资源...
if exist "web" (
    xcopy "web\*" "%RELEASE_DIR%\web\" /s /e /q >nul
    echo [成功] Web资源已复制
) else (
    echo [警告] 未找到web文件夹
)

echo.
echo 复制配置文件...
if exist "config.json" (
    copy "config.json" "%RELEASE_DIR%\config\" >nul
    echo [成功] config.json已复制
) else (
    echo [警告] 未找到config.json
)

if exist "data\translations.json" (
    copy "data\translations.json" "%RELEASE_DIR%\config\" >nul
    echo [成功] translations.json已复制
) else (
    echo [警告] 未找到data\translations.json
)

echo.
echo 复制启动脚本...
if exist "start-cli-app.bat" copy "start-cli-app.bat" "%RELEASE_DIR%\" >nul
if exist "start-web-app.bat" copy "start-web-app.bat" "%RELEASE_DIR%\" >nul
echo [成功] 启动脚本已复制

echo.
echo 复制标准值文件...
if exist "data\standard_values.json" (
    copy "data\standard_values.json" "%RELEASE_DIR%\ref\" >nul
    echo [成功] standard_values.json已复制
) else (
    echo [警告] 未找到data\standard_values.json
)

echo.
echo 创建用户说明文件...
(
echo # 诺依曼趋势测试工具 - 用户版本
echo.
echo ## 目录结构说明
echo.
echo - `bin/` - 可执行文件目录
echo   - `neumann_cli_app.exe` - 命令行界面应用
echo   - `neumann_web_app.exe` - Web界面应用
echo - `lib/` - 运行库文件（通常为空，使用静态编译）
echo - `web/` - Web界面资源文件
echo - `config/` - 配置文件目录
echo   - `config.json` - 应用程序配置
echo   - `translations.json` - 多语言翻译文件
echo - `data/` - 用户数据目录（存放保存的数据集）
echo - `ref/` - 参考文件目录
echo   - `standard_values.json` - 标准值表
echo.
echo ## 使用方法
echo.
echo ### 方法一：使用启动脚本
echo - 双击 `start-cli-app.bat` 启动命令行界面
echo - 双击 `start-web-app.bat` 启动Web界面
echo.
echo ### 方法二：直接运行
echo - 进入 `bin` 目录
echo - 运行 `neumann_cli_app.exe` 或 `neumann_web_app.exe`
echo.
echo ## 注意事项
echo.
echo - 首次运行时会在 `data` 目录下创建用户数据集
echo - 配置文件位于 `config` 目录，可手动编辑
echo - Web界面默认端口为8080，可在配置文件中修改
) > "%RELEASE_DIR%\README.md"

echo [成功] 用户说明文件已创建

echo.
echo 验证发布包...
echo 检查关键文件是否存在:
if exist "%RELEASE_DIR%\bin\neumann_cli_app.exe" (
    echo [✓] neumann_cli_app.exe
) else (
    echo [✗] neumann_cli_app.exe - 缺失
)

if exist "%RELEASE_DIR%\bin\neumann_web_app.exe" (
    echo [✓] neumann_web_app.exe
) else (
    echo [✗] neumann_web_app.exe - 缺失
)

if exist "%RELEASE_DIR%\config\config.json" (
    echo [✓] config.json
) else (
    echo [✗] config.json - 缺失
)

if exist "%RELEASE_DIR%\config\translations.json" (
    echo [✓] translations.json
) else (
    echo [✗] translations.json - 缺失
)

if exist "%RELEASE_DIR%\ref\standard_values.json" (
    echo [✓] standard_values.json
) else (
    echo [✗] standard_values.json - 缺失
)

echo.
echo ===== 发布包生成完成 =====
echo 发布包位置: %RELEASE_DIR%
echo 可以将整个 %RELEASE_DIR% 文件夹打包发布到GitHub
echo.

pause 