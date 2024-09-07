#ifndef GAMESESSION_H
#define GAMESESSION_H

#include <string>
#include <winsock2.h>
#include <mutex>
#include <condition_variable>
#include <unordered_set>

class GameSession {
public:
    GameSession(std::string player1, SOCKET client1Socket, std::string player2, SOCKET client2Socket,
                std::mutex& playingMutex, std::condition_variable& cvPlaying,
                std::unordered_set<std::string>& playingUsers);
    ~GameSession() = default;

    void runGame();

private:
    void updateScores(const std::string& player1Str, const std::string& player2Str);
    void playRound();

    std::string player1;
    SOCKET client1Socket;
    std::string player2;
    SOCKET client2Socket;

    int score1;
    int score2;

    std::mutex& playingMutex;
    std::condition_variable& cvPlaying;
    std::unordered_set<std::string>& playingUsers;
};

#endif //GAMESESSION_H
