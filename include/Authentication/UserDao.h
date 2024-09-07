#ifndef USERDAO_H
#define USERDAO_H

#include <string>
#include "sqlite3.h"
#include "User.h"

/**
 * DAO class which handles storing and retrieving users in a 'user' table of 'db' database.
 * Takes an argument 'dbPath' which is used to open/create database and store in 'db'
 * 'user' table consists of two columns: 'username' and 'password'
 */
class UserDAO {
public:
    explicit UserDAO(const std::string& dbPath);
    ~UserDAO();

    [[nodiscard]] User getUserByName(const std::string& username) const;
    void addUser(const User& user) const;
    [[nodiscard]] bool authenticateUser(const User& user) const;

private:
    sqlite3* db{};
};

#endif // USERDAO_H
