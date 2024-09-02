#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include "UserDao.h"
#include "PasswordChecker.h"
#include <memory>

class AuthHandler {
public:
    AuthHandler(std::shared_ptr<UserDAO> dao, std::shared_ptr<PasswordChecker> checker);
    ~AuthHandler() = default;

    [[nodiscard]] std::string loginUser(const std::string& username, const std::string& password) const;
    [[nodiscard]] std::string registerUser(const std::string& username,
        const std::string& password, const std::string& repeated_password) const;

private:
    std::shared_ptr<UserDAO> userDao;
    std::shared_ptr<PasswordChecker> passwordChecker;
};

#endif //AUTHHANDLER_H
