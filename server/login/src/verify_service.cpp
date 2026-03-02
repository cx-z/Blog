#include "verify_service.h"
#include "crow_all.hpp"
#include "jwt_utils.h"
#include "database.h"

void verify_service::verifyToken(crow::SimpleApp& app, Database& db)
{
    CROW_ROUTE(app, "/api/auth/verify").methods("POST"_method)
    ([&db](const crow::request& req) {
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

        if (!db.isUserActive(user_id)) {
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
}
