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
            timestamp INTEGER NOT NULL,
            user_id INTEGER,
            deleted_by_admin INTEGER NOT NULL DEFAULT 0,
            deleted_at INTEGER
        );
        
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            role TEXT DEFAULT 'user',
            is_deleted INTEGER NOT NULL DEFAULT 0,
            deleted_at INTEGER,
            original_username TEXT
        );
    )";

    if (!executeSQL(sql)) return false;

    // 如果 posts 表存在但没有 user_id 字段，尝试添加该列（兼容旧数据库）
    // 查询表结构以判断列是否存在
    bool has_user_id = false;
    bool has_deleted_by_admin = false;
    bool has_deleted_at = false;
    const char* pragma_sql = "PRAGMA table_info(posts);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, pragma_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* colname = sqlite3_column_text(stmt, 1);
            if (colname) {
                std::string name = reinterpret_cast<const char*>(colname);
                if (name == "user_id") {
                    has_user_id = true;
                } else if (name == "deleted_by_admin") {
                    has_deleted_by_admin = true;
                } else if (name == "deleted_at") {
                    has_deleted_at = true;
                }
            }
        }
    }
    if (stmt) sqlite3_finalize(stmt);

    if (!has_user_id) {
        executeSQL("ALTER TABLE posts ADD COLUMN user_id INTEGER;");
    }
    if (!has_deleted_by_admin) {
        executeSQL("ALTER TABLE posts ADD COLUMN deleted_by_admin INTEGER NOT NULL DEFAULT 0;");
    }
    if (!has_deleted_at) {
        executeSQL("ALTER TABLE posts ADD COLUMN deleted_at INTEGER;");
    }

    bool has_is_deleted = false;
    bool has_user_deleted_at = false;
    bool has_original_username = false;
    const char* pragma_users_sql = "PRAGMA table_info(users);";
    stmt = nullptr;
    if (sqlite3_prepare_v2(db, pragma_users_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* colname = sqlite3_column_text(stmt, 1);
            if (colname) {
                std::string name = reinterpret_cast<const char*>(colname);
                if (name == "is_deleted") {
                    has_is_deleted = true;
                } else if (name == "deleted_at") {
                    has_user_deleted_at = true;
                } else if (name == "original_username") {
                    has_original_username = true;
                }
            }
        }
    }
    if (stmt) sqlite3_finalize(stmt);

    if (!has_is_deleted) {
        executeSQL("ALTER TABLE users ADD COLUMN is_deleted INTEGER NOT NULL DEFAULT 0;");
    }
    if (!has_user_deleted_at) {
        executeSQL("ALTER TABLE users ADD COLUMN deleted_at INTEGER;");
    }
    if (!has_original_username) {
        executeSQL("ALTER TABLE users ADD COLUMN original_username TEXT;");
    }

    return true;
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

bool Database::insertPost(const std::string& title, const std::string& content, int user_id, Post& out_post) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    long long timestamp = ms.count();
    
    std::string sql = "INSERT INTO posts (title, content, timestamp, user_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, timestamp);
    sqlite3_bind_int(stmt, 4, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Step failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    
    out_post.id = (int)sqlite3_last_insert_rowid(db);
    out_post.title = title;
    out_post.content = content;
    out_post.timestamp = timestamp;
    out_post.user_id = user_id;
    out_post.author = "";
    out_post.deleted_by_admin = 0;
    out_post.deleted_at = 0;
    
    sqlite3_finalize(stmt);
    return true;
}

std::vector<Post> Database::getAllPosts() {
    std::vector<Post> posts;
    // 使用 LEFT JOIN 获取作者用户名（若存在）
    std::string sql = R"(
        SELECT p.id, p.title, p.content, p.timestamp, p.user_id, u.username, p.deleted_by_admin, p.deleted_at
        FROM posts p
        LEFT JOIN users u ON p.user_id = u.id
        ORDER BY p.timestamp DESC;
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return posts;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Post post;
        post.id = sqlite3_column_int(stmt, 0);
        post.title = sqlite3_column_text(stmt, 1) ? std::string((const char*)sqlite3_column_text(stmt, 1)) : "";
        post.content = sqlite3_column_text(stmt, 2) ? std::string((const char*)sqlite3_column_text(stmt, 2)) : "";
        post.timestamp = sqlite3_column_int64(stmt, 3);
        post.user_id = sqlite3_column_int(stmt, 4);
        post.author = sqlite3_column_text(stmt, 5) ? std::string((const char*)sqlite3_column_text(stmt, 5)) : std::string("");
        post.deleted_by_admin = sqlite3_column_int(stmt, 6);
        post.deleted_at = sqlite3_column_int64(stmt, 7);
        posts.push_back(post);
    }

    sqlite3_finalize(stmt);
    return posts;
}

Post Database::getPostById(int id) {
    Post post{-1, "", "", 0, -1, "", 0, 0};
    std::string sql = R"(
        SELECT p.id, p.title, p.content, p.timestamp, p.user_id, u.username, p.deleted_by_admin, p.deleted_at
        FROM posts p
        LEFT JOIN users u ON p.user_id = u.id
        WHERE p.id = ?;
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return post;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        post.id = sqlite3_column_int(stmt, 0);
        post.title = sqlite3_column_text(stmt, 1) ? std::string((const char*)sqlite3_column_text(stmt, 1)) : "";
        post.content = sqlite3_column_text(stmt, 2) ? std::string((const char*)sqlite3_column_text(stmt, 2)) : "";
        post.timestamp = sqlite3_column_int64(stmt, 3);
        post.user_id = sqlite3_column_int(stmt, 4);
        post.author = sqlite3_column_text(stmt, 5) ? std::string((const char*)sqlite3_column_text(stmt, 5)) : std::string("");
        post.deleted_by_admin = sqlite3_column_int(stmt, 6);
        post.deleted_at = sqlite3_column_int64(stmt, 7);
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
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE) && (sqlite3_changes(db) > 0);
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
    bool success = (sqlite3_step(stmt) == SQLITE_DONE) && (sqlite3_changes(db) > 0);
    sqlite3_finalize(stmt);
    return success;
}

bool Database::softDeletePost(int id, long long deleted_at) {
    std::string sql = "UPDATE posts SET deleted_by_admin = 1, deleted_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int64(stmt, 1, deleted_at);
    sqlite3_bind_int(stmt, 2, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE) && (sqlite3_changes(db) > 0);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<Post> Database::getPostsByUser(int user_id) {
    std::vector<Post> posts;
    std::string sql = R"(
        SELECT p.id, p.title, p.content, p.timestamp, p.user_id, u.username, p.deleted_by_admin, p.deleted_at
        FROM posts p
        LEFT JOIN users u ON p.user_id = u.id
        WHERE p.user_id = ?
        ORDER BY p.timestamp DESC;
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return posts;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Post post;
        post.id = sqlite3_column_int(stmt, 0);
        post.title = sqlite3_column_text(stmt, 1) ? std::string((const char*)sqlite3_column_text(stmt, 1)) : "";
        post.content = sqlite3_column_text(stmt, 2) ? std::string((const char*)sqlite3_column_text(stmt, 2)) : "";
        post.timestamp = sqlite3_column_int64(stmt, 3);
        post.user_id = sqlite3_column_int(stmt, 4);
        post.author = sqlite3_column_text(stmt, 5) ? std::string((const char*)sqlite3_column_text(stmt, 5)) : std::string("");
        post.deleted_by_admin = sqlite3_column_int(stmt, 6);
        post.deleted_at = sqlite3_column_int64(stmt, 7);
        posts.push_back(post);
    }

    sqlite3_finalize(stmt);
    return posts;
}

// ==================== 用户操作 ====================

bool Database::insertUser(const std::string& username, const std::string& password_hash,
                          const std::string& salt, int& out_user_id) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    long long created_at = ms.count();
    
    std::string sql = "INSERT INTO users (username, password_hash, salt, created_at, role) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, created_at);
    sqlite3_bind_text(stmt, 5, "user", -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Step failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    
    out_user_id = (int)sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return true;
}

User Database::getUserByUsername(const std::string& username) {
    User user{-1, "", "", "", 0, "user", 0, 0, ""};
    std::string sql = "SELECT id, username, password_hash, salt, created_at, role, is_deleted, deleted_at, original_username FROM users WHERE username = ? AND is_deleted = 0;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return user;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.username = sqlite3_column_text(stmt, 1) ? std::string((const char*)sqlite3_column_text(stmt, 1)) : std::string("");
        user.password_hash = sqlite3_column_text(stmt, 2) ? std::string((const char*)sqlite3_column_text(stmt, 2)) : std::string("");
        user.salt = sqlite3_column_text(stmt, 3) ? std::string((const char*)sqlite3_column_text(stmt, 3)) : std::string("");
        user.created_at = sqlite3_column_int64(stmt, 4);
        user.role = sqlite3_column_text(stmt, 5) ? std::string((const char*)sqlite3_column_text(stmt, 5)) : std::string("user");
        user.is_deleted = sqlite3_column_int(stmt, 6);
        user.deleted_at = sqlite3_column_int64(stmt, 7);
        user.original_username = sqlite3_column_text(stmt, 8) ? std::string((const char*)sqlite3_column_text(stmt, 8)) : std::string("");
    }
    
    sqlite3_finalize(stmt);
    return user;
}

User Database::getUserById(int id) {
    User user{-1, "", "", "", 0, "user", 0, 0, ""};
    std::string sql = "SELECT id, username, password_hash, salt, created_at, role, is_deleted, deleted_at, original_username FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return user;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = sqlite3_column_int(stmt, 0);
        user.username = sqlite3_column_text(stmt, 1) ? std::string((const char*)sqlite3_column_text(stmt, 1)) : std::string("");
        user.password_hash = sqlite3_column_text(stmt, 2) ? std::string((const char*)sqlite3_column_text(stmt, 2)) : std::string("");
        user.salt = sqlite3_column_text(stmt, 3) ? std::string((const char*)sqlite3_column_text(stmt, 3)) : std::string("");
        user.created_at = sqlite3_column_int64(stmt, 4);
        user.role = sqlite3_column_text(stmt, 5) ? std::string((const char*)sqlite3_column_text(stmt, 5)) : std::string("user");
        user.is_deleted = sqlite3_column_int(stmt, 6);
        user.deleted_at = sqlite3_column_int64(stmt, 7);
        user.original_username = sqlite3_column_text(stmt, 8) ? std::string((const char*)sqlite3_column_text(stmt, 8)) : std::string("");
    }

    sqlite3_finalize(stmt);
    return user;
}

bool Database::userExists(const std::string& username) {
    std::string sql = "SELECT 1 FROM users WHERE username = ? AND is_deleted = 0 LIMIT 1;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::isUserActive(int id) {
    std::string sql = "SELECT is_deleted FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool active = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        active = sqlite3_column_int(stmt, 0) == 0;
    }

    sqlite3_finalize(stmt);
    return active;
}

bool Database::deleteAccount(int user_id, const std::string& new_username, const std::string& original_username, long long deleted_at) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }

    sqlite3_stmt* delete_posts_stmt = nullptr;
    std::string delete_posts_sql = "DELETE FROM posts WHERE user_id = ?;";
    if (sqlite3_prepare_v2(db, delete_posts_sql.c_str(), -1, &delete_posts_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_int(delete_posts_stmt, 1, user_id);
    if (sqlite3_step(delete_posts_stmt) != SQLITE_DONE) {
        sqlite3_finalize(delete_posts_stmt);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(delete_posts_stmt);

    sqlite3_stmt* update_user_stmt = nullptr;
    std::string update_user_sql = "UPDATE users SET is_deleted = 1, deleted_at = ?, original_username = ?, username = ? WHERE id = ? AND is_deleted = 0;";
    if (sqlite3_prepare_v2(db, update_user_sql.c_str(), -1, &update_user_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    sqlite3_bind_int64(update_user_stmt, 1, deleted_at);
    sqlite3_bind_text(update_user_stmt, 2, original_username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(update_user_stmt, 3, new_username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(update_user_stmt, 4, user_id);

    bool ok = (sqlite3_step(update_user_stmt) == SQLITE_DONE) && (sqlite3_changes(db) > 0);
    sqlite3_finalize(update_user_stmt);

    if (!ok) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}
