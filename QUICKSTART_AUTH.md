# 🚀 认证系统快速开始指南

## 编译和运行

```bash
# 1. 进入项目目录
cd /Users/bytedance/Projects/Blog

# 2. 编译项目（包含 OpenSSL 依赖）
./build.sh

# 3. 启动服务器
cd build
./bin/blog_server

# 服务器输出: Blog server starting on http://0.0.0.0:8080
```

## 使用应用

### 方式 1：网页界面

1. **打开浏览器**: http://localhost:8080
2. **自动重定向到登录页**（因为首次访问未登录）
3. **创建新账号**:
   - 点击"立即注册"
   - 输入账号（≥3字符）和密码（≥6字符）
   - 点击注册
4. **自动跳转到首页**
5. **发布文章**:
   - 点击"发布文章"
   - 输入标题和内容
   - 点击发布

### 方式 2：API 测试

#### 1️⃣ 注册用户
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"myuser","password":"password123"}'

# 响应 (201 Created):
# {
#   "success": true,
#   "message": "Registration successful",
#   "data": {
#     "user_id": 1,
#     "username": "myuser",
#     "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
#   }
# }
```

#### 2️⃣ 登录用户
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"myuser","password":"password123"}'

# 响应 (200 OK):
# {
#   "success": true,
#   "message": "Login successful",
#   "data": {
#     "user_id": 1,
#     "username": "myuser",
#     "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
#   }
# }
```

#### 3️⃣ 验证 Token
```bash
curl -X POST http://localhost:8080/api/auth/verify \
  -H "Content-Type: application/json" \
  -d '{"token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."}'

# 响应 (200 OK):
# {
#   "success": true,
#   "message": "Token is valid",
#   "data": {
#     "user_id": 1,
#     "username": "myuser"
#   }
# }
```

#### 4️⃣ 创建文章（需要 Token）
```bash
TOKEN="从登录响应获取的 token"

curl -X POST http://localhost:8080/api/posts \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "title": "我的第一篇博客",
    "content": "这是通过 API 创建的文章"
  }'

# 响应 (201 Created):
# {
#   "success": true,
#   "data": {
#     "id": 1,
#     "title": "我的第一篇博客",
#     "content": "这是通过 API 创建的文章",
#     "timestamp": 1770476185973
#   }
# }
```

#### ❌ 没有 Token 创建文章（失败）
```bash
curl -X POST http://localhost:8080/api/posts \
  -H "Content-Type: application/json" \
  -d '{"title": "Test", "content": "test"}'

# 响应 (401 Unauthorized):
# {
#   "success": false,
#   "message": "Missing or invalid Authorization header"
# }
```

## 错误码参考

| 状态码 | 含义 | 场景 |
|--------|------|------|
| 200 | 成功 | 登录、验证 Token 成功 |
| 201 | 已创建 | 注册成功、创建文章成功 |
| 400 | 请求错误 | 缺少字段、密码过短等 |
| 401 | 未授权 | 密码错误、Token 无效或过期、缺少 Token |
| 409 | 冲突 | 账号已存在 |
| 500 | 服务器错误 | 数据库操作失败 |

## 关键安全特性

✅ **密码加密**: SHA-256 + 16 字节随机 Salt  
✅ **Token 签名**: HMAC-SHA256 签名，难以伪造  
✅ **Token 过期**: 默认 7 天自动过期  
✅ **无状态认证**: 服务器无需存储会话信息  
✅ **权限隔离**: 未登录用户无法创建/编辑/删除文章  

## 数据库结构

### users 表
```sql
CREATE TABLE users (
  id INTEGER PRIMARY KEY AUTOINCREMENT,           -- 用户 ID
  username TEXT UNIQUE NOT NULL,                  -- 账号（唯一）
  password_hash TEXT NOT NULL,                    -- 密码哈希值
  salt TEXT NOT NULL,                             -- Salt（十六进制）
  created_at INTEGER NOT NULL                     -- 创建时间（毫秒）
);
```

### posts 表（保持不变）
```sql
CREATE TABLE posts (
  id INTEGER PRIMARY KEY AUTOINCREMENT,           -- 文章 ID
  title TEXT NOT NULL,                            -- 标题
  content TEXT NOT NULL,                          -- 内容
  timestamp INTEGER NOT NULL                      -- 发布时间（毫秒）
);
```

## 前端技术细节

### Token 存储位置
- localStorage 中的 `authToken` 键存储 JWT Token
- localStorage 中的 `username` 键存储用户名
- localStorage 中的 `userId` 键存储用户 ID

### 自动跳转逻辑
- `index.html` 和 `editor.html` 在加载时自动检查 Token
- 如果 Token 无效或缺失，自动重定向到 `login.html`
- 用户点击"退出登录"时清除 localStorage 并重定向

### 请求认证
```javascript
// 后端自动附加 Token 到所有需要认证的请求
const response = await authenticatedFetch('/api/posts', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ title: '...', content: '...' })
});
// 自动添加: Authorization: Bearer {token}
```

## 常见问题

**Q: 如何修改 Token 过期时间？**  
A: 编辑 `server/include/jwt_utils.h` 中的 `EXPIRATION_TIME` 常量。

**Q: 如何修改 JWT 签名密钥？**  
A: 编辑 `server/include/jwt_utils.h` 中的 `SECRET_KEY` 常量。

**Q: 密码可以明文存储吗？**  
A: 不可以，所有密码必须使用 SHA-256+Salt 哈希后存储。

**Q: Token 过期后怎么办？**  
A: 前端会自动检测到 401 错误，清除 Token 并重定向到登录页。

**Q: 可以在多个设备登录吗？**  
A: 可以，每次登录都会生成新的 Token，无需注销前一个设备。

## 故障排查

**问题**: 编译时找不到 OpenSSL  
**解决**: macOS 用户运行 `brew install openssl`

**问题**: 登录页面无法加载  
**解决**: 确保服务器正在运行，检查 http://localhost:8080/login.html

**问题**: "Token 无效"错误  
**解决**: Token 可能已过期（7天），请重新登录

**问题**: 无法创建文章  
**解决**: 检查 Authorization Header 是否正确包含了 Token

## 下一步优化（可选）

- [ ] 添加邮箱验证
- [ ] 实现密码重置功能
- [ ] 支持第三方登录（Google/GitHub）
- [ ] 添加刷新 Token 机制
- [ ] 实现用户权限级别（管理员/普通用户）
- [ ] 添加操作日志和审计功能
- [ ] 支持 HTTPS 和 CORS 限制
