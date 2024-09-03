#ifndef USER_H
#define USER_H

#include <string>

/**
 * Simplest class of this project. It is here to store username and password of a client.
 * It is used by UserDao and authHandler to make the code more readable
 */
class User {
public:
    User(std::string  username, std::string  password);
    ~User() = default;

    [[nodiscard]] bool equals(const User& other) const;

    [[nodiscard]] std::string getUsername() const;
    [[nodiscard]] std::string getPassword() const;

private:
    std::string username;
    std::string password;
};

#endif //USER_H
