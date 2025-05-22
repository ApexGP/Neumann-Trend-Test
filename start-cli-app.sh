#!/bin/bash
echo "正在启动诺依曼趋势测试命令行工具..."

# 确保当前目录是项目根目录
cd "$(dirname "$0")"

# 检查并创建必要的目录和文件
if [ ! -d "data" ]; then
    echo "创建data目录..."
    mkdir -p data
fi

if [ ! -f "data/standard_values.json" ]; then
    echo "警告：标准值文件不存在，将创建空文件..."
    echo "{}" >data/standard_values.json
fi

# 查找CLI应用程序
CLI_APP=""

if [ -f "bin/neumann_cli_app" ]; then
    CLI_APP="bin/neumann_cli_app"
elif [ -f "build/linux-release/bin/neumann_cli_app" ]; then
    echo "警告: 使用旧版本的构建输出路径"
    CLI_APP="build/linux-release/bin/neumann_cli_app"
else
    echo "错误: 未找到命令行应用程序可执行文件"
    echo "请确保已成功构建项目"
    read -p "按回车键继续..." KEY
    exit 1
fi

# 显示摘要信息
echo "-----------------------------------------"
echo "诺依曼趋势测试命令行工具启动摘要："
echo "-----------------------------------------"
echo "可执行文件：$CLI_APP"
echo "当前工作目录：$(pwd)"
echo "数据目录：$(pwd)/data"
echo "-----------------------------------------"
echo "可用命令行选项："
echo "-f, --file [文件路径]    处理CSV数据文件"
echo "-h, --help              显示帮助信息"
echo "-----------------------------------------"

# 启动应用程序
$CLI_APP "$@"

read -p "按回车键继续..." KEY
