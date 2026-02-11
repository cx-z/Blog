# Project Summary

**项目名称**：简易全栈博客系统  
**完成日期**：2026-02-06  
**形态**：C++17（Crow）后端 + SQLite + 静态前端页面（web/）

## 这份文档的定位

本文件只保留“项目结果与能力边界”的汇总信息，避免重复维护接口细节与构建细节：

- 端到端入口：../../QUICKSTART.md
- 构建、运行、API 汇总与排错：../../README.md
- 代码结构与核心流程：../architecture/project-overview.md
- 认证：../auth/login-register.md、../auth/token-session.md
- 文章：../posts/write.md、../posts/list.md、../posts/read.md、../posts/delete.md

## 核心能力（面向产品）

- 账号体系：注册、登录、JWT 无状态会话、Token 校验与过期
- 文章能力：创建、编辑、列表、阅读、删除
- 权限与删除语义：
  - 普通用户：仅能访问自己的文章
  - 管理员：可查看所有文章；删除他人文章为软删除（保留记录并标记）
- 安全基线：参数化查询（SQL 注入防护）、HTML 转义（XSS 防护）、基础 CORS 配置

## 交付物（仓库内）

### 脚本

- build.sh：编译脚本（详见 README）
- test-auth.sh：认证/会话与受保护接口的集成测试脚本
- test-api.sh：文章 CRUD 的 API 测试脚本（不含鉴权）

### 前端页面

- web/login.html、web/register.html：登录/注册
- web/index.html：列表/阅读/删除入口
- web/editor.html：创建/编辑入口

## 验收清单（最小闭环）

- 服务器可启动并提供静态页面
- 注册/登录成功后能拿到并保存 Token
- 发布文章后首页列表可见
- 能阅读文章详情
- 作者可删除自己的文章
- 管理员可软删他人文章，作者仍可见但不可编辑

## 可选扩展方向

- 刷新令牌（Refresh Token）与服务端撤销机制
- 列表分页、搜索、筛选与摘要字段
- 公开/私密文章模型与分享链接
- 审计日志（管理员操作记录）
