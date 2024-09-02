#ifndef LENGTHCHECKER_H
#define LENGTHCHECKER_H

#include "PasswordChecker.h"

class LengthChecker final : public PasswordChecker {
public:
    explicit LengthChecker(const size_t minLength, std::shared_ptr<PasswordChecker> nextChecker = nullptr)
        : PasswordChecker(std::move(nextChecker)), minLength(minLength) {}

    [[nodiscard]] std::string check(const std::string& password) const override;

private:
    size_t minLength;
};

#endif // LENGTHCHECKER_H
