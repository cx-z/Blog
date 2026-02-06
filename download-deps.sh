#!/bin/bash
# 手动下载 Crow 和 nlohmann/json 的脚本
# 这个脚本应该在 build 目录中运行

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEPS_DIR="$PROJECT_DIR/build/deps"

echo "=========================================="
echo "  📥 下载博客系统依赖文件"
echo "=========================================="
echo ""
echo "依赖文件将被下载到: $DEPS_DIR"
echo ""

# 创建目录结构
mkdir -p "$DEPS_DIR"/{crow_include,json_include/nlohmann}

# 下载 Crow 框架 (使用 GitHub releases CDN 链接)
echo "[1/2] 下载 Crow 框架..."
if curl -L --progress-bar \
    "https://github.com/CrowCpp/Crow/releases/download/v1.0%2B5/crow_all.hpp" \
    -o "$DEPS_DIR/crow_include/crow_all.hpp"; then
    echo "✓ Crow 下载成功"
    echo "   位置: $DEPS_DIR/crow_include/crow_all.hpp"
else
    echo "❌ Crow 下载失败 (尝试备用源...)"
    # 尝试备用源（jsdelivr CDN）
    if curl -L --progress-bar \
        "https://cdn.jsdelivr.net/gh/CrowCpp/Crow@v1.0+5/include/crow_all.hpp" \
        -o "$DEPS_DIR/crow_include/crow_all.hpp"; then
        echo "✓ Crow 下载成功 (来自备用源)"
    else
        echo "❌ Crow 下载失败 (所有源)"
        echo ""
        echo "故障排除:"
        echo "  1. 检查网络连接"
        echo "  2. 尝试使用代理: export http_proxy=..."
        echo "  3. 手动下载:"
        echo "     • 访问: https://github.com/CrowCpp/Crow/releases"
        echo "     • 下载 crow_all.hpp"
        echo "     • 放到: $DEPS_DIR/crow_include/"
        exit 1
    fi
fi

echo ""

# 下载 nlohmann/json (更稳定的源)
echo "[2/2] 下载 nlohmann/json..."
if curl -L --progress-bar \
    "https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp" \
    -o "$DEPS_DIR/json_include/nlohmann/json.hpp"; then
    echo "✓ JSON 下载成功"
    echo "   位置: $DEPS_DIR/json_include/nlohmann/json.hpp"
else
    echo "❌ JSON 下载失败"
    echo ""
    echo "故障排除:"
    echo "  1. 检查网络连接"
    echo "  2. 尝试使用代理: export http_proxy=..."
    echo "  3. 手动下载: https://github.com/nlohmann/json/releases"
    exit 1
fi

echo ""
echo "=========================================="
echo "✓ 所有依赖下载完成！"
echo "=========================================="
echo ""
echo "📁 文件验证:"
echo ""

# 验证文件
if [ -f "$DEPS_DIR/crow_include/crow_all.hpp" ]; then
    CROW_SIZE=$(du -h "$DEPS_DIR/crow_include/crow_all.hpp" | cut -f1)
    CROW_BYTES=$(wc -c < "$DEPS_DIR/crow_include/crow_all.hpp")
    if [ "$CROW_BYTES" -gt 100000 ]; then
        echo "  ✓ crow_all.hpp ($CROW_SIZE)"
    else
        echo "  ❌ crow_all.hpp 文件过小 ($CROW_BYTES 字节) - 可能下载失败"
        echo "     请检查网络连接或尝试手动下载"
        exit 1
    fi
else
    echo "  ❌ crow_all.hpp (文件不存在)"
    exit 1
fi

if [ -f "$DEPS_DIR/json_include/nlohmann/json.hpp" ]; then
    JSON_SIZE=$(du -h "$DEPS_DIR/json_include/nlohmann/json.hpp" | cut -f1)
    JSON_BYTES=$(wc -c < "$DEPS_DIR/json_include/nlohmann/json.hpp")
    if [ "$JSON_BYTES" -gt 100000 ]; then
        echo "  ✓ json.hpp ($JSON_SIZE)"
    else
        echo "  ❌ json.hpp 文件过小 ($JSON_BYTES 字节) - 可能下载失败"
        exit 1
    fi
else
    echo "  ❌ json.hpp (文件不存在)"
    exit 1
fi

echo ""
echo "🚀 现在可以编译了:"
echo "   cd $PROJECT_DIR/build"
echo "   cmake ../server"
echo "   cmake --build . --config Release"
echo ""
