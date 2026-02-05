#!/bin/bash
# 博客系统快速编译和启动脚本

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "=========================================="
echo "  简易博客系统 - 快速启动脚本"
echo "=========================================="
echo ""

# 检查依赖
echo "✓ 检查依赖..."
if ! command -v cmake &> /dev/null; then
    echo "❌ 错误: CMake 未安装。请运行: brew install cmake"
    exit 1
fi

if ! command -v sqlite3 &> /dev/null; then
    echo "❌ 错误: SQLite3 未安装。请运行: brew install sqlite3"
    exit 1
fi

echo "✓ 依赖检查完成"
echo ""

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    echo "📁 创建构建目录..."
    mkdir -p "$BUILD_DIR"
fi

# 进入构建目录
cd "$BUILD_DIR"

# CMake 配置
echo "🔧 运行 CMake 配置..."
cmake "$PROJECT_DIR/server"

# 编译
echo "🔨 编译项目..."
cmake --build . --config Release

echo ""
echo "=========================================="
echo "  ✓ 编译完成！"
echo "=========================================="
echo ""
echo "📝 要启动服务器，请运行:"
echo "   cd $BUILD_DIR"
echo "   ./bin/blog_server"
echo ""
echo "🌐 然后在浏览器打开:"
echo "   http://localhost:8080/index.html"
echo ""
echo "📖 更多信息请查看 README.md"
echo ""
