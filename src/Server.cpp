#include "Server.h"
#include <iostream>
#include <utility>
#include "HelperFunctions.h"

//starts initializing WinSock and setups 'listeningSocket'
Server::Server(std::string  ip, const int port, std::shared_ptr<ServerAuthenticator> serverAuthenticator)
    : ip(std::move(ip)), port(port), listeningSocket(INVALID_SOCKET),
        running(false), authenticator(std::move(serverAuthenticator)) {
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
 * Starts with loginRegistrationPhase, if client logs in successfully, stores the username in 'activeUsers'
 * so while logged in, no other client can log in using the same account
 */
void Server::handleClient(const SOCKET clientSocket) {
    std::string username = authenticator->loginRegistrationPhase(clientSocket, activeUsers);
    if (username.empty()) {
        closesocket(clientSocket);
        return;
    }
    activeUsers.insert(username);

    std::vector<std::string> userInput = promptUser(clientSocket, {"HELLO WORLD!\n(press X to exit): "});
    if (userInput.empty()) {
        closesocket(clientSocket);
        activeUsers.erase(username);
        return;
    }

    closesocket(clientSocket);
    activeUsers.erase(username);
}
