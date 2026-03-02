# Login & Register

## 功能概览

本模块覆盖：

- 注册：`POST /api/auth/register`
- 登录：`POST /api/auth/login`
- 注销账号：`POST /api/auth/delete`（需要二次输入密码确认，会删除该用户所有博客）

相关页面与脚本：

- 页面：../../web/register.html、../../web/login.html
- 脚本：../../web/js/auth.js

## 快速验证

先按 README.md 启动服务，然后执行：

### 注册

```bash
curl -i -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"myuser","password":"password123"}'
```

### 登录

```bash
curl -i -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"myuser","password":"password123"}'
```

### 注销账号

```bash
TOKEN="替换为登录响应中的 token"

curl -i -X POST http://localhost:8080/api/auth/delete \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"password":"password123"}'
```

## API 详解

### POST /api/auth/register

创建用户并返回 Token。

**Request**

- Headers
  - `Content-Type: application/json`
- Body
  - `username`: string，至少 3 个字符
  - `password`: string，至少 6 个字符

**Response**

- `201 Created`
  - `success`: `true`
  - `message`: `"Registration successful"`
  - `data.user_id`: number
  - `data.username`: string
  - `data.role`: string（当前固定为 `"user"`）
  - `data.token`: string（JWT）

**Example**

```json
{
  "success": true,
  "message": "Registration successful",
  "data": {
    "user_id": 1,
    "username": "myuser",
    "role": "user",
    "token": "..."
  }
}
```

### POST /api/auth/login

登录成功后返回 Token。

**Request**

- Headers
  - `Content-Type: application/json`
- Body
  - `username`: string
  - `password`: string

**Response**

- `200 OK`
  - `success`: `true`
  - `message`: `"Login successful"`
  - `data.user_id`: number
  - `data.username`: string
  - `data.role`: string（来自 users.role）
  - `data.token`: string（JWT）

**Example**

```json
{
  "success": true,
  "message": "Login successful",
  "data": {
    "user_id": 1,
    "username": "myuser",
    "role": "user",
    "token": "..."
  }
}
```

### POST /api/auth/delete

注销当前登录账号，并删除该账号的所有博客文章。

说明：

- 必须携带 `Authorization: Bearer <token>`
- 必须在 Body 中再次输入密码确认
- 注销后会释放 username 以允许同名重新注册；新注册用户会获得新的 `user_id`

**Request**

- Headers
  - `Content-Type: application/json`
  - `Authorization: Bearer <token>`
- Body
  - `password`: string（当前账号密码）

**Response**

- `200 OK`
  - `success`: `true`
  - `message`: `"Account deleted"`
- `400 Bad Request`
  - `message`: `"Missing password"`
- `401 Unauthorized`
  - `message`: `"Missing or invalid Authorization header"` / `"Invalid or expired token"` / `"Invalid password"` / `"Account is deleted"`
- `500 Internal Server Error`
  - `message`: `"Failed to delete account"`

## 错误码

### 注册

| HTTP | message | 场景 |
|---|---|---|
| 400 | Missing username or password | Body 缺少字段 |
| 400 | Username must be at least 3 characters | username 不合法 |
| 400 | Password must be at least 6 characters | password 不合法 |
| 409 | Username already exists | 用户名已存在 |
| 500 | Failed to register user | 写库失败 |

### 登录

| HTTP | message | 场景 |
|---|---|---|
| 400 | Missing username or password | Body 缺少字段 |
| 401 | Invalid username or password | 用户不存在或密码错误 |

### 注销账号

| HTTP | message | 场景 |
|---|---|---|
| 400 | Missing password | Body 缺少 password |
| 401 | Missing or invalid Authorization header | 未携带/格式错误 |
| 401 | Invalid or expired token | token 无效/过期/账号已注销 |
| 401 | Invalid password | 二次确认密码错误 |
| 401 | Account is deleted | 账号已注销 |
| 500 | Failed to delete account | 事务执行失败 |

## 数据库结构

认证相关依赖 users 表（初始化时自动创建）：

```sql
CREATE TABLE IF NOT EXISTS users (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  username TEXT UNIQUE NOT NULL,
  password_hash TEXT NOT NULL,
  salt TEXT NOT NULL,
  created_at INTEGER NOT NULL,
  role TEXT DEFAULT 'user',
  is_deleted INTEGER NOT NULL DEFAULT 0,
  deleted_at INTEGER,
  original_username TEXT
);
```

字段说明：

- `password_hash`: `SHA256(password + salt)` 的结果（十六进制字符串）
- `salt`: 以十六进制字符串存储
- `role`: 当前默认写入 `"user"`
- `is_deleted`: 是否已注销（1 表示已注销）
- `deleted_at`: 注销时间（毫秒时间戳）
- `original_username`: 注销前的原用户名（便于审计/排查）

## 前端细节

### localStorage 键

前端使用 ../../web/js/auth.js 管理本地会话：

- `authToken`: JWT Token
- `username`: 用户名
- `userId`: 用户 ID
- `role`: 角色（`user` / `admin`）

### 页面流转

- 受保护页面（如 index.html、editor.html）会调用 `authManager.checkAuthRequired()`：
  - 未登录：跳转到 `login.html`
  - Token 无效：清理 localStorage 并跳转到 `login.html`

## FAQ

### 密码可以明文存储吗？

不可以。后端会生成 salt，并以 `SHA256(password + salt)` 存储哈希值。

### role 从哪里来？

- 注册接口返回的 role 当前固定为 `"user"`
- 登录接口返回 role 来自数据库的 `users.role`

## 排错

### 注册返回 409

用户名已存在；换一个 username 或清理 users 表数据后重试。

### 登录返回 401

确认用户名/密码正确；也可以先用注册接口创建新账号，再登录。

## 配置与定制

### 修改 username/password 校验规则

注册接口的校验逻辑在后端路由中实现，调整位置：

- ../../server/login/src/register_service.cpp（`POST /api/auth/register`）
  - username 最小长度
  - password 最小长度
- ../../server/login/src/login_service.cpp（`POST /api/auth/login`）
  - 如需增加更多账号/密码格式校验，可在此处调整

## 优化清单

- 账号规则（允许字符集、最大长度）与密码强度（复杂度）规则参数化
- 密码哈希升级为更强的 KDF（例如 Argon2 / scrypt / PBKDF2）
- 增加登录失败次数限制与延迟（防爆破）
- 注册/登录增加统一错误码字段（减少前端依赖 message 文本）
