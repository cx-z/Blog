#!/bin/bash

# 登录功能测试脚本
set -e

BASE_URL="http://localhost:8080"

echo "=========================================="
echo "  登录功能集成测试"
echo "=========================================="
echo ""

# 1. 测试注册
echo "✅ 测试 1: 用户注册"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
REGISTER_RESPONSE=$(curl -s -X POST "$BASE_URL/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser2","password":"password123"}')

echo "$REGISTER_RESPONSE" | python3 -m json.tool || echo "$REGISTER_RESPONSE"
REGISTER_TOKEN=$(echo "$REGISTER_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('data', {}).get('token', ''))" 2>/dev/null || echo "")

if [ -z "$REGISTER_TOKEN" ]; then
    echo "❌ 注册失败！"
    exit 1
fi
echo "✅ 注册成功，获取 Token"
echo ""

# 2. 测试重复注册
echo "✅ 测试 2: 检测重复账号"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
DUPLICATE_RESPONSE=$(curl -s -X POST "$BASE_URL/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser2","password":"password123"}')
echo "$DUPLICATE_RESPONSE" | python3 -m json.tool
echo ""

# 3. 测试登录
echo "✅ 测试 3: 用户登录"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser2","password":"password123"}')
echo "$LOGIN_RESPONSE" | python3 -m json.tool
echo ""

# 4. 测试错误的密码
echo "✅ 测试 4: 错误的密码"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
WRONG_PASS_RESPONSE=$(curl -s -X POST "$BASE_URL/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser2","password":"wrongpassword"}')
echo "$WRONG_PASS_RESPONSE" | python3 -m json.tool
echo ""

# 5. 测试 Token 验证
echo "✅ 测试 5: Token 验证"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
VERIFY_RESPONSE=$(curl -s -X POST "$BASE_URL/api/auth/verify" \
  -H "Content-Type: application/json" \
  -d "{\"token\":\"$REGISTER_TOKEN\"}")
echo "$VERIFY_RESPONSE" | python3 -m json.tool
echo ""

# 6. 测试创建文章（不带 Token）
echo "✅ 测试 6: 创建文章（无 Token - 应该失败）"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
NO_TOKEN_RESPONSE=$(curl -s -X POST "$BASE_URL/api/posts" \
  -H "Content-Type: application/json" \
  -d '{"title":"Test","content":"Content"}')
echo "$NO_TOKEN_RESPONSE" | python3 -m json.tool
echo ""

# 7. 测试创建文章（带 Token）
echo "✅ 测试 7: 创建文章（有效 Token）"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
VALID_TOKEN_RESPONSE=$(curl -s -X POST "$BASE_URL/api/posts" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $REGISTER_TOKEN" \
  -d '{"title":"My First Blog Post","content":"This is my first blog post with authentication!"}')
echo "$VALID_TOKEN_RESPONSE" | python3 -m json.tool
echo ""

echo "=========================================="
echo "  ✅ 所有测试完成！"
echo "=========================================="
