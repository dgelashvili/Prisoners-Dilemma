#ifndef MATCH_H
#define MATCH_H

#include <string>

/**
 * Simplest class of this project. It is here to store two usernames and
 * their corresponding average scores during the match.
 * It is used by MatchDao and GameSession to make the code more readable
 */
class Match {
public:
    Match(std::string user1, double score1, std::string user2, double score2);
    ~Match() = default;

    [[nodiscard]] std::string getUser1() const;
    [[nodiscard]] double getScore1() const;
    [[nodiscard]] std::string getUser2() const;
    [[nodiscard]] double getScore2() const;

private:
    std::string user1;
    double score1;
    std::string user2;
    double score2;
};

#endif //MATCH_H
