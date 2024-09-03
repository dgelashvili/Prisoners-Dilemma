#ifndef PASSWORDCHECKER_H
#define PASSWORDCHECKER_H

#include <memory>
#include <string>

/**
 * This is a main class for a chain of responsibility. Each member of this class has a pointer 'nextChecker'
 * which is a pointer to the other member of the class, thus, creating a chain.
 * Also, each member should implement 'check' method, to check the password uniquely.
 */
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
        return "";
    }

protected:
    std::shared_ptr<PasswordChecker> nextChecker;
};

#endif // PASSWORDCHECKER_H
