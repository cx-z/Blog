#include "crow_all.hpp"
#include "database.h"
#include "crypto_utils.h"
#include "jwt_utils.h"
#include "login_service.h"
#include "register_service.h"
#include "verify_service.h"
#include "post_service.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdlib>

int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    namespace fs = std::filesystem;

    // 根据可执行文件位置解析项目根目录（支持 build/bin 与 直接执行两种方式）
    fs::path exe_path;
    if (argc > 0) {
        try {
            exe_path = fs::canonical(argv[0]);
        } catch (...) {
            exe_path = fs::absolute(argv[0]);
        }
    } else {
        exe_path = fs::current_path();
    }

    fs::path exe_dir = exe_path.parent_path();
    fs::path project_root;
    if (exe_dir.filename() == "bin") {
        // 处理 build/bin/blog_server 与 project_root/bin/blog_server 两种情况
        if (exe_dir.parent_path().filename() == "build") {
            project_root = exe_dir.parent_path().parent_path();
        } else {
            project_root = exe_dir.parent_path();
        }
    } else {
        project_root = fs::current_path();
    }

    // 初始化数据库（使用项目根目录下的 db/blog.db）
    std::string db_path = (project_root / "db" / "blog.db").string();
    if (const char* env_db_path = std::getenv("BLOG_DB_PATH")) {
        if (*env_db_path) {
            db_path = env_db_path;
        }
    }
    std::cout << "Resolved project root: " << project_root << std::endl;
    std::cout << "Resolved db path: " << db_path << std::endl;
    Database db(db_path);

    if (!db.init()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    std::cout << "Database initialized successfully" << std::endl;

    // 静态文件根目录
    std::string web_base = (project_root / "web").string();


    // ==================== API 路由 ====================
    
    // ==================== 认证接口 ====================
    
    // POST /api/auth/register - 用户注册
    register_service register_service;
    register_service.registerUser(app, db);
    
    // POST /api/auth/login - 用户登录
    login_service login_service;
    login_service.loginUser(app, db);
    login_service.deleteAccount(app, db);
    
    // POST /api/auth/verify - 验证 JWT Token
    verify_service verify_service;
    verify_service.verifyToken(app, db);
    
    // ==================== 文章接口 ====================
    
    // GET /api/posts - 获取所有文章（需要认证；普通用户只看自己的文章，管理员看全部）
    PostService post_service;
    post_service.getPostList(app, db);

    // GET /api/posts/:id - 获取单篇文章（需要认证且仅限作者或管理员）
    post_service.getPost(app, db);

    // POST /api/posts - 创建新文章（需要认证）
    post_service.createPost(app, db);

    // PUT /api/posts/:id - 更新文章（需要认证）
    post_service.updatePost(app, db);

    // DELETE /api/posts/:id - 删除文章（需要认证）
    post_service.deletePost(app, db);

    // OPTIONS 请求处理 (CORS 预检)
    CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)
    ([](const crow::request&, std::string) {
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        return res;
    });

    // ==================== 静态文件服务 ====================
    // 提供前端静态文件（放在最后以便 API 路由优先匹配）
    CROW_ROUTE(app, "/")
    ([]() {
        crow::response res;
        res.code = 301;
        res.add_header("Location", "/index.html");
        return res;
    });

    // 提供任意静态文件
    CROW_ROUTE(app, "/<path>")
    ([&web_base](const crow::request& req, std::string path) {
        // 如果请求以 api 开头，交给 API 路由处理
        if (path.rfind("api/", 0) == 0 || path == "api") {
            return crow::response(404, "Not Found");
        }

        std::string file_path = web_base + "/" + path;
        
        // 简单的安全检查，防止路径遍历（仅检查请求路径部分）
        if (path.find("..") != std::string::npos) {
            return crow::response(404, "Not Found");
        }
        
        // 读取文件
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return crow::response(404, "Not Found");
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return crow::response(buffer.str());
    });

    CROW_LOG_INFO << "Blog server starting on http://0.0.0.0:8080";
    int port = 8080;
    if (const char* env_port = std::getenv("BLOG_PORT")) {
        try {
            int parsed = std::stoi(env_port);
            if (parsed > 0 && parsed <= 65535) {
                port = parsed;
            }
        } catch (...) {
        }
    }
    app.port(static_cast<uint16_t>(port)).multithreaded().run();
    
    return 0;
}
