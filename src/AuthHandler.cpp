#include "AuthHandler.h"
#include <utility>

AuthHandler::AuthHandler(std::shared_ptr<UserDAO> dao, std::shared_ptr<PasswordChecker> checker)
    : userDao(std::move(dao)), passwordChecker(std::move(checker)) {}

/**
 * Handles logging of a user.
 * Checks if 'username' does exist.
 * Checks if 'password' matches the one which was typed during registration.
 *
 * Returns corresponding message for each scenario
 */
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

/**
 * Handles registering of a user.
 * Checks if 'password' and 'repeated_password' match
 * Checks if 'password' passed all checks of 'passwordChecker' chain
 * Checks if 'username' already exists in 'users' table
 * On a successful registration, adds user in a table using 'userDao'
 *
 * Returns corresponding message for each scenario
 */
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
