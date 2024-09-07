#include "Server.h"
#include <iostream>
#include <utility>
#include "HelperFunctions.h"
#include "GameSession.h"

//starts initializing WinSock and setups 'listeningSocket'
Server::Server(std::string  ip, const int port, std::shared_ptr<ServerAuthenticator> serverAuthenticator,
                std::shared_ptr<MatchDAO> matchDAO)
    : ip(std::move(ip)), port(port), listeningSocket(INVALID_SOCKET),
        running(false), authenticator(std::move(serverAuthenticator)), matchDAO(std::move(matchDAO)) {
    initializeWinSock();
    setupListeningSocket();
}

//calls 'stop()' which handles clearing everything
Server::~Server() {
    stop();
}

void Server::initializeWinSock() {
    WSADATA wsaData;
    if (const int result = WSAStartup(MAKEWORD(2, 2), &wsaData); result != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
    }
}

void Server::setupListeningSocket() {
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == INVALID_SOCKET) {
        const auto err = WSAGetLastError();
        stop();
        throw std::runtime_error("Socket creation failed: " + std::to_string(err));
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    if (bind(listeningSocket, (sockaddr*)(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        const auto err = WSAGetLastError();
        stop();
        throw std::runtime_error("Bind failed: " + std::to_string(err));
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        const auto err = WSAGetLastError();
        stop();
        throw std::runtime_error("Listen failed: " + std::to_string(err));
    }
}

//changes state of 'running' to true and starts accepting connections
void Server::start() {
    running = true;
    std::cout << "Server started on " << ip << ":" << port << std::endl;
    matchmakingThread = std::thread(&Server::matchmakingLoop, this);
    acceptConnections();
}

/**
 * Changes state of 'running' ro false
 * Closes 'listeningSocket' if one is valid.
 * Cleans up WSA
 * Joins every thread which is stored in clientThreads
 */
void Server::stop() {
    running = false;

    if (listeningSocket != INVALID_SOCKET) {
        closesocket(listeningSocket);
    }
    WSACleanup();
    cvMatchMaking.notify_all();
    if (matchmakingThread.joinable()) {
        matchmakingThread.join();
    }
    for (auto& t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    activeUsers.clear();
}

//accepts new client and stores its corresponding new thread into 'clientThreads'
void Server::acceptConnections() {
    while (running) {
        SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            if (const auto err = WSAGetLastError(); err != 10004) {
                stop();
                throw std::runtime_error("Accept failed: " + std::to_string(err));
            }
            continue;
        }

        clientThreads.emplace_back(&Server::handleClient, this, clientSocket);
    }
}

/**
 * This is a single thread which is for pairing users which are in the queue.
 * It uses matchmakingQueue where all players who want to play are
 * Everything is thread-saved using condition variable and mutex
 *
 * After pairing, starts a game between two of them
 */
void Server::matchmakingLoop() {
    while (running) {
        std::unique_lock<std::mutex> lock(matchmakingMutex);
        cvMatchMaking.wait(lock, [&]() { return matchmakingQueue.size() >= 2 || !running; });

        while (running && matchmakingQueue.size() >= 2) {
            const std::string player1 = matchmakingQueue.front().first;
            const SOCKET client1Socket = matchmakingQueue.front().second;
            matchmakingQueue.pop();
            const std::string player2 = matchmakingQueue.front().first;
            const SOCKET client2Socket = matchmakingQueue.front().second;
            matchmakingQueue.pop();

            sendToClient(client1Socket, "Paired with " + player2 + "! Get Ready!\n");
            sendToClient(client2Socket, "Paired with " + player1 + "! Get Ready!\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));

            {
                std::lock_guard<std::mutex> lock2(playingMutex);
                playingUsers.insert(player1);
                playingUsers.insert(player2);
            }
            GameSession gameSession(player1, client1Socket, player2, client2Socket,
                playingMutex, cvPlaying, playingUsers, matchDAO);
            std::thread([gameSession]() mutable {
                gameSession.runGame();
            }).detach();

            cvHandleClient.notify_all();
        }
    }
}

/**
 * This is a main menu of the game. It asks user if one wants to play or exit
 * Once the player chooses to play, this method puts username and socket associated with them into queue
 * and notifies 'matchmakingLoop' that it should check the size of 'matchmakingQueue'
 *
 * Waits up until the game is ended and only then goes back to the start of the loop
 */
void Server::mainMenuLoop(const SOCKET clientSocket, const std::string& username) {
    const auto matchmakingTimeout = std::chrono::seconds(30);
    while(running) {
        std::string mainMenu = "play/exit (P/X): ";
        std::vector<std::string> userInput = promptUser(clientSocket, {mainMenu});
        if (userInput.empty()) {
            break;
        }
        if (userInput[0] == "P") {
            {
                std::lock_guard<std::mutex> lock(matchmakingMutex);
                const std::pair pair(username, clientSocket);
                matchmakingQueue.push(pair);
            }
            cvMatchMaking.notify_one();
            {
                std::unique_lock<std::mutex> lock(matchmakingMutex);
                if (cvHandleClient.wait_for(lock, matchmakingTimeout,
                    [&]() { return !findInQueue(username, matchmakingQueue) || !running; })) {
                    if (!running) {
                        return;
                    }
                } else {
                    removeFromQueue(username, matchmakingQueue);
                    sendToClient(clientSocket, "Matchmaking timeout. Try again.\n");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
            }
            {
                std::unique_lock<std::mutex> lock(playingMutex);
                cvPlaying.wait(lock, [&]() { return !playingUsers.contains(username) || !running;});
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else if (userInput[0] == "X") {
            const auto goodbye_message = "Goodbye!";
            sendToClient(clientSocket, goodbye_message);
            break;
        } else {
            const auto unknown_message = "unknown command: " + userInput[0] + "\n";
            sendToClient(clientSocket, unknown_message);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

/**
 * Starts with loginRegistrationPhase, if client logs in successfully, stores the username in 'activeUsers'
 * so while logged in, no other client can log in using the same account
 */
void Server::handleClient(const SOCKET clientSocket) {
    const std::string username = authenticator->loginRegistrationPhase(clientSocket, activeUsersMutex, activeUsers);
    if (username.empty()) {
        closesocket(clientSocket);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(activeUsersMutex);
        activeUsers.insert(username);
    }

    mainMenuLoop(clientSocket, username);

    closesocket(clientSocket);
    {
        std::lock_guard<std::mutex> lock(activeUsersMutex);
        activeUsers.erase(username);
    }
}
