#!/bin/bash
# API 测试脚本 - 使用 curl 测试博客 API

API_BASE="http://localhost:8080/api"

echo "=========================================="
echo "  博客系统 API 测试脚本"
echo "=========================================="
echo ""

# 检查服务器是否在线
echo "[1] 检查服务器连接..."
if ! curl -s "$API_BASE/posts" > /dev/null 2>&1; then
    echo "❌ 错误: 无法连接到服务器"
    echo "请确保后端服务正在运行: ./bin/blog_server"
    exit 1
fi
echo "✓ 服务器正在线"
echo ""

# 获取所有文章
echo "[2] 获取所有文章 (GET /api/posts)..."
curl -s "$API_BASE/posts" | jq . 2>/dev/null || curl -s "$API_BASE/posts"
echo ""
echo ""

# 创建新文章
echo "[3] 创建新文章 (POST /api/posts)..."
CREATE_RESPONSE=$(curl -s -X POST "$API_BASE/posts" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "API 测试文章",
    "content": "这是通过 API 创建的测试文章。\n\n特性包括:\n- RESTful API\n- SQLite 数据库\n- Crow Web 框架\n- 实时更新"
  }')

echo "$CREATE_RESPONSE" | jq . 2>/dev/null || echo "$CREATE_RESPONSE"

# 提取新创建文章的 ID
NEW_POST_ID=$(echo "$CREATE_RESPONSE" | jq -r '.data.id' 2>/dev/null)
if [ -z "$NEW_POST_ID" ] || [ "$NEW_POST_ID" == "null" ]; then
    NEW_POST_ID="1"
fi
echo ""
echo "✓ 文章已创建，ID: $NEW_POST_ID"
echo ""

# 获取单篇文章
echo "[4] 获取单篇文章 (GET /api/posts/$NEW_POST_ID)..."
curl -s "$API_BASE/posts/$NEW_POST_ID" | jq . 2>/dev/null || curl -s "$API_BASE/posts/$NEW_POST_ID"
echo ""
echo ""

# 更新文章
echo "[5] 更新文章 (PUT /api/posts/$NEW_POST_ID)..."
curl -s -X PUT "$API_BASE/posts/$NEW_POST_ID" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "API 测试文章 (已更新)",
    "content": "这是更新后的内容。\n\n更新时间: '$(date)'\n更新者: curl 脚本"
  }' | jq . 2>/dev/null
echo ""
echo ""

# 删除文章
echo "[6] 删除文章 (DELETE /api/posts/$NEW_POST_ID)..."
curl -s -X DELETE "$API_BASE/posts/$NEW_POST_ID" | jq . 2>/dev/null || curl -s -X DELETE "$API_BASE/posts/$NEW_POST_ID"
echo ""
echo ""

# 最后获取所有文章
echo "[7] 最终验证 - 获取所有文章..."
curl -s "$API_BASE/posts" | jq . 2>/dev/null || curl -s "$API_BASE/posts"
echo ""

echo "=========================================="
echo "✓ API 测试完成"
echo "=========================================="
echo ""
echo "💡 提示: 如果输出不是 JSON 格式，请确保安装了 jq"
echo "    macOS: brew install jq"
echo "    Linux: apt-get install jq"
