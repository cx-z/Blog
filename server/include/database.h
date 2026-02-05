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

    json to_json() const {
        return json{
            {"id", id},
            {"title", title},
            {"content", content},
            {"timestamp", timestamp}
        };
    }
};

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    bool init();
    bool createTablesIfNotExist();
    
    // CRUD 操作
    bool insertPost(const std::string& title, const std::string& content, Post& out_post);
    std::vector<Post> getAllPosts();
    Post getPostById(int id);
    bool updatePost(int id, const std::string& title, const std::string& content);
    bool deletePost(int id);

private:
    sqlite3* db = nullptr;
    std::string db_path;
    
    bool executeSQL(const std::string& sql);
};
