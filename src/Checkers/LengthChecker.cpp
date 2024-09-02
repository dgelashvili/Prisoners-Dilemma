#include "LengthChecker.h"

std::string LengthChecker::check(const std::string& password) const {
    if (password.length() < minLength) {
        return "Password must be at least " + std::to_string(minLength) + " characters long.";
    }
    return PasswordChecker::check(password);
}
