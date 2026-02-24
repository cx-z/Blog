#include "login_service.h"
#include "crypto_utils.h"
#include "database.h"
#include "jwt_utils.h"

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
