#pragma once

#include "crow_all.hpp"

#include <string>

class Database;

class register_service
{
private:
    /* data */
public:
    // 注册用户
    void registerUser(crow::SimpleApp& app, Database& db);
};
