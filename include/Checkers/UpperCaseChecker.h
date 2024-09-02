#ifndef UPPERCASECHECKER_H
#define UPPERCASECHECKER_H

#include "PasswordChecker.h"

class UpperCaseChecker final : public PasswordChecker {
public:
    explicit UpperCaseChecker(std::shared_ptr<PasswordChecker> nextChecker = nullptr)
        : PasswordChecker(std::move(nextChecker)) {}

    [[nodiscard]] std::string check(const std::string& password) const override;
};

#endif //UPPERCASECHECKER_H
