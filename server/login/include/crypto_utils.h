#pragma once

#include <string>

class CryptoUtils {
public:
    // 生成随机 salt（16 字节）
    static std::string generateSalt();
    
    // 将二进制 salt 转换为十六进制字符串
    static std::string saltToHex(const std::string& salt);
    
    // 将十六进制字符串转换为二进制 salt
    static std::string hexToSalt(const std::string& hex);
    
    // SHA-256 哈希
    static std::string sha256(const std::string& input);
    
    // 密码哈希：SHA256(password + salt)
    static std::string hashPassword(const std::string& password, const std::string& salt);
    
    // 验证密码
    static bool verifyPassword(const std::string& password, const std::string& storedHash, 
                                const std::string& storedSaltHex);
};
