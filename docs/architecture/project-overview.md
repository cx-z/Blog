# 项目概览

本项目是一个 C++（Crow）+ SQLite 的全栈博客系统，包含前端静态页面与后端 REST API，并在文章接口上启用 JWT 鉴权。

## 文档导航

- 30 秒入口：../../QUICKSTART.md
- 构建与 API 汇总：../../README.md
- 产品形态总览（PRD/规格）：../../PRODUCT_OVERVIEW.md
- 登录/注册：../auth/login-register.md
- Token 会话：../auth/token-session.md
- 文章：../posts/write.md、../posts/list.md、../posts/read.md、../posts/delete.md

## 项目结构（关键路径）

```
Blog/
├── server/
│   ├── src/
│   │   ├── main.cpp                 # 路由装配、鉴权、静态文件服务
│   │   ├── database.cpp             # SQLite 表与 CRUD
│   ├── posts/
│   │   ├── include/
│   │   │   └── post_service.h       # 文章路由注册（接收 app/db）
│   │   └── src/
│   │       └── post_service.cpp     # posts 路由实现（list/read/write/delete）
│   ├── login/
│   │   ├── include/
│   │   │   ├── login_service.h      # 登录路由注册（接收 app/db）
│   │   │   └── register_service.h   # 注册路由注册（接收 app/db）
│   │   │   └── verify_service.h     # verify 路由注册（接收 app/db）
│   │   │   └── jwt_utils.h          # JWT 生成/校验/解析接口
│   │   │   └── crypto_utils.h       # 密码加盐哈希接口
│   │   └── src/
│   │       ├── login_service.cpp    # POST /api/auth/login + POST /api/auth/delete
│   │       └── register_service.cpp # POST /api/auth/register
│   │       └── verify_service.cpp   # POST /api/auth/verify
│   │       └── jwt_utils.cpp        # JWT 生成/校验/解析实现
│   │       └── crypto_utils.cpp     # 密码加盐哈希实现
│   ├── third_party/
│   │   ├── crow_include/            # Crow 单头文件依赖（本地下载）
│   │   └── json_include/            # nlohmann/json 单头文件依赖（本地下载）
│   └── include/
│       ├── database.h               # Post/User 结构与 DB 接口
│   └── CMakeLists.txt               # 构建配置（包含 login/src/*.cpp 与 posts/src/*.cpp）
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
  - Crow 路由：认证/文章路由装配与静态文件服务
  - 认证路由装配：将 login/register/verify 服务注册到同一个 app
  - 文章路由装配：将 posts 服务注册到同一个 app
  - 鉴权：解析 `Authorization: Bearer <token>` 并校验 JWT
  - 静态文件服务：将 `web/` 作为站点根目录对外提供
- `server/posts/*`
  - 文章接口路由：将 posts 相关路由定义与处理逻辑从 main.cpp 拆分出来
  - 依赖注入：通过参数接收 `crow::SimpleApp& app` 与 `Database& db` 并完成路由注册
- `server/login/*`
  - 登录/注册/verify 接口路由：将认证相关的路由定义与处理逻辑从 main.cpp 拆分出来
  - 依赖注入：通过参数接收 `crow::SimpleApp& app` 与 `Database& db` 并完成路由注册
- `server/src/database.cpp`
  - 自动建表与兼容性迁移（为旧库补列）
  - users/posts CRUD（含管理员软删语义）
- `server/third_party/*`
  - 后端构建所需的第三方头文件依赖（本地下载，Git 忽略）
- `server/CMakeLists.txt`
  - 构建目标与源文件清单（需要包含 `server/login/src/*.cpp` 与 `server/posts/src/*.cpp` 才能链接通过）
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
  role TEXT DEFAULT 'user',
  is_deleted INTEGER NOT NULL DEFAULT 0,
  deleted_at INTEGER,
  original_username TEXT
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
    │   ├─ POST /api/auth/register（server/login）
    │   ├─ POST /api/auth/login（server/login）
    │   └─ POST /api/auth/verify（server/login）
    │   └─ POST /api/auth/delete（server/login，注销账号）
    │
    └─ 文章接口（需要 Authorization: Bearer <token>，server/posts）
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
- 密码存储：注册时生成 salt，使用哈希存储（server/login/include/crypto_utils.h + server/login/src/crypto_utils.cpp），不保存明文
- JWT：HMAC-SHA256 签名 + 过期时间校验（server/login/include/jwt_utils.h + server/login/src/jwt_utils.cpp）

更细的接口级错误码、前端会话行为与排错：

- 认证：../auth/login-register.md、../auth/token-session.md
- 文章：../posts/write.md、../posts/list.md、../posts/read.md、../posts/delete.md
