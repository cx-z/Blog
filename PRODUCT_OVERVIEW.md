# 📦 产品形态总览（PRD/规格）

本文件描述“当前产品形态的全貌”（面向实现与协作），不展开接口级细节与构建细节。

- 30 秒入口：QUICKSTART.md
- 构建、运行、API 汇总与排错：README.md
- 工程架构与代码结构：docs/architecture/project-overview.md

---

## 1. 产品定位

一个带认证与权限控制的简易博客系统：

- 前端：静态页面（web/），通过 API 与后端交互
- 后端：Crow 提供 REST API，同时提供静态文件服务
- 数据库：SQLite 持久化 users/posts

## 2. 用户与角色

### 2.1 角色

- `user`：普通用户
- `admin`：管理员

### 2.2 会话形态

- 登录成功后获得 JWT Token（无状态）
- 前端将 Token 存储在 localStorage，并在请求时通过 `Authorization: Bearer <token>` 发送

细节见：

- docs/auth/login-register.md
- docs/auth/token-session.md

## 3. 信息架构（页面）

| 页面 | URL | 入口/跳转 | 说明 |
|---|---|---|---|
| 登录 | `/login.html` | 未登录访问受保护页会跳转 | 登录后进入首页 |
| 注册 | `/register.html` | 登录页跳转 | 注册后可直接进入首页 |
| 首页（列表） | `/index.html` | 登录成功后进入 | 展示文章列表；支持阅读、删除 |
| 编辑器 | `/editor.html` | 首页进入 | 新建文章 |
| 编辑器（编辑模式） | `/editor.html?mode=edit&id=<id>` | 首页“编辑”入口 | 编辑自己的文章 |

## 4. 业务对象与关键规则

### 4.1 Post（文章）

文章包含：

- `title`、`content`
- `timestamp`
- `user_id`（作者）
- `deleted_by_admin`、`deleted_at`（管理员软删标记）

### 4.2 可见性与权限边界（核心）

| 能力 | user（普通用户） | admin（管理员） | 说明 |
|---|---|---|---|
| 列表（GET /api/posts） | 仅自己的文章 | 所有文章（含软删标记） | docs/posts/list.md |
| 阅读（GET /api/posts/{id}） | 仅自己的文章 | 任意文章 | docs/posts/read.md |
| 创建（POST /api/posts） | 可创建 | 可创建 | docs/posts/write.md |
| 编辑（PUT /api/posts/{id}） | 仅编辑自己的文章 | 仅编辑自己的文章 | 管理员不能编辑他人文章 |
| 删除（DELETE /api/posts/{id}） | 仅删除自己的文章（硬删） | 可删除任意文章 | 删除语义见下 |

### 4.3 删除语义（作者硬删 / 管理员软删）

- 作者删除自己的文章：硬删除（从 posts 表移除）
- 管理员删除他人文章：软删除（写入 `deleted_by_admin=1` 与 `deleted_at`）
- 软删后：
  - 文章记录仍存在
  - 作者仍可在列表中看到并可阅读/删除，但不可编辑
  - 管理员视角会标记为“已被管理员删除”，并避免重复软删

细节见：docs/posts/delete.md、docs/posts/write.md、docs/posts/list.md

## 5. 核心用户路径（端到端）

### 5.1 普通用户路径

1. 注册/登录 → 获得 Token
2. 进入首页查看自己的文章列表
3. 进入编辑器发布文章
4. 在首页阅读文章详情
5. 删除自己的文章

### 5.2 管理员路径

1. 登录（role=admin）
2. 查看所有文章（含作者信息与软删标记）
3. 删除他人文章（软删）

## 6. 文档分层（权威来源）

为避免重复维护，细节信息以功能文档为准：

- 认证：docs/auth/login-register.md、docs/auth/token-session.md
- 文章：docs/posts/write.md、docs/posts/list.md、docs/posts/read.md、docs/posts/delete.md

## 7. 非目标（当前不包含）

为减少误解，以下能力当前不在产品形态范围内：

- 评论、标签、收藏
- 富文本/Markdown 渲染
- 图片上传与附件管理
- 公共文章与匿名访问（当前默认需要登录且有权限边界）
- 搜索、分页、排序筛选
- 服务端会话撤销/刷新令牌（Refresh Token）

## 8. Roadmap / 扩展方向（可选）

- 公开/私密文章模型与分享链接
- 列表分页、搜索、筛选与摘要字段
- Refresh Token 与服务端撤销机制（黑名单/版本号/会话表）
- 统一的错误码字段（`error_code`）以降低前端对 message 的依赖
- 管理员审计日志（操作记录、删除原因）
