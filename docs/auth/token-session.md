# Token Session

## 功能概览

本模块覆盖：

- Token 生成与有效期
- Token 校验：`POST /api/auth/verify`
- API 鉴权约定：`Authorization: Bearer <token>`
- 前端会话行为（自动校验、401 自动登出）
- 注销账号后的会话失效规则（已注销账号的旧 Token 视为无效）

相关代码：

- 后端 JWT：../../server/login/include/jwt_utils.h（接口）+ ../../server/login/src/jwt_utils.cpp（实现）
- 后端 verify：../../server/login/src/verify_service.cpp
- 前端会话：../../web/js/auth.js

## 快速验证

### 1) 登录拿到 token

```bash
curl -s -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"myuser","password":"password123"}'
```

### 2) 调用 verify 校验 token

```bash
TOKEN="替换为登录响应中的 token"

curl -i -X POST http://localhost:8080/api/auth/verify \
  -H "Content-Type: application/json" \
  -d "{\"token\":\"$TOKEN\"}"
```

### 3) 用 Authorization 调用受保护接口

```bash
curl -i http://localhost:8080/api/posts \
  -H "Authorization: Bearer $TOKEN"
```

## API 详解

### POST /api/auth/verify

验证 JWT Token 是否有效。

**Request**

- Headers
  - `Content-Type: application/json`
- Body
  - `token`: string（JWT）

**Response**

- `200 OK`
  - `success`: `true`
  - `message`: `"Token is valid"`
  - `data.user_id`: number
  - `data.username`: string
  - `data.role`: string（默认 `"user"`）
- `400 Bad Request`
  - `message`: `"Missing token"`
- `401 Unauthorized`
  - `message`: `"Invalid or expired token"`

## 错误码

| HTTP | message | 场景 |
|---|---|---|
| 400 | Missing token | verify 缺少 token 字段 |
| 401 | Invalid or expired token | 签名不匹配 / 过期 / token 格式错误 / 账号不存在或已注销 |

## 数据库结构

Token 本身不落库；会话状态由前端保存 Token 决定。权限信息来自 users 表的 `role` 字段。

## 前端细节

### localStorage 键

../../web/js/auth.js 使用：

- `authToken`
- `username`
- `userId`
- `role`

### 自动校验与自动登出

- 受保护页面调用 `authManager.checkAuthRequired()`：
  - 未登录：跳转 `login.html`
  - 已登录：调用 `/api/auth/verify` 验证 token
  - 校验失败：清理 localStorage 并跳转 `login.html`
- `authenticatedFetch()` 会自动附加 `Authorization: Bearer <token>`
- 任意 API 返回 401 时：前端会清理 localStorage 并跳转 `login.html`

### “登出”是什么

当前系统没有后端登出接口；登出是前端行为：

- 清理 localStorage（Token/用户信息）
- 重定向到登录页

### “注销账号”是什么

注销账号是后端接口行为（`POST /api/auth/delete`）：

- 会删除该账号的所有博客文章
- 会将账号标记为已注销，并释放 username 以允许同名重新注册
- 注销后旧 Token 会在 `/api/auth/verify` 与所有受保护接口上被视为无效（返回 401）

## FAQ

### Token 有效期多长？

后端默认 7 天（`JwtUtils::EXPIRATION_TIME = 7 * 24 * 60 * 60`）。

## 配置与定制

### 修改 Token 过期时间

编辑 ../../server/login/src/jwt_utils.cpp：

```cpp
const long long JwtUtils::EXPIRATION_TIME = 7 * 24 * 60 * 60;
```

### 修改 JWT 签名密钥

编辑 ../../server/login/src/jwt_utils.cpp：

```cpp
const std::string JwtUtils::SECRET_KEY = "your_secret_key_here";
```

### 可以多设备登录吗？

可以。每次登录会生成新 Token；系统不维护服务端会话，因此不会主动让旧设备失效。

### 为什么 posts API 返回 401？

posts 类接口要求 `Authorization` 头满足：

- 非空
- 以 `Bearer ` 开头
- token 能通过校验（签名正确且未过期）

## 排错

### 401 Missing or invalid Authorization header

- 检查是否带了 `Authorization: Bearer <token>`
- Bearer 后面是否有空格

### 401 Invalid or expired token

- token 过期：重新登录拿新 token
- token 被截断/复制不完整：重新获取

## 测试脚本

工程根目录提供了一键测试入口，覆盖注册/登录/verify/受保护接口与文章权限校验：

```bash
bash ./test.sh
```

## 优化清单

- 支持 Refresh Token 与短期 Access Token
- 支持服务端 Token 撤销（黑名单/版本号/会话表）
- 将 `SECRET_KEY` 移出代码（环境变量/配置文件），区分开发与生产
- 为接口响应增加稳定的 `error_code` 字段，避免前端依赖 message 文本
