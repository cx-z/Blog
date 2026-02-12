# 测试指引

本工程提供一键测试入口，用于在本机修改代码后验证服务端与数据库接口行为稳定，再进行手动发布。

## 目录结构

- 根目录入口脚本：`./test.sh`
- 测试代码目录：`./test/`
  - `test-e2e.py`：启动服务后跑 HTTP 集成测试
  - `test_database.cpp`：数据库接口集成测试（通过 CTest 运行）
- 测试快照数据库：`./db/sanitized.db`

## 一键执行

在工程根目录执行：

```bash
bash ./test.sh
```

脚本会依次执行：

1. Debug 构建（调用 `./build.sh --debug`）
2. 数据库接口测试（在 `build/` 下运行 `ctest`）
3. 启动服务（使用独立端口与独立 SQLite 临时库）
4. 执行 HTTP 集成测试（`test/test-e2e.py`）

任何一步失败都会返回非 0 退出码。

## 测试账号与快照

默认使用如下白名单账号（在 `test.sh` 内配置）：

- 管理员：`Account1 / 123456`
- 普通用户：`Account3 / 123456`

默认使用 `db/sanitized.db` 作为脱敏快照数据库（脚本会复制到临时路径再运行，避免污染快照文件）。

如需修改测试账号或快照库路径，直接编辑 [test.sh](file:///Users/bytedance/Projects/Blog/test.sh) 顶部配置项即可。

## 常见问题

### 端口占用

默认端口是 `18080`。如提示端口占用，可临时改用其他端口：

```bash
BLOG_TEST_PORT=18081 bash ./test.sh
```

### 快照库不存在

如果提示找不到 `db/sanitized.db`，请确认该文件存在后再运行测试。
