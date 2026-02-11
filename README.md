# 📚 README

本仓库是一个带认证与权限控制的简易博客系统：

- 产品形态（PRD/规格）：[PRODUCT_OVERVIEW.md](PRODUCT_OVERVIEW.md)
- 30 秒入口：[QUICKSTART.md](QUICKSTART.md)

## 文档导航

### 入口与概览

- 产品形态全貌（规则/边界/权限矩阵）：[PRODUCT_OVERVIEW.md](PRODUCT_OVERVIEW.md)
- 工程架构与代码结构：[project-overview.md](docs/architecture/project-overview.md)

### 构建与运行

- 构建与运行详述：[build-and-run.md](docs/build/build-and-run.md)

### 功能规格（按功能拆分）

- 认证
  - [login-register.md](docs/auth/login-register.md)
  - [token-session.md](docs/auth/token-session.md)
- 文章
  - [write.md](docs/posts/write.md)
  - [list.md](docs/posts/list.md)
  - [read.md](docs/posts/read.md)
  - [delete.md](docs/posts/delete.md)

### 测试脚本

- 认证/会话与受保护接口：`./test-auth.sh`
- 文章 CRUD（不含鉴权）：`./test-api.sh`

## 最小运行

```bash
./build.sh --run
```
