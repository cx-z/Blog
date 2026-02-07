#pragma once

#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JwtUtils {
private:
    static const std::string SECRET_KEY;
    static const long long EXPIRATION_TIME; // 7 天（秒）
    
    // Base64 编码
    static std::string base64_encode(const std::string& input) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string encoded;
        int val = 0;
        int valb = 0;
        
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 6) {
                valb -= 6;
                encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            }
        }
        
        if (valb > 0) {
            encoded.push_back(base64_chars[(val << (6 - valb)) & 0x3F]);
        }
        
        while (encoded.size() % 4) {
            encoded.push_back('=');
        }
        
        return encoded;
    }
    
    // Base64 解码
    static std::string base64_decode(const std::string& input) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string decoded;
        std::vector<int> T(256, -1);
        
        for (int i = 0; i < 64; i++) {
            T[base64_chars[i]] = i;
        }
        
        int val = 0;
        int valb = 0;
        
        for (unsigned char c : input) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c];
            valb += 6;
            
            if (valb >= 8) {
                valb -= 8;
                decoded.push_back(char((val >> valb) & 0xFF));
            }
        }
        
        return decoded;
    }
    
    // HMAC-SHA256 签名
    static std::string hmacSha256(const std::string& message, const std::string& key) {
        unsigned char digest[SHA256_DIGEST_LENGTH];
        unsigned int digest_len;
        
        HMAC(EVP_sha256(),
             (unsigned char*)key.c_str(), key.length(),
             (unsigned char*)message.c_str(), message.length(),
             digest, &digest_len);
        
        std::stringstream ss;
        for (unsigned int i = 0; i < digest_len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
        }
        return ss.str();
    }
    
    // 十六进制转 Base64（用于签名）
    static std::string hexToBase64(const std::string& hex) {
        std::string binary;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byte = hex.substr(i, 2);
            binary += static_cast<char>(strtol(byte.c_str(), nullptr, 16));
        }
        return base64_encode(binary);
    }
    
public:
    // 生成 JWT Token
    static std::string generateToken(int userId, const std::string& username) {
        // Header
        json header;
        header["alg"] = "HS256";
        header["typ"] = "JWT";
        std::string header_str = header.dump();
        std::string header_b64 = base64_encode(header_str);
        
        // Payload
        json payload;
        payload["sub"] = userId;
        payload["username"] = username;
        payload["iat"] = std::time(nullptr);
        payload["exp"] = std::time(nullptr) + EXPIRATION_TIME;
        std::string payload_str = payload.dump();
        std::string payload_b64 = base64_encode(payload_str);
        
        // Signature
        std::string message = header_b64 + "." + payload_b64;
        std::string signature_hex = hmacSha256(message, SECRET_KEY);
        std::string signature_b64 = hexToBase64(signature_hex);
        
        return message + "." + signature_b64;
    }
    
    // 验证 JWT Token（返回用户 ID，失败返回 -1）
    static int verifyToken(const std::string& token) {
        size_t first_dot = token.find('.');
        size_t second_dot = token.find('.', first_dot + 1);
        
        if (first_dot == std::string::npos || second_dot == std::string::npos) {
            return -1;
        }
        
        std::string header_b64 = token.substr(0, first_dot);
        std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
        std::string signature_b64 = token.substr(second_dot + 1);
        
        // 验证签名
        std::string message = header_b64 + "." + payload_b64;
        std::string expected_signature_hex = hmacSha256(message, SECRET_KEY);
        std::string expected_signature_b64 = hexToBase64(expected_signature_hex);
        
        if (signature_b64 != expected_signature_b64) {
            return -1;
        }
        
        // 解析 payload
        try {
            std::string payload_str = base64_decode(payload_b64);
            json payload = json::parse(payload_str);
            
            // 检查过期时间
            long long exp = payload["exp"];
            if (exp < std::time(nullptr)) {
                return -1; // Token 已过期
            }
            
            return payload["sub"];
        } catch (...) {
            return -1;
        }
    }
    
    // 从 Token 中提取用户信息
    static json getTokenPayload(const std::string& token) {
        size_t first_dot = token.find('.');
        size_t second_dot = token.find('.', first_dot + 1);
        
        if (first_dot == std::string::npos || second_dot == std::string::npos) {
            return json();
        }
        
        std::string payload_b64 = token.substr(first_dot + 1, second_dot - first_dot - 1);
        
        try {
            std::string payload_str = base64_decode(payload_b64);
            return json::parse(payload_str);
        } catch (...) {
            return json();
        }
    }
};

// 定义静态常量
const std::string JwtUtils::SECRET_KEY = "blog_secret_key_2026";  // 生产环境应从配置文件读取
const long long JwtUtils::EXPIRATION_TIME = 7 * 24 * 60 * 60;     // 7 天
