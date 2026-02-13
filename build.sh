#!/bin/bash

# 博客系统编译脚本 - 纯编译版本
# 使用方法: ./build.sh [--help|--clean|--debug|--run]

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
SERVER_DIR="$PROJECT_DIR/server"

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 帮助
show_help() {
    echo ""
    echo "博客系统编译脚本"
    echo "================"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  --help      显示此帮助信息"
    echo "  --clean     删除并重新编译"
    echo "  --debug     编译 Debug 版本 (默认: Release)"
    echo "  --run       编译后自动运行"
    echo ""
    echo "示例:"
    echo "  $0                          # 编译 Release 版本"
    echo "  $0 --debug --run            # 编译 Debug 版本并运行"
    echo "  $0 --clean --run            # 重新编译并运行"
    echo ""
}

# 解析参数
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_SERVER=false

for arg in "$@"; do
    case $arg in
        --help)
            show_help
            exit 0
            ;;
        --clean)
            CLEAN_BUILD=true
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --run)
            RUN_SERVER=true
            ;;
        *)
            echo "错误: 未知选项 '$arg'"
            show_help
            exit 1
            ;;
    esac
done

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  📚 博客系统编译${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查依赖
echo -e "${YELLOW}📋 检查依赖文件...${NC}"
CROW_FILE="$SERVER_DIR/third_party/crow_include/crow_all.hpp"
JSON_FILE="$SERVER_DIR/third_party/json_include/nlohmann/json.hpp"

if [ ! -f "$CROW_FILE" ] || [ ! -f "$JSON_FILE" ]; then
    echo -e "${RED}❌ 缺少依赖文件${NC}"
    echo ""
    
    [ ! -f "$CROW_FILE" ] && echo "  • $CROW_FILE"
    [ ! -f "$JSON_FILE" ] && echo "  • $JSON_FILE"
    
    echo ""
    echo "解决方案: 请按 deps.md 指引下载第三方头文件到 server/third_party"
    echo ""
    exit 1
fi

echo -e "${GREEN}✓ 依赖文件完整${NC}"
echo ""

# 清理
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}🧹 清理构建目录...${NC}"
    rm -rf "$BUILD_DIR"/*
    echo -e "${GREEN}✓ 完成${NC}"
    echo ""
fi

# 创建build目录
mkdir -p "$BUILD_DIR"

# CMake 配置
echo -e "${YELLOW}🔧 CMake 配置 ($BUILD_TYPE 版本)...${NC}"
cd "$BUILD_DIR"

if ! cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "$SERVER_DIR" 2>&1 | tail -10; then
    echo -e "${RED}❌ CMake 配置失败${NC}"
    exit 1
fi

echo -e "${GREEN}✓ 配置完成${NC}"
echo ""

# 编译
echo -e "${YELLOW}⚙️  编译中...${NC}"
if cmake --build . --config "$BUILD_TYPE" --parallel; then
    echo ""
    echo -e "${GREEN}✓ 编译成功${NC}"
else
    echo ""
    echo -e "${RED}❌ 编译失败${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ 编译完成！${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

echo "📍 输出位置:"
echo "  • 执行文件: $BUILD_DIR/bin/blog_server"
echo ""

if [ "$RUN_SERVER" = true ]; then
    echo -e "${YELLOW}🚀 启动服务器...${NC}"
    echo ""
    cd "$BUILD_DIR"
    ./bin/blog_server
else
    echo "启动服务器:"
    echo -e "${YELLOW}  cd $BUILD_DIR && ./bin/blog_server${NC}"
    echo ""
fi
