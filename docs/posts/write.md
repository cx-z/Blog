# Write Post

## 功能概览

本模块覆盖：

- 创建文章：`POST /api/posts`
- 编辑文章：`PUT /api/posts/{id}`

相关页面与脚本：

- 页面：../../web/editor.html
- 脚本：../../web/js/editor.js、../../web/js/auth.js

## 快速验证

### 1) 登录拿 token

```bash
curl -i -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"myuser","password":"password123"}'
```

从响应中复制 `data.token`，然后设置：

```bash
TOKEN="替换为登录响应中的 token"
```

### 2) 创建文章

```bash
curl -i -X POST http://localhost:8080/api/posts \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"Hello","content":"From API"}'
```

### 3) 编辑文章

```bash
POST_ID="替换为创建文章返回的 id"

curl -i -X PUT "http://localhost:8080/api/posts/$POST_ID" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"Hello v2","content":"Updated"}'
```

## API 详解

### Authorization 约定

- `Authorization: Bearer <token>`
- 未携带/格式错误：`401`，message 为 `"Missing or invalid Authorization header"`
- token 无效/过期：`401`，message 为 `"Invalid or expired token"`

### POST /api/posts

创建文章（必须已登录）。

**Request**

- Headers
  - `Content-Type: application/json`
  - `Authorization: Bearer <token>`
- Body
  - `title`: string
  - `content`: string

**Response**

- `201 Created`
  - `success`: `true`
  - `data.id`: number
  - `data.title`: string
  - `data.content`: string
  - `data.timestamp`: number（毫秒）
  - `data.author`: string（可能为空，取决于 JOIN 是否能取到用户名）
- `400 Bad Request`
  - `message`: `"Missing title or content"`
- `500 Internal Server Error`
  - `message`: `"Failed to insert post"`

### PUT /api/posts/{id}

更新文章（必须是作者本人，且文章未被管理员软删）。

**Request**

- Headers
  - `Content-Type: application/json`
  - `Authorization: Bearer <token>`
- Path
  - `id`: number
- Body
  - `title`: string
  - `content`: string

**Response**

- `200 OK`
  - `success`: boolean
  - `message`: `"Post updated"` 或 `"Failed to update post"`
- `400 Bad Request`
  - `message`: `"Missing title or content"`
- `403 Forbidden`
  - `message`: `"Forbidden"`
- `404 Not Found`
  - `message`: `"Post not found"`

## 错误码

### 创建

| HTTP | message | 场景 |
|---|---|---|
| 400 | Missing title or content | Body 缺少字段 |
| 401 | Missing or invalid Authorization header | 未携带 Bearer token |
| 401 | Invalid or expired token | token 无效或过期 |
| 500 | Failed to insert post | 写库失败 |

### 编辑

| HTTP | message | 场景 |
|---|---|---|
| 400 | Missing title or content | Body 缺少字段 |
| 401 | Missing or invalid Authorization header | 未携带 Bearer token |
| 401 | Invalid or expired token | token 无效或过期 |
| 403 | Forbidden | 非作者 / 已被管理员软删 |
| 404 | Post not found | id 不存在 |

## 数据库结构

写文章依赖 posts 表：

```sql
CREATE TABLE IF NOT EXISTS posts (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  title TEXT NOT NULL,
  content TEXT NOT NULL,
  timestamp INTEGER NOT NULL,
  user_id INTEGER,
  deleted_by_admin INTEGER NOT NULL DEFAULT 0,
  deleted_at INTEGER
);
```

关键字段：

- `user_id`：作者用户 ID
- `deleted_by_admin` / `deleted_at`：管理员对他人文章执行“软删除”的标记

## 前端细节

### editor.html 的两种模式

- 新建：`editor.html`
  - 提交时调用 `POST /api/posts`
  - 成功后跳转到 `index.html`
- 编辑：`editor.html?mode=edit&id=<postId>`
  - 加载时调用 `GET /api/posts/{id}` 拉取内容并校验作者身份
  - 提交时调用 `PUT /api/posts/{id}`
  - 30 秒自动保存一次（同样走 PUT）

### 401 的处理

`authenticatedFetch()` 遇到 401 会清理本地会话并跳转到登录页，页面上会表现为“需要重新登录”。

## FAQ

### 为什么创建文章响应里 author 可能为空？

后端创建文章时直接返回插入结果；作者用户名是通过 posts/users JOIN 查询得到的，创建接口不会额外回查。

### 为什么文章被管理员删除后无法编辑？

后端 PUT 接口会对 `deleted_by_admin` 做校验，已被管理员软删的文章会返回 403。

## 排错

### 403 Forbidden（编辑）

- 确认当前登录账号是该文章作者
- 确认文章未被管理员软删（列表页会有“已被管理员删除”标记）

### 400 Missing title or content

检查请求 JSON 是否包含 `title` 与 `content`，并确保不是空字符串。

## 优化清单

- 增加 title/content 长度限制与敏感字符校验（服务端）
- 创建接口返回更完整的 data（例如包含 user_id/role/is_author），减少前端二次请求
- 增加草稿保存（本地/服务端）与编辑冲突处理
- 支持 Markdown 渲染与 XSS 白名单策略
