# 📖 新的工作流程说明

## 核心设计

为了避免 CMake FetchContent 的网络问题，本项目采用了**分离的依赖管理方式**：

```
┌─────────────────────────────────────────────────────┐
│  用户工作流程                                        │
├─────────────────────────────────────────────────────┤
│                                                      │
│  步骤 1: 下载依赖                                   │
│  ┌──────────────────────────────────┐              │
│  │ ./download-deps.sh               │              │
│  │  ↓ 使用 curl 下载                │              │
│  │  • crow_all.hpp (Crow 框架)      │              │
│  │  • json.hpp (JSON 库)             │              │
│  │  ↓                                 │              │
│  │ build/deps/                       │              │
│  └──────────────────────────────────┘              │
│                     ↓                               │
│  步骤 2: 编译项目                                  │
│  ┌──────────────────────────────────┐              │
│  │ ./build.sh                       │              │
│  │  ↓ 检查依赖文件是否存在          │              │
│  │  ↓ cmake 配置                    │              │
│  │  ↓ cmake --build                 │              │
│  │  ↓                                │              │
│  │ build/bin/blog_server            │              │
│  └──────────────────────────────────┘              │
│                     ↓                               │
│  步骤 3: 运行服务器                                │
│  ┌──────────────────────────────────┐              │
│  │ cd build && ./bin/blog_server    │              │
│  │  ↓                                │              │
│  │ http://localhost:8080/           │              │
│  └──────────────────────────────────┘              │
│                                                      │
└─────────────────────────────────────────────────────┘
```

## 为什么要分离？

### 原问题（之前的方式）

```bash
./build.sh
  ↓
  CMake 尝试 FetchContent
    ↓
    [FAIL] 404 错误
    或
    [FAIL] 网络超时
    或
    [FAIL] GitHub 限流
```

**问题：** 编译失败时，难以判断是网络问题还是代码问题。

### 新方式的好处

```bash
# 依赖下载 (可以单独测试)
./download-deps.sh
  ↓
  [成功] 获得依赖文件
  或
  [失败] 网络错误（与编译无关）
  
# 编译 (依赖已准备)
./build.sh
  ↓
  [成功] 生成可执行文件
  或
  [失败] 代码错误（与网络无关）
```

**好处：**
1. **清晰的职责分离** - 下载和编译是独立的步骤
2. **易于排查问题** - 知道在哪一步失败
3. **可重复使用** - 下载一次，可以编译多次
4. **离线编译** - 依赖下载后可以断网编译

## 脚本说明

### download-deps.sh

**功能：** 从 GitHub 下载依赖库文件

**使用：**
```bash
./download-deps.sh
```

**做什么：**
1. 创建 `build/deps/` 目录结构
2. 使用 curl 下载 Crow 框架文件
3. 使用 curl 下载 nlohmann/json 文件
4. 验证下载成功

**如果失败：** 按照屏幕提示进行故障排除

**输出位置：**
```
build/deps/
├── crow_include/
│   └── crow_all.hpp
└── json_include/
    └── nlohmann/
        └── json.hpp
```

### build.sh

**功能：** 编译项目（纯编译，不下载依赖）

**使用：**
```bash
# 基本编译（Release 模式）
./build.sh

# 清理并重新编译
./build.sh --clean

# 编译 Debug 版本
./build.sh --debug

# 编译并自动运行服务器
./build.sh --run

# 查看帮助
./build.sh --help
```

**做什么：**
1. 检查依赖文件是否存在
2. 运行 CMake 配置
3. 编译源代码
4. 生成 `build/bin/blog_server`

**如果失败：** 检查是否先运行了 `./download-deps.sh`

## 完整工作流

### 首次构建

```bash
# 进入项目目录
cd /Users/bytedance/Projects/Blog

# 第 1 步: 下载依赖 (首次，约 1-2 分钟)
./download-deps.sh

# 验证依赖
ls -lh build/deps/*/

# 第 2 步: 编译 (首次，约 2-3 分钟)
./build.sh

# 第 3 步: 运行
cd build
./bin/blog_server
```

### 之后的编译

```bash
# 如果代码改变，需要重新编译
cd /Users/bytedance/Projects/Blog
./build.sh

# 如果编译缓存问题，清理后重新编译
./build.sh --clean

# 快速编译并测试
./build.sh --run
```

### 如果需要更新依赖

```bash
# 只需重新运行下载脚本
./download-deps.sh

# 然后重新编译
./build.sh --clean
```

## 故障排除快速表

| 问题 | 解决 |
|------|------|
| `❌ 缺少依赖文件` | `./download-deps.sh` |
| `❌ Crow 下载失败` | 检查网络，或手动下载 |
| `❌ CMake 配置失败` | 检查 CMake 版本和编译器 |
| `❌ 编译失败` | 查看错误信息，检查代码 |
| `Permission denied` | `chmod +x download-deps.sh build.sh` |

## 环境变量（可选）

如果在公司网络中，可能需要代理：

```bash
export http_proxy=http://proxy.example.com:8080
export https_proxy=http://proxy.example.com:8080

./download-deps.sh
```

## 总结

新工作流程的核心原则：

✅ **分离关注点** - 依赖管理和编译是独立的
✅ **快速反馈** - 立即知道问题在哪一步
✅ **可重复性** - 相同的步骤总是有相同的结果
✅ **离线支持** - 下载后可以断网工作
✅ **易于排查** - 清晰的错误提示和解决方案

如果有任何问题，请参考 `BUILD_TROUBLESHOOTING.md` 文档。
