# 🚀 简易博客系统 - 完整编译和运行指南

## 系统要求

- **操作系统**: macOS、Linux 或 Windows (MSVC/MinGW)
- **CMake**: 版本 3.10 或更高
- **C++ 编译器**: 支持 C++17 标准
  - macOS: Clang (Xcode Command Line Tools)
  - Linux: GCC 7+ 或 Clang
- **SQLite3**: 开发库

## 依赖安装

### macOS

```bash
# 安装 Xcode Command Line Tools（如果未安装）
xcode-select --install

# 使用 Homebrew 安装依赖
brew install cmake sqlite3

# 验证安装
cmake --version
sqlite3 --version
```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake sqlite3 libsqlite3-dev
```

### Linux (Fedora/RedHat)

```bash
sudo dnf install cmake sqlite-devel
```

## 项目结构

```
Blog/
├── server/
│   ├── src/
│   │   ├── main.cpp           # 主程序和 API 路由
│   │   └── database.cpp       # 数据库实现
│   ├── include/
│   │   └── database.h         # 数据库头文件
│   └── CMakeLists.txt         # CMake 构建配置
├── web/
│   ├── index.html             # 首页（文章列表）
│   ├── editor.html            # 编辑页（发布文章）
│   ├── js/
│   │   ├── index.js           # 首页逻辑
│   │   └── editor.js          # 编辑页逻辑
│   └── css/
│       └── style.css          # 样式文件
└── db/
    └── blog.db                # SQLite 数据库文件（编译后自动生成）
```

## 编译步骤

### 1. 进入项目目录

```bash
cd /Users/bytedance/Projects/Blog
```

### 2. 创建构建目录

```bash
mkdir -p build
cd build
```

### 3. 运行 CMake 配置

```bash
cmake ../server
```

**注意**: 第一次运行时，CMake 会自动从网络下载 Crow 框架和 nlohmann/json 库。这可能需要几分钟。

### 4. 编译项目

```bash
cmake --build . --config Release
```

或使用 `make`（更快）：

```bash
make -j$(nproc)
```

### 5. 验证编译

编译成功后，应该生成可执行文件：

```bash
./bin/blog_server
```

如果看到 "Blog server starting on http://0.0.0.0:8080"，说明编译成功！

## 运行服务器

### 启动后端服务

```bash
cd /Users/bytedance/Projects/Blog/build
./bin/blog_server
```

**预期输出**:
```
[2026-02-06 12:00:00] Database initialized successfully
[2026-02-06 12:00:00] Blog server starting on http://0.0.0.0:8080
```

### 访问前端页面

打开浏览器，访问以下 URL：

- **首页（文章列表）**: http://localhost:8080/index.html
- **发布文章**: http://localhost:8080/editor.html

## 功能测试

### 1. 发布文章

1. 访问 http://localhost:8080/editor.html
2. 输入标题和内容
3. 点击"发布文章"按钮
4. 成功后会自动跳转到首页

### 2. 查看文章列表

1. 访问 http://localhost:8080/index.html
2. 应该能看到已发布的文章列表
3. 点击"阅读更多"可查看完整内容

## API 接口文档

### 获取所有文章

```
GET /api/posts

响应示例:
{
  "success": true,
  "data": [
    {
      "id": 1,
      "title": "我的第一篇文章",
      "content": "这是内容...",
      "timestamp": 1707195000000000
    }
  ]
}
```

### 获取单篇文章

```
GET /api/posts/{id}

响应示例:
{
  "success": true,
  "data": {
    "id": 1,
    "title": "我的第一篇文章",
    "content": "这是内容...",
    "timestamp": 1707195000000000
  }
}
```

### 创建新文章

```
POST /api/posts
Content-Type: application/json

请求体:
{
  "title": "新文章标题",
  "content": "新文章内容"
}

响应示例:
{
  "success": true,
  "data": {
    "id": 2,
    "title": "新文章标题",
    "content": "新文章内容",
    "timestamp": 1707195100000000
  }
}
```

### 更新文章

```
PUT /api/posts/{id}
Content-Type: application/json

请求体:
{
  "title": "更新后的标题",
  "content": "更新后的内容"
}

响应示例:
{
  "success": true,
  "message": "Post updated"
}
```

### 删除文章

```
DELETE /api/posts/{id}

响应示例:
{
  "success": true,
  "message": "Post deleted"
}
```

## 问题排查

### 问题 1：CMake 找不到 SQLite3

**解决方案**:
```bash
# macOS
brew install sqlite3

# Linux
sudo apt-get install libsqlite3-dev

# 如果仍未找到，手动指定路径
cmake ../server -DSQLITE3_INCLUDE_DIR=/path/to/sqlite3/include
```

### 问题 2：编译失败，提示缺少头文件

**原因**: Crow 或 nlohmann/json 下载失败。

**解决方案**:
```bash
# 删除构建目录并重新开始
cd /Users/bytedance/Projects/Blog
rm -rf build
mkdir build
cd build
cmake ../server  # 重新下载依赖
cmake --build .
```

### 问题 3：端口 8080 已被占用

**错误信息**: "Address already in use"

**解决方案**:
1. 找到占用端口 8080 的进程：
```bash
lsof -i :8080
```

2. 杀死进程或修改 [main.cpp](server/src/main.cpp) 中的端口号，修改这一行：
```cpp
app.port(8080).multithreaded().run();  // 改为其他端口，如 8081
```

### 问题 4：前端无法访问 API

**原因**: CORS 问题或服务器未启动。

**排查步骤**:
1. 确保后端服务运行在 http://localhost:8080
2. 检查浏览器控制台是否有错误信息
3. 确保 API 响应包含正确的 CORS 头部

## 生产部署建议

### 1. 生成发布版本

```bash
cmake ../server -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### 2. 使用反向代理（nginx）

```nginx
server {
    listen 80;
    server_name example.com;

    location /api/ {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }

    location / {
        root /path/to/Blog/web;
        try_files $uri $uri/ /index.html;
    }
}
```

### 3. 后台运行服务

```bash
# 使用 nohup
nohup ./bin/blog_server > blog_server.log 2>&1 &

# 或使用 systemd（Linux）
# 创建 /etc/systemd/system/blog.service
[Unit]
Description=Blog Server
After=network.target

[Service]
Type=simple
ExecStart=/path/to/blog_server
WorkingDirectory=/path/to/Blog
Restart=always

[Install]
WantedBy=multi-user.target
```

## 技术栈总结

| 组件 | 技术 | 版本 |
|------|------|------|
| 后端框架 | Crow | v1.0+5 |
| 编程语言 | C++ | C++17 |
| 数据库 | SQLite | 3.x |
| JSON 库 | nlohmann/json | v3.11.2 |
| 前端框架 | Bootstrap | v5.3.0 |
| 构建工具 | CMake | 3.10+ |

## 常见优化

### 1. 添加分页功能

修改 `/api/posts` 端点以支持分页：

```cpp
app.get("/api/posts", [&db](const crow::request& req) {
    int page = 1, limit = 10;
    
    auto page_param = req.url_params.get("page");
    if (page_param) page = std::stoi(page_param);
    
    auto limit_param = req.url_params.get("limit");
    if (limit_param) limit = std::stoi(limit_param);
    
    // 修改 SQL 查询以支持 LIMIT 和 OFFSET
    // ...
});
```

### 2. 添加搜索功能

```cpp
app.get("/api/posts/search", [&db](const crow::request& req) {
    auto query = req.url_params.get("q");
    // 模糊搜索实现
    // ...
});
```

### 3. 添加标签/分类

扩展数据库表以支持标签。

## 许可证

MIT License

## 联系方式

如有问题，请查阅源代码或通过 GitHub Issues 反馈。
