#include "UpperCaseChecker.h"
#include <algorithm>

//checks whether password contains at least one upper case letter
std::string UpperCaseChecker::check(const std::string& password) const {
    if (!std::ranges::any_of(password, ::isupper)) {
        return "Password must contain upper-case letter.";
    }
    return PasswordChecker::check(password);
}
