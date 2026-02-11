# Build & Run

本文件包含构建与运行的详细说明；README.md 作为入口与导航。

---

## 1. 系统要求

- 操作系统：macOS / Linux / Windows（MSVC/MinGW）
- CMake：3.10+
- C++ 编译器：支持 C++17
- SQLite3：开发库（头文件与链接库）
- OpenSSL：开发库（用于加密与 JWT）

## 2. 依赖安装

### macOS（Homebrew）

```bash
brew install cmake sqlite3 openssl asio
```

说明：

- OpenSSL 在 macOS 上常见安装路径是 `/opt/homebrew/opt/openssl@3`
- 本工程还在 CMake 中固定引用了 `/opt/homebrew/opt/asio/include`

### Linux（Ubuntu/Debian）

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake sqlite3 libsqlite3-dev libssl-dev
```

### Linux（Fedora/RedHat）

```bash
sudo dnf install cmake sqlite-devel openssl-devel
```

## 3. 构建

### 方式 A：脚本构建（推荐）

```bash
./build.sh
```

常用选项：

```bash
./build.sh --clean
./build.sh --debug
./build.sh --run
```

编译输出：

- 可执行文件：`build/bin/blog_server`

### 方式 B：手动 CMake（不推荐）

```bash
mkdir -p build
cd build
cmake ../server
cmake --build . --config Release
./bin/blog_server
```

## 4. 运行

### 启动服务器

```bash
cd build && ./bin/blog_server
```

### 访问页面

- 首页（文章列表）：http://localhost:8080/index.html
- 编辑器（创建/编辑）：http://localhost:8080/editor.html
- 登录：http://localhost:8080/login.html
- 注册：http://localhost:8080/register.html

## 5. 构建问题排查

### CMake 找不到 SQLite3

macOS：

```bash
brew install sqlite3
```

Linux：

```bash
sudo apt-get install libsqlite3-dev
```

必要时可在 CMake 配置阶段指定 include 目录（示例）：

```bash
cmake ../server -DSQLITE3_INCLUDE_DIR=/path/to/sqlite3/include
```

### 端口 8080 已被占用

```bash
lsof -i :8080
```

或者修改 [main.cpp](../../server/src/main.cpp) 的端口配置并重新编译。

### 前端无法访问 API

排查步骤：

1. 确保后端服务运行在 http://localhost:8080
2. 检查浏览器控制台是否有错误信息
3. 确认 API 响应包含 CORS 头

## 6. 生产部署（可选）

### Release 构建

```bash
cd build
cmake ../server -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### 使用反向代理（nginx）

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

### 后台运行服务

```bash
nohup ./bin/blog_server > blog_server.log 2>&1 &
```

或使用 systemd（Linux）管理进程（按需配置 service 文件）。
