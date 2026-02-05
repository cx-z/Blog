#include "crow_all.hpp"
#include "database.h"
#include <iostream>
#include <filesystem>

int main() {
    crow::SimpleApp app;
    
    // 初始化数据库
    std::string db_path = "../db/blog.db";
    Database db(db_path);
    
    if (!db.init()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    std::cout << "Database initialized successfully" << std::endl;

    // ==================== 静态文件服务 ====================
    // 提供前端静态文件
    app.get("/", [](const crow::request&) {
        return crow::response(crow::status::permanent_redirect, "/index.html");
    });

    // 提供任意静态文件
    app.get("/<path>", [](const crow::request& req) {
        std::string file_path = "../web/" + req.url_params.get("path");
        
        // 简单的安全检查，防止路径遍历
        if (file_path.find("..") != std::string::npos) {
            return crow::response(crow::status::not_found);
        }
        
        // 读取文件
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return crow::response(crow::status::not_found);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return crow::response(buffer.str());
    });

    // ==================== API 路由 ====================
    
    // GET /api/posts - 获取所有文章
    app.get("/api/posts", [&db](const crow::request&) {
        auto posts = db.getAllPosts();
        std::vector<crow::json::wvalue> json_posts;
        
        for (const auto& post : posts) {
            json_posts.push_back(post.to_json());
        }
        
        crow::json::wvalue response;
        response["success"] = true;
        response["data"] = json_posts;
        
        auto res = crow::response(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // GET /api/posts/:id - 获取单篇文章
    app.get("/api/posts/<int>", [&db](int id) {
        Post post = db.getPostById(id);
        
        crow::json::wvalue response;
        
        if (post.id == -1) {
            response["success"] = false;
            response["message"] = "Post not found";
            return crow::response(crow::status::not_found, response);
        }
        
        response["success"] = true;
        response["data"] = post.to_json();
        
        auto res = crow::response(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // POST /api/posts - 创建新文章
    app.post("/api/posts", [&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("title") || !body.has("content")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing title or content";
            
            auto res = crow::response(crow::status::bad_request, error);
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
            response["data"] = new_post.to_json();
            auto res = crow::response(crow::status::created, response);
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        } else {
            response["success"] = false;
            response["message"] = "Failed to insert post";
            auto res = crow::response(crow::status::internal_server_error, response);
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    });

    // PUT /api/posts/:id - 更新文章
    app.put("/api/posts/<int>", [&db](const crow::request& req, int id) {
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("title") || !body.has("content")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing title or content";
            
            auto res = crow::response(crow::status::bad_request, error);
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
        
        auto res = crow::response(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // DELETE /api/posts/:id - 删除文章
    app.delete_("/api/posts/<int>", [&db](int id) {
        bool success = db.deletePost(id);
        
        crow::json::wvalue response;
        response["success"] = success;
        response["message"] = success ? "Post deleted" : "Failed to delete post";
        
        auto res = crow::response(response);
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // OPTIONS 请求处理 (CORS 预检)
    app.options("/<path>", [](const crow::request&) {
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        return res;
    });

    CROW_LOG_INFO << "Blog server starting on http://0.0.0.0:8080";
    app.port(8080).multithreaded().run();
    
    return 0;
}
