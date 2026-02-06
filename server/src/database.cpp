#include "database.h"
#include <iostream>
#include <chrono>

Database::Database(const std::string& db_path) : db_path(db_path) {}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::init() {
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // 启用外键约束
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    
    return createTablesIfNotExist();
}

bool Database::createTablesIfNotExist() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS posts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            content TEXT NOT NULL,
            timestamp INTEGER NOT NULL
        );
    )";
    
    return executeSQL(sql);
}

bool Database::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::insertPost(const std::string& title, const std::string& content, Post& out_post) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    long long timestamp = ms.count();
    
    std::string sql = "INSERT INTO posts (title, content, timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, timestamp);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Step failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    
    out_post.id = (int)sqlite3_last_insert_rowid(db);
    out_post.title = title;
    out_post.content = content;
    out_post.timestamp = timestamp;
    
    sqlite3_finalize(stmt);
    return true;
}

std::vector<Post> Database::getAllPosts() {
    std::vector<Post> posts;
    std::string sql = "SELECT id, title, content, timestamp FROM posts ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return posts;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Post post;
        post.id = sqlite3_column_int(stmt, 0);
        post.title = std::string((const char*)sqlite3_column_text(stmt, 1));
        post.content = std::string((const char*)sqlite3_column_text(stmt, 2));
        post.timestamp = sqlite3_column_int64(stmt, 3);
        posts.push_back(post);
    }
    
    sqlite3_finalize(stmt);
    return posts;
}

Post Database::getPostById(int id) {
    Post post{-1, "", "", 0};
    std::string sql = "SELECT id, title, content, timestamp FROM posts WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return post;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        post.id = sqlite3_column_int(stmt, 0);
        post.title = std::string((const char*)sqlite3_column_text(stmt, 1));
        post.content = std::string((const char*)sqlite3_column_text(stmt, 2));
        post.timestamp = sqlite3_column_int64(stmt, 3);
    }
    
    sqlite3_finalize(stmt);
    return post;
}

bool Database::updatePost(int id, const std::string& title, const std::string& content) {
    std::string sql = "UPDATE posts SET title = ?, content = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, id);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool Database::deletePost(int id) {
    std::string sql = "DELETE FROM posts WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}
