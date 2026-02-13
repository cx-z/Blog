由于本仓库引入的依赖有：

## brew install

- cmake
- sqlite3
- asio
- db-browser-for-sqlite
- openssl@3

## 后端第三方库（不随仓库提交）

本仓库不会把 `server/third_party` 纳入 Git；请在本地按以下版本下载并放置到约定路径。

- Crow：v1.3.0.0  
  仓库：https://github.com/CrowCpp/Crow.git
- nlohmann/json：v3.11.2  
  仓库：https://github.com/nlohmann/json.git

### 放置路径（构建脚本/CMake 约定）

- `server/third_party/crow_include/crow_all.hpp`
- `server/third_party/json_include/nlohmann/json.hpp`

### 下载与安装（示例）

在工程根目录执行：

```bash
mkdir -p server/third_party/crow_include server/third_party/json_include/nlohmann

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

git clone --depth 1 --branch v1.3.0.0 https://github.com/CrowCpp/Crow.git "$TMP_DIR/crow"
cp "$TMP_DIR/crow/include/crow_all.h" server/third_party/crow_include/crow_all.hpp

git clone --depth 1 --branch v3.11.2 https://github.com/nlohmann/json.git "$TMP_DIR/json"
cp "$TMP_DIR/json/single_include/nlohmann/json.hpp" server/third_party/json_include/nlohmann/json.hpp
```
