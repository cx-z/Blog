# List Posts

## 功能概览

本模块覆盖：

- 获取文章列表：`GET /api/posts`
- 权限过滤：
  - 普通用户：只返回自己的文章
  - 管理员：返回所有文章（含被软删标记的文章）

相关页面与脚本：

- 页面：../../web/index.html
- 脚本：../../web/js/index.js、../../web/js/auth.js

## 快速验证

```bash
TOKEN="替换为登录响应中的 token"

curl -i http://localhost:8080/api/posts \
  -H "Authorization: Bearer $TOKEN"
```

## API 详解

### GET /api/posts

获取文章列表（必须已登录）。

**Request**

- Headers
  - `Authorization: Bearer <token>`

**Response**

- `200 OK`
  - `success`: `true`
  - `data`: array
    - `id`: number
    - `title`: string
    - `content`: string
    - `timestamp`: number（毫秒）
    - `author`: string（可能为空）
    - `user_id`: number
    - `deleted_by_admin`: number（0/1）
    - `deleted_at`: number（毫秒或 0）
    - `is_author`: boolean（仅用于前端 UI 控制）

## 错误码

| HTTP | message | 场景 |
|---|---|---|
| 401 | Missing or invalid Authorization header | 未携带 Bearer token |
| 401 | Invalid or expired token | token 无效或过期 |

## 数据库结构

查询来自 posts 表，并通过 `posts.user_id = users.id` 获取作者用户名：

```sql
SELECT p.id, p.title, p.content, p.timestamp, p.user_id, u.username, p.deleted_by_admin, p.deleted_at
FROM posts p
LEFT JOIN users u ON p.user_id = u.id
ORDER BY p.timestamp DESC;
```

## 前端细节

### 列表加载

../../web/js/index.js 在页面加载时：

- `authManager.checkAuthRequired()`：未登录或 token 失效会跳转到登录页
- `authenticatedFetch('/api/posts')` 拉取列表并渲染

### UI 行为（与接口字段关系）

- `deleted_by_admin = 1`：
  - 显示“已被管理员删除”徽标
  - 管理员视角下，该条目的删除按钮会禁用
- `is_author = true`：
  - 显示“编辑文章”按钮（跳转 `editor.html?mode=edit&id=...`）
- 管理员视角会额外展示作者信息（优先 `author`，其次 `user_id`）

## FAQ

### 为什么列表里会返回 content？

后端当前直接返回完整 content；前端会在列表里截断显示（例如 `substring(0, 150)`）。

### 为什么 author 可能为空？

作者用户名来自 posts/users 的 LEFT JOIN。如果旧数据里 `user_id` 不存在或无对应 users 记录，`author` 可能为空。

## 排错

### 列表页一直跳回登录页

通常是 token 无效或过期导致 401；前端会自动清理本地会话并跳转登录页。重新登录获取新 token。

### 列表接口返回 401 Missing or invalid Authorization header

确认请求带上：

- `Authorization: Bearer <token>`

## 优化清单

- 列表分页与按时间范围查询（减少一次性返回内容）
- 列表接口改为返回摘要字段（例如 `content_excerpt`），详情页再拉全文
- 增加排序/筛选（按作者、按是否删除、按关键字）
