#pragma once
#include "crow_all.hpp"

#include <string>

class Database;

class verify_service
{
private:
    /* data */
public:
    // 验证 Token
    void verifyToken(crow::SimpleApp& app, Database& db);
};