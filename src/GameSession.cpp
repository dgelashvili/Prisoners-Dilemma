#include "GameSession.h"
#include "HelperFunctions.h"

GameSession::GameSession(std::string player1, const SOCKET client1Socket, std::string player2, const SOCKET client2Socket,
                         std::mutex& playingMutex, std::condition_variable& cvPlaying,
                         std::unordered_set<std::string>& playingUsers)
    : player1(std::move(player1)), client1Socket(client1Socket),
      player2(std::move(player2)), client2Socket(client2Socket),
      playingMutex(playingMutex), cvPlaying(cvPlaying), playingUsers(playingUsers) {}

void GameSession::runGame() const {
    sendToClient(client1Socket, "Enter a number: ");
    sendToClient(client2Socket, "Enter a number: ");

    std::string player1Str = receiveFromClient(client1Socket);
    std::string player2Str = receiveFromClient(client2Socket);
    if (player1Str.empty()) {
        player1Str = "0";
    }
    if (player2Str.empty()) {
        player2Str = "0";
    }

    const int player1Number = std::stoi(player1Str);
    const int player2Number = std::stoi(player2Str);

    sendToClient(client1Socket, "Your opponent typed: " + player2Str + "\n");
    sendToClient(client2Socket, "Your opponent typed: " + player1Str + "\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (player1Number > player2Number) {
        sendToClient(client1Socket, "You win the match!\n");
        sendToClient(client2Socket, "You lose the match!\n");
    } else if (player2Number > player1Number) {
        sendToClient(client1Socket, "You lose the match!\n");
        sendToClient(client2Socket, "You win the match!\n");
    } else {
        sendToClient(client1Socket, "The match is a draw!\n");
        sendToClient(client2Socket, "The match is a draw!\n");
    }

    {
        std::lock_guard<std::mutex> lock(playingMutex);
        playingUsers.erase(player1);
        playingUsers.erase(player2);
    }
    cvPlaying.notify_all();
}
