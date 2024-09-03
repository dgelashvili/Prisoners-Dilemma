#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include "AuthHandler.h"
#include <unordered_set>

#pragma comment(lib, "Ws2_32.lib")

/**
 * This is a Server Class, which is a main class in this project
 * Part of its methods are here to initialize the necessary sockets and setup for future connections.
 * Others are here to handle each connected client.
 *
 * The Server Class has 'authHandler' object which helps it to login and register users.
 * 'authMutex' ensures that 'authHandler' methods are called atomically.
 * 'activeUsers' store the usernames of clients who logged in successfully. This is here to control
 * that no two clients can log in into the same user simultaneously.
 */
class Server {
public:
    Server(std::string  ip, int port, std::shared_ptr<AuthHandler> authHandler);
    ~Server();

    void start();
    void stop();

private:
    static void initializeWinSock();
    void setupListeningSocket();
    void acceptConnections();
    static std::vector<std::string> promptUser(SOCKET clientSocket, const std::vector<std::string>& messages);
    bool handleRegistration(SOCKET clientSocket);
    std::string handleLogin(SOCKET clientSocket);
    std::string loginRegistrationPhase(SOCKET clientSocket);
    void handleClient(SOCKET clientSocket);

    std::string ip;
    int port;
    SOCKET listeningSocket;
    std::shared_ptr<AuthHandler> authHandler;
    std::vector<std::thread> clientThreads;
    std::unordered_set<std::string> activeUsers;
    std::mutex authMutex;
    std::atomic<bool> running;
};

#endif // SERVER_H
