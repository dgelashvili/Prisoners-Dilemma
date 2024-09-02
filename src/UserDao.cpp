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
    executeSQL(createTableSQL);
}

UserDAO::~UserDAO() {
    if (db != nullptr) {
        sqlite3_close(db);
    }
}

User UserDAO::getUserByName(const std::string& name) const {
    sqlite3_stmt* stmt;
    const std::string querySQL = "SELECT Username, Password FROM Users WHERE Username = ?";
    if (sqlite3_prepare_v2(db, querySQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    User user("", "");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user = User(username, password);
    }

    sqlite3_finalize(stmt);
    return user;
}

void UserDAO::addUser(const User& user) const {
    const std::string insertSQL = "INSERT INTO Users (Username, Password) VALUES ('" +
                                  user.getUsername() + "', '" + user.getPassword() + "');";
    executeSQL(insertSQL);
}

bool UserDAO::authenticateUser(const User& user) const {
    const User storedUser = getUserByName(user.getUsername());
    return user.equals(storedUser);
}

void UserDAO::executeSQL(const std::string& sql) const {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = "Users: ";
        error += errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }
}
