# Delete Post

## 功能概览

本模块覆盖：

- 删除文章：`DELETE /api/posts/{id}`
- 权限规则：
  - 作者本人可删除自己的文章（硬删除）
  - 管理员可删除任意文章
- 删除语义：
  - 管理员删除他人文章：软删除（更新 `deleted_by_admin/deleted_at`）
  - 其他情况：硬删除（从 posts 表删除行）

相关页面与脚本：

- 页面：../../web/index.html
- 脚本：../../web/js/index.js、../../web/js/auth.js

## 快速验证

```bash
TOKEN="替换为登录响应中的 token"
POST_ID="替换为文章 id"

curl -i -X DELETE "http://localhost:8080/api/posts/$POST_ID" \
  -H "Authorization: Bearer $TOKEN"
```

## API 详解

### DELETE /api/posts/{id}

删除文章（必须已登录，且满足权限规则）。

**Request**

- Headers
  - `Authorization: Bearer <token>`
- Path
  - `id`: number

**Response**

- `200 OK`
  - `success`: boolean
  - `message`: `"Post deleted"` 或 `"Failed to delete post"`
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

### posts 表删除语义

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

### 硬删除

```sql
DELETE FROM posts WHERE id = ?;
```

### 软删除（管理员删除他人文章）

```sql
UPDATE posts SET deleted_by_admin = 1, deleted_at = ? WHERE id = ?;
```

## 前端细节

### 删除按钮

../../web/js/index.js：

- 点击删除会弹出确认框
- 成功后重新拉取列表
- 管理员视角下，如果 `deleted_by_admin = 1`，删除按钮会禁用（避免重复软删）

### 软删对 UI 的影响

- 列表项会显示“已被管理员删除”徽标
- `is_author` 会变为 `false`（用于隐藏“编辑”按钮）

## FAQ

### 管理员软删后，作者还能看到文章吗？

能。软删只是标记字段，文章仍在 posts 表中，作者列表仍会返回该文章，但会带上 `deleted_by_admin = 1`。

### 管理员软删后，作者还能编辑吗？

不能。后端 PUT /api/posts/{id} 对 `deleted_by_admin` 做了校验，会返回 403。

## 排错

### 403 Forbidden

- 普通用户只能删除自己的文章
- 管理员可以删除任意文章，但需要 role=admin 且 token 有效

### 删除后仍在列表里

- 如果是管理员删除他人文章：这是软删的预期行为
- 如果你期望彻底移除，需要作者本人删除（硬删），或调整后端策略

## 优化清单

- 统一删除语义（作者也软删），提供回收站与恢复接口
- 增加管理员审计日志（谁在什么时候删除了哪篇文章）
- 增加“删除原因”字段与展示
- 为软删文章增加筛选（默认隐藏/可显示）
