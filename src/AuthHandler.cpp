#include "AuthHandler.h"
#include <utility>

AuthHandler::AuthHandler(std::shared_ptr<UserDAO> dao, std::shared_ptr<PasswordChecker> checker)
    : userDao(std::move(dao)), passwordChecker(std::move(checker)) {}

std::string AuthHandler::loginUser(const std::string &username, const std::string &password) const {
    const User user = userDao->getUserByName(username);
    if (user.getUsername().empty()) {
        return "User " + username + " does not exist.";
    }

    if (password != user.getPassword()) {
        return "User " + username + " does not match password.";
    }

    return "User " + username + " logged in successfully";
}

std::string AuthHandler::registerUser(const std::string &username,
    const std::string &password, const std::string &repeated_password) const {
    if (password != repeated_password) {
        return "Passwords do not match.";
    }

    if (const std::string validationError = passwordChecker->check(password); !validationError.empty()) {
        return validationError;
    }

    if (const User user = userDao->getUserByName(username); !user.getUsername().empty()) {
        return "Username " + username + " already exists.";
    }

    const auto newUser = User(username, password);
    userDao->addUser(newUser);
    return "User " + username + " registered successfully.";
}
