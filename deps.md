由于本仓库引入的依赖有：

推荐使用 Homebrew 安装系统依赖：

```bash
brew install cmake sqlite3 asio
```

项目运行时还会下载并使用以下头文件库（由 `download-deps.sh` 获取到 `build/deps/`）：

- Crow (crow_all.hpp)
- nlohmann/json (json.hpp)

这些头文件位于：

- `build/deps/crow_include/crow_all.hpp`
- `build/deps/json_include/nlohmann/json.hpp`

请先运行 `./download-deps.sh` 下载依赖文件，然后再运行 `./build.sh` 编译。
由于本仓库引入的依赖有：
# brew install
- cmake
- sqlite3
- asio