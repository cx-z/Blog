# ⚡ 快速参考卡

## 常用命令

### 首次设置（一次性）

```bash
cd /Users/bytedance/Projects/Blog

# 1. 下载依赖库（1-2 分钟）
./download-deps.sh

# 2. 编译项目（2-3 分钟）
./build.sh
```

### 日常使用

```bash
# 编译（快速，2-3 分钟）
./build.sh

# 编译并运行
./build.sh --run

# 清理重新编译
./build.sh --clean

# 编译 Debug 版本
./build.sh --debug
```

### 运行服务器

```bash
cd build
./bin/blog_server

# 或从项目根目录
./build.sh --run
```

### 访问博客

```
首页：http://localhost:8080/index.html
发布：http://localhost:8080/editor.html
```

---

## 故障排除 30 秒速查

| 错误 | 原因 | 解决 |
|------|------|------|
| `❌ 缺少依赖文件` | 未下载依赖 | `./download-deps.sh` |
| `❌ 编译失败` | 代码问题 | 检查错误信息 |
| `❌ 命令找不到` | 权限问题 | `chmod +x *.sh` |
| `ERR_CONNECTION_REFUSED` | 服务器未运行 | `./build.sh --run` |

---

## 文件夹说明

```
📁 Blog/
├── 📁 server/          # C++ 后端代码
├── 📁 web/             # HTML/CSS/JS 前端
├── 📁 build/           # 编译输出（自动生成）
│   ├── bin/            # 可执行文件
│   └── deps/           # 依赖库文件（download-deps.sh）
├── 📁 db/              # 数据库存放（自动生成）
└── 📄 *.sh             # 脚本文件
```

---

## 核心脚本说明

### download-deps.sh
```bash
# 功能：下载依赖库文件
# 时间：1-2 分钟
# 使用：./download-deps.sh
```

### build.sh
```bash
# 功能：编译项目
# 选项：--clean, --debug, --run
# 使用：./build.sh [--run]
```

---

## 配置位置

| 项 | 位置 |
|----|------|
| API 端口 | server/src/main.cpp |
| 数据库位置 | db/blog.db |
| 前端地址 | web/*.html |
| 构建参数 | server/CMakeLists.txt |

---

## 数据库问题
```bash
# 重置数据库（删除所有文章）
rm -f db/blog.db
./build.sh --run       # 会自动创建新数据库
```

---

## 环境检查

```bash
# 确保所有工具已安装
cmake --version        # 需要 >= 3.10
sqlite3 --version      # 已安装
clang++ --version      # C++ 编译器

# 检查脚本权限
ls -l build.sh download-deps.sh

# 检查依赖
ls -lh build/deps/crow_include/crow_all.hpp
ls -lh build/deps/json_include/nlohmann/json.hpp
```

---

## 安装依赖（macOS）

```bash
brew install cmake sqlite3 asio
```

---

## 更多帮助

```bash
# 查看 build.sh 选项
./build.sh --help

# 查看完整故障排除
cat BUILD_TROUBLESHOOTING.md

# 查看工作流程
cat WORKFLOW.md
```
# ⚡ 快速参考卡

## 常用命令

### 首次设置（一次性）

```bash
cd /Users/bytedance/Projects/Blog

# 1. 下载依赖库（1-2 分钟）
./download-deps.sh

# 2. 编译项目（2-3 分钟）
./build.sh
```

### 日常使用

```bash
# 编译（快速，2-3 分钟）
./build.sh

# 编译并运行
./build.sh --run

# 清理重新编译
./build.sh --clean

# 编译 Debug 版本
./build.sh --debug
```

### 运行服务器

```bash
cd /Users/bytedance/Projects/Blog

# 1. 下载依赖库（1-2 分钟）
./download-deps.sh

# 2. 编译项目（2-3 分钟）
./build.sh
```
```bash
cd build
./bin/blog_server

# 或从项目根目录
./build.sh --run
```

### 访问博客

```
首页：http://localhost:8080/index.html
发布：http://localhost:8080/editor.html
```

---

## 故障排除 30 秒速查

| 错误 | 原因 | 解决 |
|------|------|------|
| `❌ 缺少依赖文件` | 未下载依赖 | `./download-deps.sh` |
| `❌ 编译失败` | 代码问题 | 检查错误信息 |
| `❌ 命令找不到` | 权限问题 | `chmod +x *.sh` |
| `ERR_CONNECTION_REFUSED` | 服务器未运行 | `./build.sh --run` |

---

## 文件夹说明

```
📁 Blog/
├── 📁 server/          # C++ 后端代码
├── 📁 web/             # HTML/CSS/JS 前端
│   ├── bin/            # 可执行文件
│   └── deps/           # 依赖库文件（下载-deps.sh）
├── 📁 db/              # 数据库存放（自动生成）
└── 📄 *.sh             # 脚本文件
```

---

## 核心脚本说明

### download-deps.sh
```bash
# 功能：下载依赖库文件
# 时间：1-2 分钟
# 使用：./download-deps.sh
```

### build.sh
```bash
rm -f db/blog.db
./build.sh --run       # 会自动创建新数据库
```
# 使用：./build.sh [--run]
```
```bash
brew install cmake sqlite3 asio
```
## 配置位置

| 项 | 位置 |
|----|------|
| API 端口 | server/src/main.cpp (line ~40) |
| 数据库位置 | server/src/main.cpp (line ~20) |
| 前端地址 | web/*.html |
| 构建参数 | server/CMakeLists.txt |

---

## 详细文档

- 📖 **QUICKSTART.md** - 5 分钟快速启动
- 📖 **WORKFLOW.md** - 工作流程详解
- 📖 **BUILD_TROUBLESHOOTING.md** - 问题排查
- 📖 **README.md** - 项目概述
- 📖 **PROJECT_OVERVIEW.md** - 项目架构

---

## 开发小贴士

### 只改前端（不需要重新编译）
```bash
# 修改 web/ 目录中的文件
# 刷新浏览器即可看到效果
```

### 修改后端代码
```bash
cd /Users/bytedance/Projects/Blog
./build.sh --run       # 快速编译并运行
```

### 调试编译问题
```bash
# 查看完整编译输出
cd build
cmake --build . --verbose

# 查看 CMake 配置
cmake -L ..
```

### 数据库问题
```bash
# 重置数据库（删除所有文章）
rm -f db/blog.db
./build.sh --run       # 会自动创建新数据库
```

---

## 网络故障排除

### 如果 download-deps.sh 失败

```bash
# 1. 检查网络
ping github.com

# 2. 如果有代理
export http_proxy=http://proxy:8080
./download-deps.sh

# 3. 手动下载（参考 BUILD_TROUBLESHOOTING.md）
```

---

## 性能提示

```bash
# 快速增量编译（只编译改变的部分）
./build.sh

# 完整重编（清理所有对象文件）
./build.sh --clean

# 并行编译（充分利用多核 CPU）
# 自动启用（--parallel）
```

---

## 环境检查

```bash
# 确保所有工具已安装
cmake --version        # 需要 >= 3.10
sqlite3 --version      # 已安装
clang++ --version      # C++ 编译器

# 检查脚本权限
ls -l build.sh download-deps.sh

# 检查依赖
ls -lh build/deps/crow_include/crow_all.hpp
ls -lh build/deps/json_include/nlohmann/json.hpp
```

---

## 更多帮助

```bash
# 查看 build.sh 选项
./build.sh --help

# 查看完整故障排除
cat BUILD_TROUBLESHOOTING.md

# 查看工作流程
cat WORKFLOW.md
```
