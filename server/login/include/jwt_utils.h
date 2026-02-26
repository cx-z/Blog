#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JwtUtils {
private:
    static const std::string SECRET_KEY;
    static const long long EXPIRATION_TIME; // 7 天（秒）
    
    // Base64 编码
    static std::string base64_encode(const std::string& input);
    
    // Base64 解码
    static std::string base64_decode(const std::string& input);
    
    // HMAC-SHA256 签名
    static std::string hmacSha256(const std::string& message, const std::string& key);
    
    // 十六进制转 Base64（用于签名）
    static std::string hexToBase64(const std::string& hex);
    
public:
    // 生成 JWT Token（包含 role）
    static std::string generateToken(int userId, const std::string& username, const std::string& role);
    
    // 验证 JWT Token（返回用户 ID，失败返回 -1）
    static int verifyToken(const std::string& token);
    
    // 从 Token 中提取用户信息
    static json getTokenPayload(const std::string& token);
};
