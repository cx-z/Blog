#include "register_service.h"
#include "crypto_utils.h"
#include "database.h"
#include "jwt_utils.h"

void register_service::registerUser(crow::SimpleApp& app, Database& db)
{
    CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
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
        
        // 验证账号不为空
        if (username.empty() || username.length() < 3) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Username must be at least 3 characters";
            
            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        // 验证密码强度
        if (password.empty() || password.length() < 6) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Password must be at least 6 characters";
            
            crow::response res(error);
            res.code = 400;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        // 检查账号是否已存在
        if (db.userExists(username)) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Username already exists";
            
            crow::response res(error);
            res.code = 409;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        // 生成 salt 并哈希密码
        std::string salt = CryptoUtils::generateSalt();
        std::string salt_hex = CryptoUtils::saltToHex(salt);
        std::string password_hash = CryptoUtils::hashPassword(password, salt);
        
        // 将用户写入数据库
        int user_id;
        if (!db.insertUser(username, password_hash, salt_hex, user_id)) {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Failed to register user";
            
            crow::response res(error);
            res.code = 500;
            res.add_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        
        // 注册成功，返回用户 ID、角色 和 JWT Token
        std::string token = JwtUtils::generateToken(user_id, username, std::string("user"));
        
        crow::json::wvalue response;
        response["success"] = true;
        response["message"] = "Registration successful";
        response["data"]["user_id"] = user_id;
        response["data"]["username"] = username;
        response["data"]["role"] = "user";
        response["data"]["token"] = token;
        
        crow::response res(response);
        res.code = 201;
        res.add_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
