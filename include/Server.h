#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <unordered_set>
#include <mutex>
#include "ServerAuthenticator.h"

#pragma comment(lib, "Ws2_32.lib")

/**
 * This is a Server Class, which is a main class in this project
 * Part of its methods are here to initialize the necessary sockets and setup for future connections.
 * Others are here to handle each connected client.
 *
 * The Server Class has 'ServerAuthenticator' object which helps it to login and register users.
 * 'activeUsers' store the usernames of clients who logged in successfully. This is here to control
 * that no two clients can log in into the same user simultaneously.
 */
class Server {
public:
    Server(std::string  ip, int port, std::shared_ptr<ServerAuthenticator> serverAuthenticator);
    ~Server();

    void start();
    void stop();

private:
    static void initializeWinSock();
    void setupListeningSocket();
    void acceptConnections();
    void handleClient(SOCKET clientSocket);

    std::string ip;
    int port;
    SOCKET listeningSocket;
    std::vector<std::thread> clientThreads;
    std::mutex activeUsersMutex;
    std::unordered_set<std::string> activeUsers;
    std::atomic<bool> running;
    std::shared_ptr<ServerAuthenticator> authenticator;
};

#endif // SERVER_H
