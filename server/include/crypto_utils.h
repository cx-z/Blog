#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/sha.h>

class CryptoUtils {
public:
    // 生成随机 salt（16 字节）
    static std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::string salt;
        for (int i = 0; i < 16; i++) {
            salt += static_cast<char>(dis(gen));
        }
        return salt;
    }
    
    // 将二进制 salt 转换为十六进制字符串
    static std::string saltToHex(const std::string& salt) {
        std::stringstream ss;
        for (unsigned char c : salt) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        return ss.str();
    }
    
    // 将十六进制字符串转换为二进制 salt
    static std::string hexToSalt(const std::string& hex) {
        std::string salt;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byte = hex.substr(i, 2);
            char c = (char)strtol(byte.c_str(), nullptr, 16);
            salt += c;
        }
        return salt;
    }
    
    // SHA-256 哈希
    static std::string sha256(const std::string& input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)input.c_str(), input.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    // 密码哈希：SHA256(password + salt)
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        return sha256(password + salt);
    }
    
    // 验证密码
    static bool verifyPassword(const std::string& password, const std::string& storedHash, 
                                const std::string& storedSaltHex) {
        std::string salt = hexToSalt(storedSaltHex);
        std::string computedHash = hashPassword(password, salt);
        return computedHash == storedHash;
    }
};
