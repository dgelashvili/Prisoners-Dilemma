#include "UserDAO.h"
#include <stdexcept>
#include <iostream>

UserDAO::UserDAO(const std::string& dbPath)
    : db(nullptr)
{
    if (sqlite3_open(dbPath.c_str(), &db)) {
        throw std::runtime_error("Could not open database");
    }
    const std::string createTableSQL = "CREATE TABLE IF NOT EXISTS Users ("
                                        "Username TEXT PRIMARY KEY, "
                                        "Password TEXT NOT NULL);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = "Users: ";
        error += errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }
}

UserDAO::~UserDAO() {
    if (db != nullptr) {
        sqlite3_close(db);
    }
}

/**
 *Takes a 'username' as an argument and returns User object which contains the password store in database.
 *If the 'username' does not appear in database, returns 'default' User object
*/
User UserDAO::getUserByName(const std::string& username) const {
    sqlite3_stmt* stmt;
    const std::string querySQL = "SELECT Username, Password FROM Users WHERE Username = ?";
    if (sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    User user("", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const std::string password = (const char*)(sqlite3_column_text(stmt, 1));
        user = User(username, password);
    }

    sqlite3_finalize(stmt);
    return user;
}

//Adds user with unique username in the database
void UserDAO::addUser(const User& user) const {
    sqlite3_stmt* stmt;
    const std::string insertSQL = "INSERT INTO Users (Username, Password) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    sqlite3_bind_text(stmt, 1, user.getUsername().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.getPassword().c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string error = "Failed to add user: ";
        error += sqlite3_errmsg(db);
        throw std::runtime_error(error);
    }

    sqlite3_finalize(stmt);
}

//checks if user with the same fields exist in the database
bool UserDAO::authenticateUser(const User& user) const {
    const User storedUser = getUserByName(user.getUsername());
    return user.equals(storedUser);
}
