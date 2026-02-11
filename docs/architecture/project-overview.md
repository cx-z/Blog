# 项目概览

本项目是一个 C++（Crow）+ SQLite 的全栈博客系统，包含前端静态页面与后端 REST API，并在文章接口上启用 JWT 鉴权。

## 文档导航

- 30 秒入口：../../QUICKSTART.md
- 构建与 API 汇总：../../README.md
- 认证入口：../../QUICKSTART_AUTH.md
- 登录/注册：../auth/login-register.md
- Token 会话：../auth/token-session.md
- 文章：../posts/write.md、../posts/list.md、../posts/read.md、../posts/delete.md

## 项目结构（关键路径）

```
Blog/
├── server/
│   ├── src/
│   │   ├── main.cpp                 # 路由、鉴权、静态文件服务
│   │   └── database.cpp             # SQLite 表与 CRUD
│   └── include/
│       ├── database.h               # Post/User 结构与 DB 接口
│       ├── crypto_utils.h           # 密码加盐哈希
│       └── jwt_utils.h              # JWT 生成/校验/解析
├── web/
│   ├── index.html                   # 列表/阅读/删除入口
│   ├── editor.html                  # 新建/编辑入口
│   ├── login.html                   # 登录
│   ├── register.html                # 注册
│   └── js/
│       ├── auth.js                  # Token 管理、路由保护、authenticatedFetch
│       ├── index.js                 # 列表/阅读/删除交互
│       └── editor.js                # 新建/编辑交互
├── docs/
│   ├── architecture/
│   │   └── project-overview.md      # 本文档（项目总览）
│   ├── auth/
│   └── posts/
```

## 核心组件与职责

- `server/src/main.cpp`
  - Crow 路由：认证接口与文章接口
  - 鉴权：解析 `Authorization: Bearer <token>` 并校验 JWT
  - 静态文件服务：将 `web/` 作为站点根目录对外提供
- `server/src/database.cpp`
  - 自动建表与兼容性迁移（为旧库补列）
  - users/posts CRUD（含管理员软删语义）
- `web/js/auth.js`
  - localStorage 会话存储
  - 受保护页面校验（verify token）
  - `authenticatedFetch` 自动附加 Authorization 并处理 401

## 数据库设计（关键表）

### users

```sql
CREATE TABLE IF NOT EXISTS users (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  username TEXT UNIQUE NOT NULL,
  password_hash TEXT NOT NULL,
  salt TEXT NOT NULL,
  created_at INTEGER NOT NULL,
  role TEXT DEFAULT 'user'
);
```

### posts

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

## API 工作流（端到端）

```
客户端（浏览器）
    ↓
静态资源请求（/index.html、/editor.html、/js/*、/css/*）
    ↓
Crow 静态文件服务（根目录 web/）
    ↓
前端脚本发起 API 请求（fetch）
    ↓
Crow API 路由（/api/*）
    ├─ 认证接口（无需 Authorization）
    │   ├─ POST /api/auth/register
    │   ├─ POST /api/auth/login
    │   └─ POST /api/auth/verify
    │
    └─ 文章接口（需要 Authorization: Bearer <token>）
        ├─ 校验请求头格式
        ├─ JwtUtils::verifyToken(token)
        ├─ 读取 role（普通用户 / admin）
        └─ Database::* 执行读写
            ├─ GET /api/posts
            ├─ GET /api/posts/{id}
            ├─ POST /api/posts
            ├─ PUT /api/posts/{id}
            └─ DELETE /api/posts/{id}（管理员删除他人文章为软删）
    ↓
SQLite（db/blog.db）
    ↓
JSON 响应
    ↓
前端渲染（列表/Modal/跳转）
```

用户级别的验收路径见：../../QUICKSTART.md  
认证与会话详解见：../auth/login-register.md、../auth/token-session.md

## 安全考虑（总览）

- SQL 注入防护：数据库写入使用参数化绑定（详见 server/src/database.cpp 的 sqlite3_bind_*）
- XSS 防护：前端渲染前会做 HTML 转义（见 web/js/index.js）
- CORS：API 响应添加 `Access-Control-Allow-Origin: *`（见 server/src/main.cpp）
- 路径遍历防护：静态文件路由拒绝包含 `..` 的路径片段（见 server/src/main.cpp）
- 密码存储：注册时生成 salt，使用哈希存储（crypto_utils.h），不保存明文
- JWT：HMAC-SHA256 签名 + 过期时间校验（jwt_utils.h）

更细的接口级错误码、前端会话行为与排错：

- 认证：../auth/login-register.md、../auth/token-session.md
- 文章：../posts/write.md、../posts/list.md、../posts/read.md、../posts/delete.md
