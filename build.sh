#!/bin/bash
echo "===== 构建诺依曼趋势测试工具 ====="

mkdir -p build
cd build

echo "配置项目..."
cmake ..

echo "编译项目..."
cmake --build .

echo "运行测试..."
ctest

echo ""
echo "构建完成。可执行文件位于 build/bin 目录中。"
echo "- neumann_cli_app: 命令行界面应用"
echo "- neumann_web_app: Web界面应用"
echo ""

cd .. 