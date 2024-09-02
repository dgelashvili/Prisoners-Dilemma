#ifndef USER_H
#define USER_H

#include <string>

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
