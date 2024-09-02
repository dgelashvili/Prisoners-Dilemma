#include "Server.h"
#include <iostream>
#include <utility>

Server::Server(std::string  ip, const int port, std::shared_ptr<AuthHandler> authHandler)
    : ip(std::move(ip)), port(port), listeningSocket(INVALID_SOCKET),
        authHandler(std::move(authHandler)), running(false) {
    initializeWinSock();
    setupListeningSocket();
}

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

    if (bind(listeningSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
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

void Server::start() {
    running = true;
    std::cout << "Server started on " << ip << ":" << port << std::endl;
    acceptConnections();
}

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
}

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

std::vector<std::string> Server::promptUser(const SOCKET clientSocket, const std::vector<std::string>& messages) {
    std::vector<std::string> userInput;
    char receiveBuffer[512];
    for (const auto & message : messages) {
        send(clientSocket, message.c_str(), static_cast<int>(strlen(message.c_str())), 0);
        const int receiveResult = recv(clientSocket, receiveBuffer, 512, 0);
        if (receiveResult <= 0) {
            return {};
        }
        receiveBuffer[receiveResult] = '\0';
        std::string receiveString(receiveBuffer);
        userInput.emplace_back(receiveString);
    }
    return userInput;
}

std::string Server::loginPhase(const SOCKET clientSocket) {
    while (true) {
        std::vector<std::string> userInput = promptUser(clientSocket, {"Enter command (REG/LOG/EXIT): "});
        if (userInput.empty()) {
            break;
        }
        if (const std::string& command = userInput[0]; command == "REG") {
            userInput = promptUser(clientSocket, {"Enter username: ", "Enter password: ", "Repeat password: "});
            if (userInput.empty()) {
                break;
            }

            std::lock_guard<std::mutex> lock(authMutex);
            std::string output = authHandler->registerUser(userInput[0], userInput[1], userInput[2]);
            send(clientSocket, output.c_str(), static_cast<int>(strlen(output.c_str())), 0);
        } else if (command == "LOG") {
            userInput = promptUser(clientSocket, {"Enter username: ", "Enter password: "});
            if (userInput.empty()) {
                break;
            }

            std::lock_guard<std::mutex> lock(authMutex);
            std::string output = authHandler->loginUser(userInput[0], userInput[1]);

            if (output.find("logged in successfully") != std::string::npos) {
                if (activeUsers.contains(userInput[0])) {
                    output = "You are already logged in.";
                } else {
                    send(clientSocket, output.c_str(), static_cast<int>(strlen(output.c_str())), 0);
                    activeUsers.insert(userInput[0]);
                    return userInput[0];
                }
            }

            send(clientSocket, output.c_str(), static_cast<int>(strlen(output.c_str())), 0);
        } else if (command == "EXIT") {
            const auto goodbye_message = "Goodbye!";
            send(clientSocket, goodbye_message, static_cast<int>(strlen(goodbye_message)), 0);
            break;
        } else {
            const auto unknown_message = "unknown command: " + command;
            send(clientSocket, unknown_message.c_str(), static_cast<int>(strlen(unknown_message.c_str())), 0);
        }
    }

    return "";
}


void Server::handleClient(const SOCKET clientSocket) {
    std::string username = loginPhase(clientSocket);
    if (username.empty()) {
        closesocket(clientSocket);
        return;
    }

    std::vector<std::string> userInput = promptUser(clientSocket, {"HELLO WORLD!\n(press X to exit): "});
    if (userInput.empty()) {
        closesocket(clientSocket);
        activeUsers.erase(username);
        return;
    }

    closesocket(clientSocket);
    activeUsers.erase(username);
}
