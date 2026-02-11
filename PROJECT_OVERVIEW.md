# 项目概览

## 📊 项目文件清单

```
Blog/
├── server/                          # C++ 后端
│   ├── src/
│   │   ├── main.cpp                 # 主程序 + API 路由 (255 行)
│   │   │   ├── 静态文件服务
│   │   │   ├── GET /api/posts       # 获取所有文章
│   │   │   ├── GET /api/posts/:id   # 获取单篇文章
│   │   │   ├── POST /api/posts      # 创建文章
│   │   │   ├── PUT /api/posts/:id   # 更新文章
│   │   │   └── DELETE /api/posts/:id # 删除文章
│   │   └── database.cpp             # 数据库实现 (203 行)
│   │       ├── SQLite 连接管理
│   │       ├── 表创建
│   │       └── CRUD 操作
│   ├── include/
│   │   ├── database.h               # 数据库头文件 (48 行)
│   │   │   ├── Post 数据结构
│   │   │   └── Database 类接口
│   │   ├── crypto_utils.h           # 密码加密工具
│   │   └── jwt_utils.h              # JWT 生成与校验
│   └── CMakeLists.txt               # CMake 构建配置
│
├── web/                             # 前端
│   ├── index.html                   # 首页（文章列表）
│   │   └── Bootstrap 5 + 自定义样式
│   ├── editor.html                  # 发布页（编辑器）
│   │   └── 表单验证 + API 交互
│   ├── login.html                   # 登录页
│   ├── register.html                # 注册页
│   ├── js/
│   │   ├── auth.js                  # 认证逻辑
│   │   ├── index.js                 # 首页逻辑 (73 行)
│   │   │   ├── 加载文章列表
│   │   │   ├── 显示文章详情
│   │   │   ├── 格式化时间
│   │   │   └── HTML 转义
│   │   └── editor.js                # 编辑页逻辑 (48 行)
│   │       ├── 表单提交处理
│   │       ├── POST 请求
│   │       └── 错误提示
│   └── css/
│       └── style.css                # 全局样式 (226 行)
│           ├── 卡片动画
│           ├── 表单美化
│           ├── 响应式设计
│           └── 深色主题导航
│
├── db/                              # 数据库文件夹
│   └── blog.db                      # SQLite 数据库（自动生成）
│
├── build/                           # 构建输出（自动生成）
│   ├── bin/
│   │   └── blog_server              # 编译后的可执行文件
│   ├── CMakeCache.txt               # CMake 生成文件
│   └── CMakeFiles/                  # CMake 中间文件
│
├── .vscode/                         # 编辑器配置
│   └── settings.json                # 工作区设置
│
├── AUTH_IMPLEMENTATION.md           # 认证实现说明
├── COMPLETION_SUMMARY.md            # 完成总结
├── PROJECT_OVERVIEW.md              # 项目概览
├── QUICKSTART.md                    # 快速启动
├── README.md                        # 完整编译运行指南
├── deps.md                          # 依赖说明
├── docs/                            # 功能文档（按模块拆分）
│   ├── auth/
│   │   ├── login-register.md        # 登录/注册
│   │   └── token-session.md         # Token 会话
│   └── posts/
│       ├── delete.md                # 删除文章（含管理员软删）
│       ├── list.md                  # 文章列表
│       ├── read.md                  # 阅读文章
│       └── write.md                 # 创建/编辑文章
├── build.sh                         # 快速编译脚本
├── test-api.sh                      # API 测试脚本
├── test-auth.sh                     # 认证测试脚本
└── .gitignore                       # Git 忽略配置
```

## 🏗️ 核心技术实现

### 后端架构

```cpp
// 数据结构
struct Post {
    int id;                    // 自增主键
    std::string title;         // 文章标题
    std::string content;       // 文章内容
    long long timestamp;       // 发布时间戳
    int user_id;               // 作者用户 ID
    std::string author;        // 作者用户名
    int deleted_by_admin;      // 是否被管理员删除
    long long deleted_at;      // 删除时间戳
};

// 数据库类
class Database {
    // SQLite 连接管理
    sqlite3* db;
    
    // CRUD 方法
    bool insertPost(...)       // 创建
    vector<Post> getAllPosts() // 读取列表
    Post getPostById(int)      // 读取单条
    bool updatePost(...)       // 更新
    bool deletePost(int)       // 删除
    bool softDeletePost(...)   // 软删除
};

// Crow Web 服务器
app.get("/api/posts", ...)      // RESTful 路由
app.post("/api/posts", ...)
app.put("/api/posts/<int>", ...)
app.delete_("/api/posts/<int>", ...)
```

### 前端架构

```javascript
// 首页 (index.html)
- 异步加载文章列表
- 实时渲染 DOM
- 模态框显示详情
- 时间格式化

// 发布页 (editor.html)
- 表单验证
- POST 请求发送
- 成功/错误提示
- 自动跳转

// 样式系统
- Bootstrap 5 框架
- 自定义 CSS 动画
- Flexbox 响应式布局
- 深色导航栏主题
```

## 📈 数据库设计

```sql
CREATE TABLE posts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    content TEXT NOT NULL,
    timestamp INTEGER NOT NULL,
    user_id INTEGER,
    deleted_by_admin INTEGER NOT NULL DEFAULT 0,
    deleted_at INTEGER
);

-- 索引（可选优化）
CREATE INDEX idx_timestamp ON posts(timestamp DESC);
```

## 🔄 API 工作流

```
客户端（浏览器）
    ↓
[HTTP 请求]
    ↓
Crow 服务器 (端口 8080)
    ↓
    ├─ 静态文件: /index.html, /editor.html
    │   └─ 返回 HTML/JS/CSS
    │
    └─ API 路由
        ├─ GET /api/posts
        │   └─ Database::getAllPosts() → JSON
        ├─ GET /api/posts/{id}
        │   └─ Database::getPostById() → JSON
        ├─ POST /api/posts
        │   └─ Database::insertPost() → JSON
        ├─ PUT /api/posts/{id}
        │   └─ Database::updatePost() → JSON
        └─ DELETE /api/posts/{id}
            └─ Database::deletePost()/softDeletePost() → JSON
    ↓
SQLite 数据库 (blog.db)
    ├─ 读写
    └─ 事务处理
    ↓
[JSON 响应]
    ↓
客户端（刷新页面）
```

## 📱 用户流程

### 发布文章
```
1. 打开浏览器 → http://localhost:8080/editor.html
2. 输入标题和内容
3. 点击"发布文章"
4. JavaScript 发送 POST /api/posts
5. 后端验证 → 插入数据库 → 返回 JSON
6. 前端显示成功提示 → 自动跳转到首页
```

### 阅读文章
```
1. 打开首页 → http://localhost:8080/index.html
2. JavaScript 异步加载 GET /api/posts
3. 渲染文章卡片列表
4. 点击"阅读更多"
5. 发送 GET /api/posts/{id}
6. 模态框显示完整内容
```

## 💡 代码亮点

### 1. 数据库参数化查询
```cpp
sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
// 防止 SQL 注入
```

### 2. JSON 序列化
```cpp
json to_json() const {
    return json{
        {"id", id},
        {"title", title},
        {"content", content},
        {"timestamp", timestamp}
    };
}
```

### 3. CORS 跨域支持
```cpp
res.add_header("Access-Control-Allow-Origin", "*");
res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
```

### 4. 异步数据加载
```javascript
async function loadPosts() {
    const response = await fetch('/api/posts');
    const result = await response.json();
    // 动态渲染 DOM
}
```

##  安全考虑

1. **SQL 注入防护**: 使用参数化查询
2. **XSS 防护**: HTML 转义
3. **CORS 配置**: 明确的跨域头部
4. **路径遍历防护**: 静态文件路径检查

---

**项目完成时间**: 2026-02-06
