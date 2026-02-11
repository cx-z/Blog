由于本仓库引入的依赖有：
# brew install
- cmake
- sqlite3
- asio
- db-browser-for-sqlite

项目运行时还会下载并使用以下头文件库（由 `download-deps.sh` 获取到 `build/deps/`）：

- Crow (crow_all.hpp)
- nlohmann/json (json.hpp)

这些头文件位于：

- `build/deps/crow_include/crow_all.hpp`
- `build/deps/json_include/nlohmann/json.hpp`
