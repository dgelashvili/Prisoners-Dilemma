#include "Match.h"
#include <utility>

Match::Match(std::string user1, const double score1, std::string user2, const double score2)
    : user1(std::move(user1)), score1(score1), user2(std::move(user2)), score2(score2) {}

std::string Match::getUser1() const {
    return user1;
}

double Match::getScore1() const {
    return score1;
}

std::string Match::getUser2() const {
    return user2;
}

double Match::getScore2() const {
    return score2;
}
