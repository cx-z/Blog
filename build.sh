#!/bin/bash
# 博客系统快速编译和启动脚本 (改进版 - 支持重试)

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
MAX_RETRIES=3
RETRY_COUNT=0

echo "=========================================="
echo "  简易博客系统 - 快速启动脚本 v2"
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

# 初始化 Git 仓库（如果还没有）
if [ ! -d "$PROJECT_DIR/.git" ]; then
    echo "📁 初始化 Git 仓库..."
    cd "$PROJECT_DIR"
    git init
    git config user.email "you@example.com"
    git config user.name "Blog User"
    echo "✓ Git 仓库已初始化"
    echo ""
fi

# 清理旧的构建目录
if [ -d "$BUILD_DIR" ]; then
    echo "🧹 清理旧的构建文件..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
echo "📁 创建构建目录..."
mkdir -p "$BUILD_DIR"

# 进入构建目录
cd "$BUILD_DIR"

# CMake 配置 - 带重试机制
echo "🔧 运行 CMake 配置..."
while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    if cmake "$PROJECT_DIR/server" -DCMAKE_VERBOSE_MAKEFILE=ON; then
        echo "✓ CMake 配置成功"
        break
    else
        RETRY_COUNT=$((RETRY_COUNT + 1))
        if [ $RETRY_COUNT -lt $MAX_RETRIES ]; then
            echo "⚠️  CMake 配置失败，重试 ($RETRY_COUNT/$MAX_RETRIES)..."
            sleep 2
        else
            echo "❌ CMake 配置失败 (重试 $MAX_RETRIES 次后放弃)"
            echo ""
            echo "💡 故障排除建议:"
            echo "   1. 查看 BUILD_TROUBLESHOOTING.md"
            echo "   2. 检查网络连接"
            echo "   3. 尝试清理和重新开始:"
            echo "      rm -rf $BUILD_DIR"
            echo "      ./build.sh"
            exit 1
        fi
    fi
done

# 编译
echo ""
echo "🔨 编译项目..."
if ! cmake --build . --config Release; then
    echo "❌ 编译失败"
    echo ""
    echo "💡 常见解决方案:"
    echo "   1. 检查错误信息是否涉及下载失败"
    echo "   2. 手动下载依赖文件（见 BUILD_TROUBLESHOOTING.md）"
    echo "   3. 清理并重试: rm -rf $BUILD_DIR && ./build.sh"
    exit 1
fi

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
echo "📖 更多信息请查看 README.md 或 BUILD_TROUBLESHOOTING.md"
echo ""
