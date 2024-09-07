#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include "UserDao.h"
#include "PasswordChecker.h"
#include <memory>

/**
 * This class takes responsibility of handling login and registration of a user.
 * It has a chain of 'passwordChecker's in order to suggest user a stronger password options.
 * Also, it has 'userDao' object to ensure that all new users are added to the Database
 * and the passwords during login phase are correct
 */
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
