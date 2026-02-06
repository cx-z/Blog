cmake_minimum_required(VERSION 3.10)
project(BlogServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 查找 SQLite3
find_package(SQLite3 REQUIRED)

# ==================== 手动管理依赖 ====================
# 依赖由 download-deps.sh 脚本手动下载到以下目录：
# - ../build/deps/crow_include/crow_all.hpp
# - ../build/deps/json_include/nlohmann/json.hpp

# 检查依赖文件是否存在
# CMAKE_CURRENT_BINARY_DIR 是 build 目录，所以用 ./deps/
set(CROW_FILE "${CMAKE_CURRENT_BINARY_DIR}/deps/crow_include/crow_all.hpp")
set(JSON_FILE "${CMAKE_CURRENT_BINARY_DIR}/deps/json_include/nlohmann/json.hpp")

if(NOT EXISTS "${CROW_FILE}")
    message(FATAL_ERROR 
        "❌ 错误: Crow 框架文件未找到！\n"
        "   文件应该在: ${CROW_FILE}\n"
        "   \n"
        "   请先运行下载脚本:\n"
        "   cd ${CMAKE_CURRENT_BINARY_DIR}/..\n"
        "   chmod +x ../download-deps.sh\n"
        "   ../download-deps.sh\n"
        "   \n"
        "   然后重新运行 CMake"
    )
endif()

if(NOT EXISTS "${JSON_FILE}")
    message(FATAL_ERROR 
        "❌ 错误: nlohmann/json 文件未找到！\n"
        "   文件应该在: ${JSON_FILE}\n"
        "   \n"
        "   请先运行下载脚本:\n"
        "   cd ${CMAKE_CURRENT_BINARY_DIR}/..\n"
        "   chmod +x ../download-deps.sh\n"
        "   ../download-deps.sh"
    )
endif()

message(STATUS "✓ 依赖文件已找到")
message(STATUS "  • Crow: ${CROW_FILE}")
message(STATUS "  • JSON: ${JSON_FILE}")

# 添加可执行文件
add_executable(blog_server
    src/main.cpp
    src/database.cpp
)

# 链接库
target_link_libraries(blog_server PRIVATE SQLite::SQLite3)

# 包含头文件
target_include_directories(blog_server PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/deps/crow_include
    ${CMAKE_CURRENT_BINARY_DIR}/deps/json_include
)

# 设置输出目录
set_target_properties(blog_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
