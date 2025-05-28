#!/bin/bash

echo "===== 诺依曼趋势测试工具 ====="
echo "Neumann Trend Test Tool"
echo ""
echo "启动CLI应用程序，您可以在程序内选择启动Web服务器"
echo "Starting CLI application, you can choose to start web server within the program"
echo ""

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 查找可执行文件
if [ -f "$SCRIPT_DIR/bin/neumann_cli_app" ]; then
    cd "$SCRIPT_DIR"
    exec "./bin/neumann_cli_app" "$@"
elif [ -f "$SCRIPT_DIR/neumann_cli_app" ]; then
    cd "$SCRIPT_DIR"
    exec "./neumann_cli_app" "$@"
else
    echo "Error: neumann_cli_app not found"
    echo "错误: 未找到 neumann_cli_app"
    echo "Please ensure the script is in the same directory as the executable"
    echo "请确保脚本与可执行文件在同一目录中"
    echo "or the executable is in the bin subdirectory."
    echo "或可执行文件在bin子目录中。"
    echo ""
    read -p "Press Enter to continue..."
    exit 1
fi
