#ifndef UPPERCASECHECKER_H
#define UPPERCASECHECKER_H

#include "PasswordChecker.h"

/**
 * Very similar to LowerCaseChecker class, UpperCaseChecker handles checking whether the password
 * user typed during registration contains at least one upper case letter
 */
class UpperCaseChecker final : public PasswordChecker {
public:
    explicit UpperCaseChecker(std::shared_ptr<PasswordChecker> nextChecker = nullptr)
        : PasswordChecker(std::move(nextChecker)) {}

    [[nodiscard]] std::string check(const std::string& password) const override;
};

#endif //UPPERCASECHECKER_H
