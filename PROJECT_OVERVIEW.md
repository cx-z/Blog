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
│   │   └── database.h               # 数据库头文件 (48 行)
│   │       ├── Post 数据结构
│   │       └── Database 类接口
│   └── CMakeLists.txt               # CMake 构建配置
│
├── web/                             # 前端
│   ├── index.html                   # 首页（文章列表）
│   │   └── Bootstrap 5 + 自定义样式
│   ├── editor.html                  # 发布页（编辑器）
│   │   └── 表单验证 + API 交互
│   ├── js/
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
│   └── bin/
│       └── blog_server              # 编译后的可执行文件
│
├── README.md                        # 完整编译运行指南
├── build.sh                         # 快速编译脚本
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
    timestamp INTEGER NOT NULL
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
            └─ Database::deletePost() → JSON
    ↓
SQLite 数据库 (blog.db)
    ├─ 读写
    └─ 事务处理
    ↓
[JSON 响应]
    ↓
客户端（刷新页面）
```

## 🚀 编译流程

1. **CMake 配置阶段**
   - 查找 CMake、C++ 编译器、SQLite3
   - 下载 Crow 和 nlohmann/json (首次)
   - 生成 Makefile/Xcode 工程

2. **编译阶段**
   - 编译 database.cpp (SQLite 操作)
   - 编译 main.cpp (Web 服务器)
   - 链接 SQLite3 库
   - 输出可执行文件 → bin/blog_server

3. **运行阶段**
   - 初始化数据库 (blog.db)
   - 创建 posts 表
   - 启动 Web 服务器
   - 监听端口 8080

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

## 🎯 关键特性

| 特性 | 实现 |
|------|------|
| **RESTful API** | Crow 路由完整支持 |
| **CORS** | OPTIONS 预检 + 响应头 |
| **错误处理** | try-catch + JSON 错误响应 |
| **数据验证** | 前端表单验证 + 后端参数检查 |
| **时间戳** | chrono 库获取精确微秒时间 |
| **响应式设计** | Bootstrap + 媒体查询 |
| **动画效果** | CSS keyframes + 过渡效果 |
| **HTML 转义** | XSS 防护 |

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

## 📝 文件大小统计

| 文件 | 行数 | 大小 |
|------|------|------|
| main.cpp | 255 | ~9 KB |
| database.cpp | 203 | ~6 KB |
| database.h | 48 | ~1.5 KB |
| index.html | ~80 | ~3 KB |
| editor.html | ~70 | ~3 KB |
| index.js | 73 | ~2.5 KB |
| editor.js | 48 | ~1.5 KB |
| style.css | 226 | ~8 KB |
| **总计** | **1,000+** | **~35 KB** |

## 🔐 安全考虑

1. **SQL 注入防护**: 使用参数化查询
2. **XSS 防护**: HTML 转义
3. **CORS 配置**: 明确的跨域头部
4. **路径遍历防护**: 静态文件路径检查

## 📦 部署建议

- **开发**: 直接运行 `./bin/blog_server`
- **生产**: 使用 Nginx 反向代理
- **数据库**: 定期备份 blog.db
- **日志**: 重定向到文件: `./blog_server > blog.log 2>&1 &`

---

**项目完成时间**: 2026-02-06
**总代码行数**: ~1000 行
**依赖数量**: 3 个 (Crow, nlohmann/json, SQLite3)
**编译时间**: ~2-3 分钟 (首次含依赖下载)
