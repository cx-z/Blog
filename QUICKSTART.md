# 🚀 QUICKSTART（30 秒入口）

## 0. 先把服务跑起来

先按 [README.md](README.md) 完成依赖安装、编译与启动。

## 1. 30 秒验收路径

1. 打开 http://localhost:8080/ （未登录会跳转到登录页）
2. 注册一个新账号并登录
3. 进入发布页 http://localhost:8080/editor.html ，发布一篇文章
4. 回到首页 http://localhost:8080/index.html ，确认能看到文章列表
5. 点击“阅读更多”，确认能打开文章详情

## 2. 文档导航（按需深入）

### 认证与会话

- 登录/注册：docs/auth/login-register.md
- Token 会话（verify/过期/登出/多设备）：docs/auth/token-session.md

### 文章功能

- 写文章（创建/编辑）：docs/posts/write.md
- 文章列表：docs/posts/list.md
- 阅读文章：docs/posts/read.md
- 删除文章（含管理员软删语义）：docs/posts/delete.md

### 其他

- 项目概览与代码结构：docs/architecture/project-overview.md
- 产品形态总览（PRD/规格）：PRODUCT_OVERVIEW.md
- 构建与运行详述：docs/build/build-and-run.md
