# 🔧 构建问题排查指南

## 概述

本项目采用**手动依赖管理**的方式：
- **download-deps.sh**: 负责下载依赖文件
- **build.sh**: 负责编译项目

这种分离的设计使得故障排除更加清晰和可控。

---

## 问题 1: download-deps.sh 下载失败

### 症状

运行 `./download-deps.sh` 时看到：
```
❌ Crow 下载失败
❌ JSON 下载失败
```

### 原因分析

1. **网络连接问题** - 无法连接到 raw.githubusercontent.com
2. **GitHub 访问限制** - IP 被限流或地理限制
3. **代理问题** - 使用了不兼容的代理

### ✅ 解决方案

#### 方案 1：检查网络连接

```bash
# 测试是否能连接到 GitHub
curl -I https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp
```

如果超时或 403，说明网络访问受限。

#### 方案 2：使用代理（如需要）

```bash
# 如果使用代理，设置环境变量
export http_proxy=http://proxy.example.com:8080
export https_proxy=http://proxy.example.com:8080

# 然后重新运行
./download-deps.sh
```

#### 方案 3：手动下载

如果自动下载失败，可以手动下载并放置在正确位置：

```bash
# 1. 创建目录结构
mkdir -p build/deps/crow_include
mkdir -p build/deps/json_include/nlohmann

# 2. 手动下载 Crow
# 访问这个 URL 并保存文件:
# https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp
# 保存到: build/deps/crow_include/crow_all.hpp

# 3. 手动下载 nlohmann/json
# 访问这个 URL 并保存文件:
# https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp
# 保存到: build/deps/json_include/nlohmann/json.hpp

# 4. 验证文件
ls -lh build/deps/crow_include/crow_all.hpp
ls -lh build/deps/json_include/nlohmann/json.hpp
```

#### 方案 4：使用浏览器下载

1. 打开浏览器访问以下 URL
2. 右键"另存为"保存文件到正确位置
3. 文件位置必须匹配下表：

| 文件 | 下载 URL | 保存位置 |
|------|----------|----------|
| Crow | https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp | `build/deps/crow_include/crow_all.hpp` |
| JSON | https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp | `build/deps/json_include/nlohmann/json.hpp` |

---

## 问题 2: build.sh 报错 "缺少依赖文件"

### 症状

```
❌ 缺少依赖文件

  • /Users/bytedance/Projects/Blog/build/deps/crow_include/crow_all.hpp
  • /Users/bytedance/Projects/Blog/build/deps/json_include/nlohmann/json.hpp

解决方案: 先运行依赖下载脚本
  ./download-deps.sh
```

### 解决方案

这是一个友好的提示，告诉你依赖还未下载。按照提示操作：

```bash
# 1. 下载依赖
./download-deps.sh

# 2. 验证文件确实存在
ls -lh build/deps/crow_include/crow_all.hpp
ls -lh build/deps/json_include/nlohmann/json.hpp

# 3. 然后编译
./build.sh
```

---

## 问题 3: CMake 配置失败

### 症状

```
❌ CMake 配置失败
```

### 原因和解决方案

#### 原因 1: CMake 版本过低

```bash
# 检查 CMake 版本（需要 3.10+）
cmake --version

# 如果版本过低，升级 CMake
brew upgrade cmake
```

#### 原因 2: 缺少编译器

```bash
# 检查是否安装了 Xcode Command Line Tools
xcode-select --install

# 或使用
gcc --version
```

#### 原因 3: SQLite3 未安装

```bash
# 检查 SQLite3
sqlite3 --version

# 如果未安装
brew install sqlite3
```

#### 原因 4: CMakeLists.txt 语法错误

检查 `server/CMakeLists.txt` 是否被意外修改。如果不确定，可以查看该文件。

---

## 问题 4: 编译错误 (C++ 代码)

### 症状

```
❌ 编译失败

[错误信息...]
```

### 原因分析

常见的编译错误：

1. **C++ 标准版本问题** - 需要 C++17 或更高

   ```bash
   # 检查编译器是否支持 C++17
   clang++ --version
   ```

2. **头文件找不到**

   确保依赖文件格式正确：
   ```bash
   # 检查文件大小（不应该很小）
   ls -lh build/deps/crow_include/crow_all.hpp
   # 应该显示 > 1MB
   
   ls -lh build/deps/json_include/nlohmann/json.hpp
   # 应该显示 > 500KB
   ```

3. **SQLite3 库未找到**

   ```bash
   # 确保 SQLite3 已安装
   brew install sqlite3
   
   # 重新编译
   cd /Users/bytedance/Projects/Blog
   rm -rf build
   ./build.sh
   ```

---

## 问题 5: 运行时错误

### 症状 1: 数据库初始化失败

```
[ERROR] Failed to initialize database
```

**解决:**
```bash
# 删除损坏的数据库文件
rm -f build/blog.db

# 重新运行服务器（会自动创建新数据库）
./bin/blog_server
```

### 症状 2: 端口已被占用

```
[ERROR] Address already in use
```

**解决:**
```bash
# 查找占用 8080 端口的进程
lsof -i :8080

# 杀死该进程（替换 PID）
kill -9 <PID>

# 或改用其他端口（需要修改源代码）
```

### 症状 3: 无法从浏览器访问

```
ERR_CONNECTION_REFUSED
```

**解决:**
```bash
# 确保服务器正在运行
ps aux | grep blog_server

# 尝试用 curl 测试
curl http://localhost:8080/index.html

# 检查防火墙设置
```

---

## 问题 6: 文件权限问题

### 症状

```
Permission denied: ./build.sh
Permission denied: ./download-deps.sh
```

### 解决方案

```bash
# 给脚本添加执行权限
chmod +x download-deps.sh
chmod +x build.sh

# 然后重新运行
./download-deps.sh
./build.sh
```

---

## 完整的"核弹级"解决方案

如果上述所有方法都不奏效，使用核弹级解决方案：

```bash
# 1. 清理所有构建文件
cd /Users/bytedance/Projects/Blog
rm -rf build
rm -rf db/*.db

# 2. 检查工具
brew install cmake sqlite3

# 3. 从头开始
chmod +x download-deps.sh build.sh

# 4. 步骤 1: 下载依赖
./download-deps.sh

# 验证文件
ls -lh build/deps/crow_include/crow_all.hpp
ls -lh build/deps/json_include/nlohmann/json.hpp

# 5. 步骤 2: 编译
./build.sh --clean

# 6. 步骤 3: 运行
cd build
./bin/blog_server
```

---

## 获取帮助

如果遇到这里未覆盖的问题，可以：

1. **查看完整日志** - 运行时保存输出：
   ```bash
   ./download-deps.sh > download.log 2>&1
   ./build.sh > build.log 2>&1
   cat build.log
   ```

2. **检查日期** - 有些 GitHub 链接可能会过期，届时需要更新 URL

3. **参考文档** - 查看项目中的其他文档：
   - README.md - 项目概述
   - QUICKSTART.md - 快速开始
   - PROJECT_OVERVIEW.md - 项目架构
