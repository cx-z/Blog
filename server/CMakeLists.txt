cmake_minimum_required(VERSION 3.10)
project(BlogServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 查找 SQLite3
find_package(SQLite3 REQUIRED)

# Crow 和 nlohmann_json 依赖
include(FetchContent)

# 设置 FetchContent 使用国内镜像或官方稳定源
set(FETCHCONTENT_QUIET OFF)  # 显示下载进度

# 下载 Crow - 使用 GitHub 原始文件 CDN
FetchContent_Declare(crow
    URL https://raw.githubusercontent.com/CrowCpp/Crow/v1.0+5/include/crow_all.hpp
    DOWNLOAD_NO_EXTRACT ON
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/crow_include
    DOWNLOAD_NAME crow_all.hpp
)

# 下载 nlohmann/json - 使用原始文件 CDN
FetchContent_Declare(nlohmann_json
    URL https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp
    DOWNLOAD_NO_EXTRACT ON
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/json_include
    DOWNLOAD_NAME json.hpp
)

# 设置超时和重试
set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

# 下载依赖（添加错误处理）
if(NOT crow_POPULATED)
    message(STATUS "下载 Crow 框架...")
    FetchContent_Populate(crow)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/crow_include)
    if(EXISTS "${crow_SOURCE_DIR}/crow_all.hpp")
        file(COPY "${crow_SOURCE_DIR}/crow_all.hpp" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/crow_include")
    endif()
endif()

if(NOT nlohmann_json_POPULATED)
    message(STATUS "下载 nlohmann/json...")
    FetchContent_Populate(nlohmann_json)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/json_include/nlohmann)
    if(EXISTS "${nlohmann_json_SOURCE_DIR}/json.hpp")
        file(COPY "${nlohmann_json_SOURCE_DIR}/json.hpp" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/json_include/nlohmann")
    endif()
endif()

# 添加可执行文件
add_executable(blog_server
    src/main.cpp
    src/database.cpp
)

# 链接库
target_link_libraries(blog_server PRIVATE SQLite::SQLite3)

# 包含头文件
target_include_directories(blog_server PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/crow_include
    ${CMAKE_CURRENT_BINARY_DIR}/json_include
)

# 设置输出目录
set_target_properties(blog_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
