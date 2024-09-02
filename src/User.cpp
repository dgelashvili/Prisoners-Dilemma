#include "User.h"

User::User(std::string username, std::string password)
    : username(std::move(username)), password(std::move(password)) {}

bool User::equals(const User &other) const {
    return username == other.username && password == other.password;
}

std::string User::getUsername() const {
    return username;
}

std::string User::getPassword() const {
    return password;
}


