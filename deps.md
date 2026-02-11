由于本仓库引入的依赖有：
# brew install
- cmake
- sqlite3
- asio
- db-browser-for-sqlite

仓库内已内置以下第三方头文件依赖（用于后端构建）：

- Crow (crow_all.hpp)
- nlohmann/json (json.hpp)

这些头文件位于：

- `server/third_party/crow_include/crow_all.hpp`
- `server/third_party/json_include/nlohmann/json.hpp`
