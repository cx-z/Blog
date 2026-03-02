#pragma once

#include "crow_all.hpp"

class Database;

class login_service
{
private:
    /* data */
public:
    // 登录用户
    void loginUser(crow::SimpleApp& app, Database& db);
    void deleteAccount(crow::SimpleApp& app, Database& db);
};
