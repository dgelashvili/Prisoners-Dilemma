#ifndef USERDAO_H
#define USERDAO_H

#include <string>
#include "sqlite3.h"
#include "User.h"

class UserDAO {
public:
    explicit UserDAO(const std::string& dbPath);
    ~UserDAO();

    [[nodiscard]] User getUserByName(const std::string& name) const;
    void addUser(const User& user) const;
    [[nodiscard]] bool authenticateUser(const User& user) const;

private:
    sqlite3* db{};

    void executeSQL(const std::string& sql) const;
};

#endif // USERDAO_H
