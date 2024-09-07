#ifndef MATCHDAO_H
#define MATCHDAO_H

#include <string>
#include <vector>
#include "sqlite3.h"
#include "Match.h"

/**
 * DAO class which handles storing and retrieving matches between users in a 'matches' table of 'db' database.
 * Takes an argument 'dbPath' which is used to open/create database and store in 'db'
 * 'matches' table consists of four columns: 'user1', 'score1', 'user2' and 'score2'
 */
class MatchDAO {
public:
    explicit MatchDAO(const std::string& dbPath);
    ~MatchDAO();

    void addMatch(const Match& match) const;
    //[[nodiscard]] std::pair<std::string, double> getAverageScore(const std::string& username) const;
    //[[nodiscard]] std::vector<std::pair<std::string, double>> getTopPlayers(int num) const;

private:
    sqlite3* db{};
};

#endif //MATCHDAO_H
