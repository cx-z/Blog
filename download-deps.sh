#!/bin/bash
# 手动下载 Crow 和 nlohmann/json 的脚本
# 如果 CMake FetchContent 失败，可以使用此脚本

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
DEPS_DIR="$BUILD_DIR/deps"

echo "=========================================="
echo "  手动下载博客系统依赖"
echo "=========================================="
echo ""

# 创建目录
mkdir -p "$BUILD_DIR"/{crow_include,json_include/nlohmann}
mkdir -p "$DEPS_DIR"

cd "$DEPS_DIR"

echo "📥 下载 Crow 框架..."
if curl -L --progress-bar \
    https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp \
    -o crow_all.hpp; then
    echo "✓ Crow 下载成功"
    cp crow_all.hpp "$BUILD_DIR/crow_include/"
else
    echo "❌ Crow 下载失败"
    echo "   可能原因："
    echo "   1. 网络连接问题"
    echo "   2. GitHub 访问限制"
    echo "   3. 文件路径已改变"
    exit 1
fi

echo ""
echo "📥 下载 nlohmann/json..."
if curl -L --progress-bar \
    https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp \
    -o json.hpp; then
    echo "✓ json 下载成功"
    cp json.hpp "$BUILD_DIR/json_include/nlohmann/"
else
    echo "❌ json 下载失败"
    exit 1
fi

echo ""
echo "=========================================="
echo "✓ 依赖下载完成！"
echo "=========================================="
echo ""
echo "文件位置："
echo "  • $BUILD_DIR/crow_include/crow_all.hpp"
echo "  • $BUILD_DIR/json_include/nlohmann/json.hpp"
echo ""
echo "现在可以运行编译："
echo "  cd $BUILD_DIR"
echo "  cmake ../server"
echo "  cmake --build . --config Release"
echo ""
