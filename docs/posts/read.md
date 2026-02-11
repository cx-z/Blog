# Read Post

## 功能概览

本模块覆盖：

- 获取单篇文章：`GET /api/posts/{id}`
- 权限规则：
  - 作者本人可读
  - 管理员可读任意文章
  - 其他用户读他人文章会返回 403

相关页面与脚本：

- 页面：../../web/index.html
- 脚本：../../web/js/index.js、../../web/js/auth.js

## 快速验证

```bash
TOKEN="替换为登录响应中的 token"
POST_ID="替换为文章 id"

curl -i "http://localhost:8080/api/posts/$POST_ID" \
  -H "Authorization: Bearer $TOKEN"
```

## API 详解

### GET /api/posts/{id}

获取文章详情（必须已登录，且满足权限规则）。

**Request**

- Headers
  - `Authorization: Bearer <token>`
- Path
  - `id`: number

**Response**

- `200 OK`
  - `success`: `true`
  - `data.id`: number
  - `data.title`: string
  - `data.content`: string
  - `data.timestamp`: number（毫秒）
  - `data.author`: string（可能为空）
  - `data.user_id`: number（作者 ID）
  - `data.deleted_by_admin`: number（0/1）
  - `data.deleted_at`: number（毫秒或 0）
  - `data.is_author`: boolean（仅用于前端 UI 控制）
- `403 Forbidden`
  - `message`: `"Forbidden"`
- `404 Not Found`
  - `message`: `"Post not found"`

## 错误码

| HTTP | message | 场景 |
|---|---|---|
| 401 | Missing or invalid Authorization header | 未携带 Bearer token |
| 401 | Invalid or expired token | token 无效或过期 |
| 403 | Forbidden | 不是作者且不是管理员 |
| 404 | Post not found | id 不存在 |

## 数据库结构

读取单篇文章通过 posts/users JOIN 查询作者信息：

```sql
SELECT p.id, p.title, p.content, p.timestamp, p.user_id, u.username, p.deleted_by_admin, p.deleted_at
FROM posts p
LEFT JOIN users u ON p.user_id = u.id
WHERE p.id = ?;
```

## 前端细节

### 首页的“阅读更多”

../../web/js/index.js 中 `showPost(id)` 会：

- 调用 `authenticatedFetch('/api/posts/{id}')`
- 将返回的 `data.title/content/timestamp` 渲染到 Bootstrap Modal
- 管理员视角会额外展示作者信息（`author` / `user_id`）

## FAQ

### 为什么普通用户无法阅读别人的文章？

后端 GET /api/posts/{id} 只允许作者本人或管理员读取，否则返回 403。

### 什么是“遗留文章 user_id 为 -1/0 不对普通用户开放”？

后端对“不是作者且非管理员”的请求会统一返回 403；如果旧数据 `user_id` 不是当前用户，则普通用户无法读取，管理员可以读取。

## 排错

### 403 Forbidden

- 确认当前账号就是文章作者
- 或使用管理员账号访问（role=admin）

### 404 Post not found

确认 id 是否存在；也可能已被作者硬删除（DELETE）。

## 优化清单

- 增加公开文章能力（例如公开/私密字段），避免必须“作者或管理员”
- 增加专用详情页（而不是 Modal）与分享链接
- 为错误响应增加稳定的 `error_code`（例如 `POST_FORBIDDEN`）
