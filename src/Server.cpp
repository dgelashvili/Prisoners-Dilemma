#include "Server.h"
#include <iostream>
#include <utility>

//starts initializing WinSock and setups 'listeningSocket'
Server::Server(std::string  ip, const int port, std::shared_ptr<AuthHandler> authHandler)
    : ip(std::move(ip)), port(port), listeningSocket(INVALID_SOCKET),
        authHandler(std::move(authHandler)), running(false) {
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
 *Changes state of 'running' ro false
 *Closes 'listeningSocket' if one is valid.
 *Cleans up WSA
 *Joins every thread which is stored in clientThreads
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
 * Takes vector of strings as a parameter and makes the simplest dialogue between server and user
 * Server starts the dialogue with the first string
 * Client responds
 * Server responds etc.
 *
 * Returns all answers of user
 */
std::vector<std::string> Server::promptUser(const SOCKET clientSocket, const std::vector<std::string>& messages) {
    std::vector<std::string> userInput;
    char receiveBuffer[512];
    for (const auto & message : messages) {
        send(clientSocket, message.c_str(), (int)(strlen(message.c_str())), 0);
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

/**
 * Using 'promptUser()', makes a registration dialogue between server and client.
 * If user disconnected returns false, otherwise true
 * Uses authHandler to send corresponding message to the client
 */
bool Server::handleRegistration(const SOCKET clientSocket) {
    const std::vector<std::string> userInput = promptUser(clientSocket,
        {"Enter username: ", "Enter password: ", "Repeat password: "});
    if (userInput.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(authMutex);
    const std::string output = authHandler->registerUser(
        userInput[0], userInput[1], userInput[2]);
    send(clientSocket, output.c_str(), (int)(strlen(output.c_str())), 0);

    return true;
}

/**
 * Using 'promptUser()', makes a login dialogue between server and client.
 * If user disconnected returns '~' as the username
 * Otherwise, if user logged in successfully, returns real username
 * Else, returns empty string, meaning a 'default' username
 * Uses authHandler to send corresponding message to the client
 */
std::string Server::handleLogin(const SOCKET clientSocket) {
    std::vector<std::string> userInput = promptUser(clientSocket,
        {"Enter username: ", "Enter password: "});
    if (userInput.empty()) {
        return "~";
    }

    std::lock_guard<std::mutex> lock(authMutex);
    std::string output = authHandler->loginUser(userInput[0], userInput[1]);

    if (output.find("logged in successfully") != std::string::npos) {
        if (activeUsers.contains(userInput[0])) {
            output = "You are already logged in.";
        } else {
            send(clientSocket, output.c_str(), (int)(strlen(output.c_str())), 0);
            return userInput[0];
        }
    }

    send(clientSocket, output.c_str(), (int)(strlen(output.c_str())), 0);
    return "";
}

/**
 * Asks client to choose between logging in, registration or exiting the application.
 * Depending on scenario, starts corresponding dialogue
 * Unless user disconnects, choose to exit or logs in successfully, this function runs in infinite loop
 *
 * Once the loop ends, if client logged in successfully, returns username of the user
 * Otherwise, returns empty string, meaning a 'default' username
 */
std::string Server::loginRegistrationPhase(const SOCKET clientSocket) {
    while (true) {
        std::vector<std::string> userInput = promptUser(clientSocket, {"Enter command (REG/LOG/EXIT): "});
        if (userInput.empty()) {
            break;
        }
        if (const std::string& command = userInput[0]; command == "REG") {
            if (!handleRegistration(clientSocket)) {
                break;
            }
        } else if (command == "LOG") {
            std::string username = handleLogin(clientSocket);
            if (username == "~") {
                break;
            }
            if (!username.empty()) {
                return username;
            }
        } else if (command == "EXIT") {
            const auto goodbye_message = "Goodbye!";
            send(clientSocket, goodbye_message, (int)(strlen(goodbye_message)), 0);
            break;
        } else {
            const auto unknown_message = "unknown command: " + command;
            send(clientSocket, unknown_message.c_str(), (int)(strlen(unknown_message.c_str())), 0);
        }
    }

    return "";
}

/**
 * Starts with loginRegistrationPhase, if client logs in successfully, stores the username in 'activeUsers'
 * so while logged in, no other client can log in using the same account
 */
void Server::handleClient(const SOCKET clientSocket) {
    std::string username = loginRegistrationPhase(clientSocket);
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
