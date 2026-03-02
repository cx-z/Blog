#include "login_service.h"
#include "crypto_utils.h"
#include "database.h"
#include "jwt_utils.h"

#include <chrono>

void login_service::loginUser(crow::SimpleApp& app, Database& db)
{
    CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
    ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        
        if (!body || !body.has("username") || !body.has("password")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing username or password";
            
            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        
        // 查询用户
        User user = db.getUserByUsername(username);
        
        if (user.id == -1) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid username or password";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        // 验证密码
        if (!CryptoUtils::verifyPassword(password, user.password_hash, user.salt)) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid username or password";
            
            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        // 登录成功，生成 JWT Token（包含 role）
        std::string token = JwtUtils::generateToken(user.id, user.username, user.role);
        
        crow::json::wvalue response;
        response["success"] = true;
        response["message"] = "Login successful";
        response["data"]["user_id"] = user.id;
        response["data"]["username"] = user.username;
        response["data"]["role"] = user.role;
        response["data"]["token"] = token;
        
        crow::response res(response);
        res.code = 200;
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}

void login_service::deleteAccount(crow::SimpleApp& app, Database& db)
{
    CROW_ROUTE(app, "/api/auth/delete").methods("POST"_method)
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

        User user = db.getUserById(user_id);
        if (user.id == -1 || user.is_deleted != 0) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Account is deleted";

            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("password")) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Missing password";

            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        std::string password = body["password"].s();

        if (!CryptoUtils::verifyPassword(password, user.password_hash, user.salt)) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Invalid password";

            crow::response res(error);
            res.code = 401;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        long long deleted_at = ms.count();

        std::string original_username = user.username;
        std::string new_username = "deleted::" + original_username + "::" + std::to_string(user_id) + "::" + std::to_string(deleted_at);

        bool success = db.deleteAccount(user_id, new_username, original_username, deleted_at);

        crow::json::wvalue response;
        response["success"] = success;
        response["message"] = success ? "Account deleted" : "Failed to delete account";

        crow::response res(response);
        res.code = success ? 200 : 500;
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
