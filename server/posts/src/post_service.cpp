#include "post_service.h"
#include "database.h"
#include "jwt_utils.h"

void PostService::getPostList(crow::SimpleApp& app, Database& db) {
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
}

void PostService::getPost(crow::SimpleApp& app, Database& db) {
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
}

void PostService::createPost(crow::SimpleApp& app, Database& db) {
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
}

void PostService::updatePost(crow::SimpleApp& app, Database& db) {
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
}

void PostService::deletePost(crow::SimpleApp& app, Database& db) {
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
}
