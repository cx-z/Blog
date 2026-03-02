#include "database.h"
#include <filesystem>
#include <iostream>
#include <random>
#include <string>

static void fail(const std::string& message) {
    std::cerr << message << std::endl;
    std::exit(1);
}

static void check(bool condition, const std::string& message) {
    if (!condition) fail(message);
}

static std::string randomSuffix() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 15);
    std::string s;
    for (int i = 0; i < 16; i++) {
        int v = dis(gen);
        s.push_back("0123456789abcdef"[v]);
    }
    return s;
}

int main() {
    namespace fs = std::filesystem;

    fs::path dir = fs::temp_directory_path() / ("blog_db_tests_" + randomSuffix());
    fs::create_directories(dir);
    fs::path db_path = dir / "test.db";

    {
        Database db(db_path.string());
        check(db.init(), "db.init failed");

        int user_id = -1;
        std::string username = "u_" + randomSuffix();
        check(db.insertUser(username, "hash", "salt", user_id), "insertUser failed");
        check(user_id > 0, "insertUser did not set user_id");
        check(db.userExists(username), "userExists should return true for active user");
        check(db.isUserActive(user_id), "isUserActive should return true for active user");

        std::string dup_username = "dup_" + randomSuffix();
        int a = -1;
        check(db.insertUser(dup_username, "hash", "salt", a), "insertUser first dup user failed");
        int b = -1;
        check(!db.insertUser(dup_username, "hash", "salt", b), "insertUser should fail on duplicate username");

        Post created;
        check(db.insertPost("t1", "c1", user_id, created), "insertPost failed");
        check(created.id > 0, "insertPost did not set post id");
        check(created.user_id == user_id, "insertPost wrong user_id");

        Post loaded = db.getPostById(created.id);
        check(loaded.id == created.id, "getPostById returned wrong id");
        check(loaded.title == "t1", "getPostById wrong title");

        check(db.updatePost(created.id, "t2", "c2"), "updatePost failed");
        Post updated = db.getPostById(created.id);
        check(updated.title == "t2", "updatePost did not update title");

        check(!db.updatePost(99999999, "x", "y"), "updatePost should fail for missing row");
        check(!db.deletePost(99999999), "deletePost should fail for missing row");
        check(!db.softDeletePost(99999999, 1), "softDeletePost should fail for missing row");

        check(db.softDeletePost(created.id, 123), "softDeletePost failed");
        Post soft_deleted = db.getPostById(created.id);
        check(soft_deleted.deleted_by_admin == 1, "softDeletePost did not set deleted_by_admin");
        check(soft_deleted.deleted_at == 123, "softDeletePost did not set deleted_at");

        Post created2;
        check(db.insertPost("t3", "c3", user_id, created2), "insertPost second failed");
        check(db.deletePost(created2.id), "deletePost failed");
        Post deleted2 = db.getPostById(created2.id);
        check(deleted2.id == -1, "deletePost should remove the row");

        long long deleted_at = 1700000000000LL;
        std::string new_username = "deleted::" + username + "::" + std::to_string(user_id) + "::" + std::to_string(deleted_at);
        check(db.deleteAccount(user_id, new_username, username, deleted_at), "deleteAccount failed");
        check(!db.isUserActive(user_id), "isUserActive should return false for deleted user");
        check(!db.userExists(username), "userExists should return false for deleted user");
        check(db.getPostsByUser(user_id).empty(), "getPostsByUser should be empty after deleteAccount");

        User u = db.getUserById(user_id);
        check(u.id == user_id, "getUserById returned wrong id");
        check(u.is_deleted == 1, "getUserById should show is_deleted=1");
        check(u.deleted_at == deleted_at, "getUserById should show correct deleted_at");
        check(u.original_username == username, "getUserById should preserve original_username");
        check(u.username == new_username, "getUserById should rename username after deleteAccount");

        check(!db.deleteAccount(user_id, new_username, username, deleted_at), "deleteAccount should fail if already deleted");

        int reused_id = -1;
        check(db.insertUser(username, "hash2", "salt2", reused_id), "insertUser should allow reusing username after deleteAccount");
        check(reused_id != user_id, "reused username should get a new user_id");
    }

    std::error_code ec;
    fs::remove_all(dir, ec);
    if (ec) {
        std::cerr << "cleanup failed: " << ec.message() << std::endl;
        return 1;
    }

    std::cout << "db_tests ok" << std::endl;
    return 0;
}
