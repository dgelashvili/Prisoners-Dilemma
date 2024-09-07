#include "GameSession.h"
#include "HelperFunctions.h"
#include <random>

GameSession::GameSession(std::string player1, const SOCKET client1Socket, std::string player2, const SOCKET client2Socket,
                         std::mutex& playingMutex, std::condition_variable& cvPlaying,
                         std::unordered_set<std::string>& playingUsers)
    : player1(std::move(player1)), client1Socket(client1Socket),
      player2(std::move(player2)), client2Socket(client2Socket),
      playingMutex(playingMutex), cvPlaying(cvPlaying), playingUsers(playingUsers) {
    score1 = 0;
    score2 = 0;
}

void GameSession::updateScores(const std::string& player1Str, const std::string& player2Str) {
    if (player1Str == "SPLIT") {
        if (player2Str == "SPLIT") {
            score1 += 3;
            score2 += 3;
        } else {
            score1 += 0;
            score2 += 5;
        }
    } else {
        if (player2Str == "SPLIT") {
            score1 += 5;
            score2 += 0;
        } else {
            score1 += 1;
            score2 += 1;
        }
    }
}


void GameSession::playRound() {
    sendToClient(client1Socket, "Do you want to split or steal? (SPLIT/STEAL): ");
    sendToClient(client2Socket, "Do you want to split or steal? (SPLIT/STEAL): ");

    std::string player1Str = receiveFromClient(client1Socket);
    std::string player2Str = receiveFromClient(client2Socket);
    if (player1Str != "STEAL") {
        player1Str = "SPLIT";
    }
    if (player2Str != "STEAL") {
        player2Str = "SPLIT";
    }

    updateScores(player1Str, player2Str);

    const std::string currentScores = player1 + ": " + std::to_string(score1) + "\n" +
                                      player2 + ": " + std::to_string(score2) + "\n";
    sendToClient(client1Socket, "Your opponent chose to: " + player2Str + "!\n\n" + currentScores);
    sendToClient(client2Socket, "Your opponent chose to: " + player1Str + "\n\n" + currentScores);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void GameSession::runGame() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distribution(3, 5);
    const int roundNumber = distribution(gen);

    for (int i = 0; i < roundNumber; i++) {
        playRound();
    }

    const double average1 = (double) score1 / roundNumber;
    const double average2 = (double) score2 / roundNumber;
    const std::string finalMessage = "Match is over!\nYour final average scores are:\n" +
                                    player1 + ": " + std::to_string(average1) + "\n" +
                                    player2 + ": " + std::to_string(average2) + "\n\n";

    if (score1 > score2) {
        sendToClient(client1Socket, finalMessage + "You won the match!\n");
        sendToClient(client2Socket, finalMessage + "You lost the match!\n");
    } else if (score2 > score1) {
        sendToClient(client1Socket, finalMessage + "You lost the match!\n");
        sendToClient(client2Socket, finalMessage + "You won the match!\n");
    } else {
        sendToClient(client1Socket, finalMessage + "Draw!\n");
        sendToClient(client2Socket, finalMessage + "Draw!\n");
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::lock_guard<std::mutex> lock(playingMutex);
        playingUsers.erase(player1);
        playingUsers.erase(player2);
    }
    cvPlaying.notify_all();
}
