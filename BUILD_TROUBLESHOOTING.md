# 🔧 构建问题排查指南

## 问题：404 错误 - GitHub 资源下载失败

如果您看到这个错误：
```
The requested URL returned error: 404
process_pending_input: nghttp2_session_mem_recv() returned -902
```

### 原因分析

这个问题通常由以下原因引起：

1. **GitHub 发布页面 URL 变更** - Release 下载链接格式改变
2. **网络问题** - GitHub 连接不稳定或被限流
3. **CMake FetchContent 配置问题** - URL 格式不正确

### ✅ 解决方案

#### 方案 1：使用已修复的 CMakeLists.txt（推荐）

我已经更新了 `server/CMakeLists.txt`，使用更稳定的 GitHub Raw CDN 链接：

```bash
# 清理旧的构建目录
cd /Users/bytedance/Projects/Blog
rm -rf build

# 重新编译
./build.sh
```

#### 方案 2：如果仍然失败，使用本地依赖

如果网络问题仍然存在，可以手动下载依赖文件：

```bash
# 1. 创建依赖目录
mkdir -p build/deps

# 2. 下载 Crow
curl -L https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp \
     -o build/deps/crow_all.hpp

# 3. 下载 nlohmann/json
curl -L https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp \
     -o build/deps/json.hpp

# 4. 复制到 CMake 期望的位置
mkdir -p build/crow_include build/json_include/nlohmann
cp build/deps/crow_all.hpp build/crow_include/
cp build/deps/json.hpp build/json_include/nlohmann/
```

#### 方案 3：使用代理（如果在中国大陆）

如果网络访问 GitHub 有困难，可以配置 git 代理：

```bash
# 设置 git 代理 (使用国内镜像)
git config --global url."https://ghproxy.com/https://github.com".insteadOf "https://github.com"

# 或设置 curl 代理
export http_proxy=http://127.0.0.1:7890
export https_proxy=http://127.0.0.1:7890

# 然后重新编译
./build.sh
```

#### 方案 4：完全离线编译

如果您已经在其他机器上成功编译过，可以复制依赖文件：

```bash
# 从成功的编译中复制
cp -r /path/to/successful/build/crow_include build/
cp -r /path/to/successful/build/json_include build/
```

---

## 问题：Git 仓库错误

如果您看到：
```
fatal: not a git repository (or any of the parent directories): .git
```

### ✅ 解决方案

```bash
# 初始化 Git 仓库（仅需一次）
cd /Users/bytedance/Projects/Blog
git init

# 配置 Git 用户信息（仅需一次）
git config user.email "you@example.com"
git config user.name "Your Name"

# 添加所有文件
git add .

# 提交初始版本
git commit -m "Initial commit: Full-stack blog system"
```

现在您可以正常使用 git 命令了：
```bash
git status
git log
git branch
```

---

## 完整的修复步骤

如果以上方法都不行，按照以下步骤来：

```bash
# 1. 进入项目目录
cd /Users/bytedance/Projects/Blog

# 2. 清理旧的构建
rm -rf build
rm -rf CMakeCache.txt CMakeFiles/

# 3. 初始化/检查 Git
git init
git status  # 应该显示有 untracked files

# 4. 重新创建构建目录并编译
mkdir build
cd build

# 5. 使用 CMake 配置（添加详细输出）
cmake ../server -DCMAKE_VERBOSE_MAKEFILE=ON

# 如果上一步失败，尝试跳过 FetchContent 依赖检查
cmake ../server --debug-output 2>&1 | head -50

# 6. 编译
cmake --build . --verbose

# 如果编译仍然失败，手动下载依赖
if [ ! -f "crow_include/crow_all.hpp" ]; then
  mkdir -p crow_include json_include/nlohmann
  curl -L https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp \
       -o crow_include/crow_all.hpp
  curl -L https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp \
       -o json_include/nlohmann/json.hpp
fi

# 7. 重新运行 CMake
cmake ../server
cmake --build .
```

---

## 网络诊断

如果仍然遇到下载问题，运行以下命令诊断：

```bash
# 检查网络连接
ping github.com

# 检查 curl 是否能下载
curl -I https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp

# 检查 DNS
nslookup raw.githubusercontent.com

# 列出当前的 git 配置
git config --list

# 检查是否有代理设置
echo $http_proxy
echo $https_proxy
```

---

## 关于 GitHub 认证

**好消息：您不需要 GitHub 账户！**

- 下载公开项目的发行版本**不需要认证**
- 只有在私有仓库或推送代码时才需要认证

如果您想为项目贡献代码，可以配置 SSH 密钥：

```bash
# 生成 SSH 密钥
ssh-keygen -t ed25519 -C "your@email.com"

# 测试连接
ssh -T git@github.com

# 配置 git 使用 SSH
git config --global url."git@github.com:".insteadOf "https://github.com/"
```

---

## 验证修复成功

修复后，运行以下命令验证：

```bash
cd /Users/bytedance/Projects/Blog

# 1. 检查 Git 状态
git status  # 应该显示 On branch master

# 2. 清理构建目录
rm -rf build
./build.sh

# 3. 验证编译成功
ls -la build/bin/blog_server  # 应该存在

# 4. 测试运行
cd build
./bin/blog_server &
sleep 2

# 5. 测试 API
curl http://localhost:8080/api/posts

# 6. 停止服务
pkill blog_server
```

---

## 常见 CMake 错误及解决

### "file DOWNLOAD" 下载失败
```bash
# 增加超时时间
cmake ../server -DFETCHCONTENT_UPDATES_DISCONNECTED=OFF \
                -DCMAKE_DOWNLOAD_TIMEOUT=300
```

### 缓存导致的问题
```bash
# 清除 CMake 缓存
cd build
rm -rf CMakeCache.txt CMakeFiles/
cmake ../server
```

### 找不到依赖文件
```bash
# 检查文件是否存在
ls -la build/crow_include/
ls -la build/json_include/nlohmann/

# 如果不存在，手动创建
mkdir -p build/crow_include
mkdir -p build/json_include/nlohmann
```

---

## 📞 需要更多帮助？

如果问题仍未解决，请收集以下信息：

```bash
# 1. CMake 版本
cmake --version

# 2. GCC/Clang 版本
g++ --version
clang --version

# 3. 系统信息
uname -a
sw_vers  # macOS

# 4. 完整的编译错误日志
cd /Users/bytedance/Projects/Blog
rm -rf build
./build.sh 2>&1 | tee build.log
# 然后查看 build.log 文件
```

---

**祝编译成功！** 🚀
