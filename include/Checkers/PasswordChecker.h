#ifndef PASSWORDCHECKER_H
#define PASSWORDCHECKER_H

#include <memory>
#include <string>

class PasswordChecker {
public:
    PasswordChecker() = default;
    explicit PasswordChecker(std::shared_ptr<PasswordChecker> nextChecker)
        : nextChecker(std::move(nextChecker)) {}

    virtual ~PasswordChecker() = default;

    [[nodiscard]] virtual std::string check(const std::string& password) const {
        if (nextChecker) {
            return nextChecker->check(password);
        }
        return "";  // No issues found
    }

protected:
    std::shared_ptr<PasswordChecker> nextChecker;
};

#endif // PASSWORDCHECKER_H
