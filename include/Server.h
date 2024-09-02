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
    std::string loginPhase(SOCKET clientSocket);
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
