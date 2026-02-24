#include "crow_all.hpp"
#include "database.h"
#include "crypto_utils.h"
#include "jwt_utils.h"
#include "login_service.h"
#include "register_service.h"

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
    
    // POST /api/auth/verify - 验证 JWT Token
    CROW_ROUTE(app, "/api/auth/verify").methods("POST"_method)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("token")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing token";
            
            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        std::string token = body["token"].s();
        int user_id = JwtUtils::verifyToken(token);
        
        if (user_id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid or expired token";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        json payload = JwtUtils::getTokenPayload(token);
        
        crow::json::wvalue response;
        response["success"] = true;
        response["message"] = "Token is valid";
        response["data"]["user_id"] = user_id;
        response["data"]["username"] = payload["username"].get<std::string>();
        response["data"]["role"] = payload.contains("role") ? payload["role"].get<std::string>() : std::string("user");
        
        crow::response res(response);
        res.code = 200;
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
    
    // ==================== 文章接口 ====================
    
    // GET /api/posts - 获取所有文章（需要认证；普通用户只看自己的文章，管理员看全部）
    CROW_ROUTE(app, "/api/posts")
    ([&db](const crow::request& req) {
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing or invalid Authorization header";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        std::string token = auth_header.substr(7);
        int user_id = JwtUtils::verifyToken(token);
        if (user_id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid or expired token";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        json payload = JwtUtils::getTokenPayload(token);
        std::string role = payload.contains("role") ? payload["role"].get<std::string>() : std::string("user");

        std::vector<Post> posts;
        if (role == "admin") {
            posts = db.getAllPosts();
        } else {
            posts = db.getPostsByUser(user_id);
        }

        crow::json::wvalue::list json_posts;
        for (const auto& post : posts) {
            crow::json::wvalue item;
            item["id"] = post.id;
            item["title"] = post.title;
            item["content"] = post.content;
            item["timestamp"] = post.timestamp;
            item["author"] = post.author;
            item["user_id"] = post.user_id;
            item["deleted_by_admin"] = post.deleted_by_admin;
            item["deleted_at"] = post.deleted_at;
            item["is_author"] = (post.user_id == user_id) && (post.deleted_by_admin == 0);
            json_posts.push_back(item);
        }

        crow::json::wvalue response;
        response["success"] = true;
        response["data"] = std::move(json_posts);

        crow::response res(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // GET /api/posts/:id - 获取单篇文章（需要认证且仅限作者或管理员）
    CROW_ROUTE(app, "/api/posts/<int>")
    ([&db](const crow::request& req, int id) {
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing or invalid Authorization header";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        std::string token = auth_header.substr(7);
        int user_id = JwtUtils::verifyToken(token);
        if (user_id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid or expired token";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        json payload = JwtUtils::getTokenPayload(token);
        std::string role = payload.contains("role") ? payload["role"].get<std::string>() : std::string("user");

        Post post = db.getPostById(id);
        
        crow::json::wvalue response;
        
        if (post.id == -1) {
            response["success"] = false;
            response["message"] = "Post not found";
            crow::response res(response);
            res.code = 404;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        // 仅作者或管理员可查看（遗留文章 user_id 为 -1/0 不对普通用户开放）
        if (post.user_id != user_id && role != "admin") {
            response["success"] = false;
            response["message"] = "Forbidden";
            crow::response res(response);
            res.code = 403;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        response["success"] = true;
        crow::json::wvalue item;
        item["id"] = post.id;
        item["title"] = post.title;
        item["content"] = post.content;
        item["timestamp"] = post.timestamp;
        item["author"] = post.author;
        item["user_id"] = post.user_id;
        item["deleted_by_admin"] = post.deleted_by_admin;
        item["deleted_at"] = post.deleted_at;
        item["is_author"] = (post.user_id == user_id) && (post.deleted_by_admin == 0);
        response["data"] = std::move(item);
        
        crow::response res(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // POST /api/posts - 创建新文章（需要认证）
    CROW_ROUTE(app, "/api/posts").methods("POST"_method)
    ([&db](const crow::request& req) {
        // 验证 JWT Token
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing or invalid Authorization header";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        std::string token = auth_header.substr(7);
        int user_id = JwtUtils::verifyToken(token);
        
        if (user_id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid or expired token";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("title") || !body.has("content")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing title or content";
            
            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        Post new_post;
        bool success = db.insertPost(
            body["title"].s(),
            body["content"].s(),
            user_id,
            new_post
        );
        
        crow::json::wvalue response;
        
        if (success) {
            response["success"] = true;
            crow::json::wvalue item;
            item["id"] = new_post.id;
            item["title"] = new_post.title;
            item["content"] = new_post.content;
            item["timestamp"] = new_post.timestamp;
            item["author"] = new_post.author;
            response["data"] = std::move(item);
            crow::response res(response);
            res.code = 201;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        } else {
            response["success"] = false;
            response["message"] = "Failed to insert post";
            crow::response res(response);
            res.code = 500;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    });

    // PUT /api/posts/:id - 更新文章（需要认证）
    CROW_ROUTE(app, "/api/posts/<int>").methods("PUT"_method)
    ([&db](const crow::request& req, int id) {
        // 验证 JWT Token
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing or invalid Authorization header";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        std::string token = auth_header.substr(7);
        int user_id = JwtUtils::verifyToken(token);
        
        if (user_id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid or expired token";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        json payload = JwtUtils::getTokenPayload(token);
        std::string role = payload.contains("role") ? payload["role"].get<std::string>() : std::string("user");
        
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("title") || !body.has("content")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing title or content";
            
            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        Post post = db.getPostById(id);
        if (post.id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Post not found";
            crow::response res(error);
            res.code = 404;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        if (post.user_id != user_id) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Forbidden";
            crow::response res(error);
            res.code = 403;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        if (post.deleted_by_admin) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Forbidden";
            crow::response res(error);
            res.code = 403;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        bool success = db.updatePost(id, body["title"].s(), body["content"].s());
        
        crow::json::wvalue response;
        response["success"] = success;
        
        if (success) {
            response["message"] = "Post updated";
        } else {
            response["message"] = "Failed to update post";
        }
        
        crow::response res(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // DELETE /api/posts/:id - 删除文章（需要认证）
    CROW_ROUTE(app, "/api/posts/<int>").methods("DELETE"_method)
    ([&db](const crow::request& req, int id) {
        // 验证 JWT Token
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing or invalid Authorization header";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        std::string token = auth_header.substr(7);
        int user_id = JwtUtils::verifyToken(token);
        
        if (user_id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid or expired token";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        json payload = JwtUtils::getTokenPayload(token);
        std::string role = payload.contains("role") ? payload["role"].get<std::string>() : std::string("user");

        Post post = db.getPostById(id);
        if (post.id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Post not found";
            crow::response res(error);
            res.code = 404;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        if (post.user_id != user_id && role != "admin") {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Forbidden";
            crow::response res(error);
            res.code = 403;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        bool success = false;
        if (role == "admin" && post.user_id != user_id) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
            success = db.softDeletePost(id, ms.count());
        } else {
            success = db.deletePost(id);
        }
        
        crow::json::wvalue response;
        response["success"] = success;
        response["message"] = success ? "Post deleted" : "Failed to delete post";
        
        crow::response res(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

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
