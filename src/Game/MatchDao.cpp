#include "MatchDao.h"

#include <stdexcept>

MatchDAO::MatchDAO(const std::string& dbPath)
    : db(nullptr)
{
    if (sqlite3_open(dbPath.c_str(), &db)) {
        throw std::runtime_error("Could not open database");
    }
    const std::string createTableSQL = "CREATE TABLE IF NOT EXISTS Matches ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                        "User1 TEXT NOT NULL, "
                                        "Score1 DOUBLE NOT NULL, "
                                        "User2 TEXT NOT NULL, "
                                        "Score2 DOUBLE NOT NULL"
                                        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = "Matches: ";
        error += errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error(error);
    }
}

MatchDAO::~MatchDAO() {
    if (db != nullptr) {
        sqlite3_close(db);
    }
}

//Adds certain match to the database
void MatchDAO::addMatch(const Match &match) const {
    sqlite3_stmt* stmt;
    const std::string insertSQL = "INSERT INTO Matches (User1, Score1, User2, Score2) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    sqlite3_bind_text(stmt, 1, match.getUser1().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, std::to_string(match.getScore1()).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, match.getUser2().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, std::to_string(match.getScore2()).c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string error = "Failed to add user: ";
        error += sqlite3_errmsg(db);
        throw std::runtime_error(error);
    }

    sqlite3_finalize(stmt);
}

//This function returns average of all scores of the username
std::pair<std::string, double> MatchDAO::getAverageScore(const std::string& username) const {
    sqlite3_stmt* stmt;
    const std::string query = "SELECT AVG "
                              "(CASE WHEN user1 = ? THEN score1 "
                              "WHEN user2 = ? THEN score2 END) as avg_score "
                              "FROM Matches "
                              "WHERE user1 = ? OR user2 = ?;";
    double avgScore = 0.0;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        avgScore = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return std::make_pair(username, avgScore);
}

//This function returns top 'num' players
std::vector<std::pair<std::string, double>> MatchDAO::getTopPlayers(int num) const {
    sqlite3_stmt* stmt;
    const std::string sql = "SELECT username, AVG(score) AS avg_score "
                            "FROM ("
                            "SELECT user1 AS username, score1 AS score FROM Matches "
                            "UNION ALL "
                            "SELECT user2 AS username, score2 AS score FROM Matches"
                            ") "
                            "GROUP BY username "
                            "ORDER BY avg_score DESC;";
    std::vector<std::pair<std::string, double>> topPlayers;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW && num--) {
        auto username = (const char*)(sqlite3_column_text(stmt, 0));
        double avg_score = sqlite3_column_double(stmt, 1);
        topPlayers.emplace_back(username, avg_score);
    }

    sqlite3_finalize(stmt);
    return topPlayers;
}



