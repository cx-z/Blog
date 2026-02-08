#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Post {
    int id;
    std::string title;
    std::string content;
    long long timestamp;
    int user_id; // 作者用户 ID（可能为 -1 表示未知）
    std::string author; // 作者用户名（便于前端展示）

    json to_json() const {
        return json{
            {"id", id},
            {"title", title},
            {"content", content},
            {"timestamp", timestamp},
            {"user_id", user_id},
            {"author", author}
        };
    }
};

struct User {
    int id;
    std::string username;
    std::string password_hash;
    std::string salt;
    long long created_at;
    std::string role;
};

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    bool init();
    bool createTablesIfNotExist();
    
    // 文章 CRUD 操作
    bool insertPost(const std::string& title, const std::string& content, int user_id, Post& out_post);
    std::vector<Post> getAllPosts();
    Post getPostById(int id);
    bool updatePost(int id, const std::string& title, const std::string& content);
    bool deletePost(int id);
    std::vector<Post> getPostsByUser(int user_id);
    
    // 用户操作
    bool insertUser(const std::string& username, const std::string& password_hash, 
                    const std::string& salt, int& out_user_id);
    User getUserByUsername(const std::string& username);
    bool userExists(const std::string& username);

private:
    sqlite3* db = nullptr;
    std::string db_path;
    
    bool executeSQL(const std::string& sql);
};
