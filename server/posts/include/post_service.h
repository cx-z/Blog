#pragma once

#include "crow_all.hpp"

class Database;

class PostService
{
private:
    /* data */
public:
    // 获取帖子列表
    void getPostList(crow::SimpleApp& app, Database& db);
    // 获取单篇文章
    void getPost(crow::SimpleApp& app, Database& db);
    // 创建帖子
    void createPost(crow::SimpleApp& app, Database& db);
    // 更新帖子
    void updatePost(crow::SimpleApp& app, Database& db);
    // 删除帖子
    void deletePost(crow::SimpleApp& app, Database& db);
};