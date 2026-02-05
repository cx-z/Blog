# 🚀 快速启动指南 (3分钟上手)

## 前置要求

✅ macOS 系统
✅ Xcode Command Line Tools
✅ CMake 3.10+
✅ SQLite3

## ⚡ 快速启动 (3 步)

### 步骤 1: 安装依赖 (仅首次)

```bash
# macOS 用户
brew install cmake sqlite3
```

### 步骤 2: 编译项目 (2-3 分钟)

```bash
cd /Users/bytedance/Projects/Blog
chmod +x build.sh
./build.sh
```

**预期输出:**
```
========================================
  简易博客系统 - 快速启动脚本
==========================================
✓ 依赖检查完成
📁 创建构建目录...
🔧 运行 CMake 配置...
🔨 编译项目...
==========================================
  ✓ 编译完成！
```

### 步骤 3: 启动服务器 (1 秒)

```bash
cd /Users/bytedance/Projects/Blog/build
./bin/blog_server
```

**预期输出:**
```
[2026-02-06 12:00:00] Database initialized successfully
[2026-02-06 12:00:00] Blog server starting on http://0.0.0.0:8080
```

✅ **完成！** 服务器已启动

---

## 🌐 访问博客

打开浏览器，访问以下地址：

| 页面 | URL | 功能 |
|------|-----|------|
| 首页 | http://localhost:8080/index.html | 查看文章列表 |
| 发布 | http://localhost:8080/editor.html | 发布新文章 |

### 测试流程

1. **首次打开首页**
   - 访问 http://localhost:8080/index.html
   - 显示"还没有文章"（数据库为空）

2. **发布测试文章**
   - 点击"发布文章"或访问 http://localhost:8080/editor.html
   - 输入标题: `我的第一篇博客`
   - 输入内容: `这是用 C++ Crow 和 SQLite 构建的博客系统！`
   - 点击"发布文章"按钮

3. **返回首页查看**
   - 自动跳转回首页
   - 看到新发布的文章
   - 点击"阅读更多"查看完整内容

---

## 📁 项目结构速览

```
Blog/
├── server/                # C++ 后端 (Crow 框架)
│   ├── src/main.cpp       # Web 服务器 + API
│   ├── src/database.cpp   # SQLite 数据库操作
│   └── CMakeLists.txt     # 构建配置
├── web/                   # 前端 (HTML/CSS/JS)
│   ├── index.html         # 首页
│   ├── editor.html        # 发布页
│   ├── js/                # JavaScript 逻辑
│   └── css/               # 样式文件
├── db/                    # 数据库文件夹
│   └── blog.db            # SQLite 数据库
└── build/                 # 编译输出 (自动生成)
    └── bin/blog_server    # 可执行程序
```

---

## 🔧 常见问题排查

### ❌ 问题: "CMake: command not found"

```bash
# 解决: 安装 CMake
brew install cmake
```

### ❌ 问题: "SQLite3 not found"

```bash
# 解决: 安装 SQLite3
brew install sqlite3
```

### ❌ 问题: 404 错误 - GitHub 资源下载失败

**The requested URL returned error: 404**

这通常是因为 GitHub 连接问题。查看完整的解决方案：

👉 [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md) ⭐ **详细故障排除指南**

快速解决：
```bash
# 方案 1：清理并重试
rm -rf build
./build.sh

# 方案 2：如果仍然失败，手动下载依赖
mkdir -p build/{crow_include,json_include/nlohmann}
curl -L https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp \
     -o build/crow_include/crow_all.hpp
curl -L https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp \
     -o build/json_include/nlohmann/json.hpp
```

### ❌ 问题: "Address already in use"

端口 8080 被占用。杀死占用的进程：

```bash
# 查找占用端口的进程
lsof -i :8080

# 杀死进程 (PID 替换为实际的 PID)
kill -9 <PID>

# 或修改 server/src/main.cpp 中的端口号
# 修改这一行: app.port(8080).multithreaded().run();
```

### ❌ 问题: 前端无法连接到后端

```bash
# 确保:
1. 后端服务正在运行 (./bin/blog_server)
2. 访问的 URL 正确 (http://localhost:8080)
3. 检查浏览器控制台是否有错误信息
```

### ❌ 编译失败: "nlohmann/json" 或 "Crow" 不存在

```bash
# 解决: 重新清理和编译
cd /Users/bytedance/Projects/Blog
rm -rf build
./build.sh
```

### ❌ Git 仓库问题: "not a git repository"

```bash
# 初始化 Git 仓库
cd /Users/bytedance/Projects/Blog
git init
git config user.email "you@example.com"
git config user.name "Your Name"
git add .
git commit -m "Initial commit"
```

---

### 📖 需要更多帮助？

- **详细故障排除** → 查看 [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md)
- **完整编译指南** → 查看 [README.md](README.md)
- **项目概览** → 查看 [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md)

---

## 🎨 进阶操作

### 查看数据库内容

```bash
# 连接到数据库
sqlite3 /Users/bytedance/Projects/Blog/db/blog.db

# 查看所有表
.tables

# 查看文章
SELECT * FROM posts;

# 退出
.quit
```

### 修改服务器端口

编辑 [server/src/main.cpp](server/src/main.cpp)，找到最后一行：

```cpp
app.port(8080).multithreaded().run();  // 改为其他端口，如 3000
```

修改后需要重新编译：

```bash
cd /Users/bytedance/Projects/Blog/build
cmake ../server
cmake --build .
```

### 添加新功能

例如，添加文章搜索功能，可以：

1. 在 [database.h](server/include/database.h) 中添加方法声明
2. 在 [database.cpp](server/src/database.cpp) 中实现
3. 在 [main.cpp](server/src/main.cpp) 中添加 API 路由
4. 在前端 JavaScript 中调用新 API

---

## 📊 技术栈总结

| 层 | 技术 | 说明 |
|----|------|------|
| **后端** | C++ 17 + Crow | 轻量级 Web 框架 |
| **数据库** | SQLite 3 | 嵌入式数据库，无需服务器 |
| **前端** | HTML5 + CSS3 + JS | 原生实现，无框架依赖 |
| **UI 框架** | Bootstrap 5 | CDN 引入，快速美化 |
| **构建工具** | CMake 3.10+ | 跨平台编译 |
| **总代码量** | 948 行 | 轻量级项目 |

---

## 📚 详细文档

- [完整编译指南](README.md) - 详细的依赖安装和故障排除
- [项目概览](PROJECT_OVERVIEW.md) - 架构设计和代码分析
- [API 文档](README.md#-api-接口文档) - RESTful API 详细说明

---

## 💡 下一步建议

1. ✅ 成功运行后端和前端
2. 💻 尝试发布和浏览文章
3. 📝 查看源代码理解实现原理
4. 🔧 添加新功能 (搜索、分类、标签等)
5. 🚀 部署到服务器 (Nginx + systemd)

---

## 支持

遇到问题？
1. 查看 [README.md](README.md) 的"问题排查"章节
2. 检查浏览器开发者工具的控制台错误信息
3. 确保所有依赖都正确安装

---

**祝您使用愉快！** 🎉
