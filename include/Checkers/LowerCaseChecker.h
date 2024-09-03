#ifndef LOWERCASECHECKER_H
#define LOWERCASECHECKER_H

#include "PasswordChecker.h"

/**
 * Similar to LengthChecker class, LowerCaseChecker handles checking whether the password
 * user typed during registration contains at least one lower case letter
 */
class LowerCaseChecker final : public PasswordChecker {
public:
    explicit LowerCaseChecker(std::shared_ptr<PasswordChecker> nextChecker = nullptr)
        : PasswordChecker(std::move(nextChecker)) {}

    [[nodiscard]] std::string check(const std::string& password) const override;
};

#endif //LOWERCASECHECKER_H
