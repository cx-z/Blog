cmake_minimum_required(VERSION 3.10)
project(BlogServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

# 添加包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/login/include)

# 查找 SQLite3
find_package(SQLite3 REQUIRED)

# 查找 OpenSSL（用于加密和 JWT）
find_package(OpenSSL REQUIRED)

set(CROW_FILE "${CMAKE_CURRENT_LIST_DIR}/third_party/crow_include/crow_all.hpp")
set(JSON_FILE "${CMAKE_CURRENT_LIST_DIR}/third_party/json_include/nlohmann/json.hpp")

if(NOT EXISTS "${CROW_FILE}")
    message(FATAL_ERROR
        "❌ 错误: Crow 头文件未找到！\n"
        "   文件应该在: ${CROW_FILE}\n"
        "   \n"
        "   请按 deps.md 指引下载第三方头文件到 server/third_party"
    )
endif()

if(NOT EXISTS "${JSON_FILE}")
    message(FATAL_ERROR
        "❌ 错误: nlohmann/json 头文件未找到！\n"
        "   文件应该在: ${JSON_FILE}\n"
        "   \n"
        "   请按 deps.md 指引下载第三方头文件到 server/third_party"
    )
endif()

message(STATUS "✓ 第三方头文件已找到")
message(STATUS "  • Crow: ${CROW_FILE}")
message(STATUS "  • JSON: ${JSON_FILE}")

find_path(ASIO_INCLUDE_DIR asio.hpp)
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "❌ 错误: asio.hpp 未找到，请安装 asio 开发包")
endif()

# 添加可执行文件
add_executable(blog_server
    src/main.cpp
    src/database.cpp
    src/crypto_utils.cpp
    src/jwt_utils.cpp
    login/src/login_service.cpp
    login/src/register_service.cpp
    login/src/verify_service.cpp
)

# 测试可执行文件
add_executable(db_tests
    ${CMAKE_CURRENT_LIST_DIR}/../test/test_database.cpp
    src/database.cpp
)

# 链接库
target_link_libraries(blog_server PRIVATE SQLite::SQLite3 OpenSSL::Crypto OpenSSL::SSL)
target_link_libraries(db_tests PRIVATE SQLite::SQLite3)

# 包含头文件
target_include_directories(blog_server PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/third_party/crow_include
    ${CMAKE_CURRENT_LIST_DIR}/third_party/json_include
    ${ASIO_INCLUDE_DIR}
)

target_include_directories(db_tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/third_party/json_include
    ${CMAKE_CURRENT_LIST_DIR}/include
)

add_test(NAME db_tests COMMAND db_tests)

# 设置输出目录
set_target_properties(blog_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
