#!/bin/bash
echo "===== 诺依曼趋势测试工具构建系统 ====="
echo ""

# 初始化变量
PRESET="linux-release"
CLEAN_BUILD=0
VERBOSE=0
ERROR_FOUND=0

# 处理命令行参数
while [[ $# -gt 0 ]]; do
  case $1 in
    debug)
      PRESET="linux-debug"
      shift
      ;;
    clean)
      CLEAN_BUILD=1
      shift
      ;;
    verbose)
      VERBOSE=1
      shift
      ;;
    *)
      echo "[警告] 未知参数: $1"
      shift
      ;;
  esac
done

# 输出配置信息
echo "构建配置:"
echo "- 预设: $PRESET"
echo ""

# 检查依赖项
echo "检查依赖项..."
# 检查CMake
if ! command -v cmake &> /dev/null; then
    echo "[错误] 未找到CMake，请安装CMake"
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  Fedora: sudo dnf install cmake"
    echo "  Arch Linux: sudo pacman -S cmake"
    ERROR_FOUND=1
fi

# 检查编译器
if ! command -v g++ &> /dev/null; then
    echo "[错误] 未找到g++编译器，请安装GCC"
    echo "  Ubuntu/Debian: sudo apt install g++"
    echo "  Fedora: sudo dnf install gcc-c++"
    echo "  Arch Linux: sudo pacman -S gcc"
    ERROR_FOUND=1
fi

# 检查nlohmann-json
if [ -z "$(pkg-config --list-all 2>/dev/null | grep nlohmann_json)" ] && 
   [ ! -f "/usr/include/nlohmann/json.hpp" ] && 
   [ ! -f "/usr/local/include/nlohmann/json.hpp" ]; then
    echo "[错误] 未找到nlohmann-json库，请安装："
    echo "  Ubuntu/Debian: sudo apt install nlohmann-json3-dev"
    echo "  Fedora: sudo dnf install nlohmann-json-devel"
    echo "  Arch Linux: sudo pacman -S nlohmann-json"
    ERROR_FOUND=1
fi

# 检查FTXUI
if [ ! -d "/usr/include/ftxui" ] && [ ! -d "/usr/local/include/ftxui" ]; then
    echo "[错误] 未找到FTXUI库，请手动安装："
    echo "  git clone https://github.com/ArthurSonzogni/FTXUI"
    echo "  cd FTXUI && mkdir build && cd build"
    echo "  cmake .. -DFTXUI_BUILD_EXAMPLES=OFF -DFTXUI_BUILD_DOCS=OFF"
    echo "  make -j$(nproc) && sudo make install"
    ERROR_FOUND=1
fi

# 检查Crow
if [ ! -f "/usr/include/crow.h" ] && [ ! -f "/usr/include/crow/crow.h" ] && 
   [ ! -f "/usr/local/include/crow.h" ] && [ ! -f "/usr/local/include/crow/crow.h" ]; then
    echo "[错误] 未找到Crow库，请手动安装："
    echo "  git clone https://github.com/CrowCpp/Crow"
    echo "  cd Crow && mkdir build && cd build"
    echo "  cmake .."
    echo "  make -j$(nproc) && sudo make install"
    ERROR_FOUND=1
fi

# 如果有错误，退出构建
if [ $ERROR_FOUND -eq 1 ]; then
    echo ""
    echo "[错误] 依赖检查失败，请安装缺少的依赖项后再试"
    exit 1
fi

echo "[成功] 所有依赖检查通过"
echo ""

# 如果需要清理构建目录
if [ $CLEAN_BUILD -eq 1 ]; then
  echo "清理构建目录: build/$PRESET"
  rm -rf "build/$PRESET"
fi

# 创建构建目录
mkdir -p "build/$PRESET"

# 检查 Ninja 是否可用
if ! command -v ninja &> /dev/null; then
  echo "[警告] 未找到Ninja构建工具，构建可能较慢。建议安装："
  echo "  Ubuntu/Debian: sudo apt install ninja-build"
  echo "  Fedora: sudo dnf install ninja-build"
  echo "  Arch Linux: sudo pacman -S ninja"
fi

# 配置项目
echo ""
echo "配置项目..."
if [ $VERBOSE -eq 1 ]; then
  cmake --preset=$PRESET --log-level=VERBOSE
else
  cmake --preset=$PRESET
fi

if [ $? -ne 0 ]; then
  echo ""
  echo "[错误] 配置失败"
  exit 1
fi

# 构建项目
echo ""
echo "编译项目..."
if [ $VERBOSE -eq 1 ]; then
  cmake --build --preset=$PRESET --verbose
else
  cmake --build --preset=$PRESET
fi

if [ $? -ne 0 ]; then
  echo ""
  echo "[错误] 构建失败"
  exit 1
fi

# 运行测试
echo ""
echo "运行测试..."
ctest --preset=$PRESET

# 显示成功信息
echo ""
echo "构建完成。可执行文件位于 bin 目录中。"
echo "- neumann_cli_app: 命令行界面应用"
echo "- neumann_web_app: Web界面应用"
echo ""
echo "用法:"
echo "- ./build.sh       : 构建Release版本"
echo "- ./build.sh debug : 构建Debug版本"
echo "- ./build.sh clean : 清理后重新构建"
echo "- ./build.sh verbose: 显示详细构建信息"
echo "" 