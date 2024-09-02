#include "LowerCaseChecker.h"
#include <algorithm>

std::string LowerCaseChecker::check(const std::string& password) const {
    if (!std::ranges::any_of(password, ::islower)) {
        return "Password must contain lower-case letter.";
    }
    return PasswordChecker::check(password);
}
