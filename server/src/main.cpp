#include "crow_all.hpp"
#include "database.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

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
    
    // GET /api/posts - 获取所有文章
    CROW_ROUTE(app, "/api/posts")
    ([&db]() {
        auto posts = db.getAllPosts();
        crow::json::wvalue::list json_posts;
        
        for (const auto& post : posts) {
            crow::json::wvalue item;
            item["id"] = post.id;
            item["title"] = post.title;
            item["content"] = post.content;
            item["timestamp"] = post.timestamp;
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

    // GET /api/posts/:id - 获取单篇文章
    CROW_ROUTE(app, "/api/posts/<int>")
    ([&db](int id) {
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
        
        response["success"] = true;
        crow::json::wvalue item;
        item["id"] = post.id;
        item["title"] = post.title;
        item["content"] = post.content;
        item["timestamp"] = post.timestamp;
        response["data"] = std::move(item);
        
        crow::response res(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // POST /api/posts - 创建新文章
    CROW_ROUTE(app, "/api/posts").methods("POST"_method)
    ([&db](const crow::request& req) {
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

    // PUT /api/posts/:id - 更新文章
    CROW_ROUTE(app, "/api/posts/<int>").methods("PUT"_method)
    ([&db](const crow::request& req, int id) {
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

    // DELETE /api/posts/:id - 删除文章
    CROW_ROUTE(app, "/api/posts/<int>").methods("DELETE"_method)
    ([&db](int id) {
        bool success = db.deletePost(id);
        
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
    app.port(8080).multithreaded().run();
    
    return 0;
}
