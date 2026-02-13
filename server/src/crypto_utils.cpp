#include "crypto_utils.h"

#include <sstream>
#include <iomanip>
#include <random>
#include <cstdlib>
#include <openssl/sha.h>

std::string CryptoUtils::generateSalt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::string salt;
    for (int i = 0; i < 16; i++) {
        salt += static_cast<char>(dis(gen));
    }
    return salt;
}

std::string CryptoUtils::saltToHex(const std::string& salt) {
    std::stringstream ss;
    for (unsigned char c : salt) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

std::string CryptoUtils::hexToSalt(const std::string& hex) {
    std::string salt;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte = hex.substr(i, 2);
        char c = (char)strtol(byte.c_str(), nullptr, 16);
        salt += c;
    }
    return salt;
}

std::string CryptoUtils::sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input.c_str(), input.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::string CryptoUtils::hashPassword(const std::string& password, const std::string& salt) {
    return sha256(password + salt);
}

bool CryptoUtils::verifyPassword(const std::string& password, const std::string& storedHash,
                                 const std::string& storedSaltHex) {
    std::string salt = hexToSalt(storedSaltHex);
    std::string computedHash = hashPassword(password, salt);
    return computedHash == storedHash;
}
