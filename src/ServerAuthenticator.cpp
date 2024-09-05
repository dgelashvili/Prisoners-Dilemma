#include "ServerAuthenticator.h"

#include <thread>

#include "HelperFunctions.h"
#include <vector>

ServerAuthenticator::ServerAuthenticator(std::shared_ptr<AuthHandler> authHandler)
    : authHandler(std::move(authHandler)) {}

/**
 * Using 'promptUser()', makes a registration dialogue between server and client.
 * If user disconnected returns false, otherwise true
 * Uses authHandler to send corresponding message to the client
 */
bool ServerAuthenticator::handleRegistration(const SOCKET clientSocket) {
    const std::vector<std::string> userInput = promptUser(clientSocket,
        {"Enter username: ", "Enter password: ", "Repeat password: "});
    if (userInput.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(authMutex);
    const std::string output = authHandler->registerUser(
        userInput[0], userInput[1], userInput[2]);
    sendToClient(clientSocket, output);

    return true;
}

/**
 * Using 'promptUser()', makes a login dialogue between server and client.
 * If user disconnected returns '~' as the username
 * Otherwise, if user logged in successfully, returns real username
 * Else, returns empty string, meaning a 'default' username
 * Uses authHandler to send corresponding message to the client
 */
std::string ServerAuthenticator::handleLogin(const SOCKET clientSocket,
                std::mutex& activeUsersMutex, const std::unordered_set<std::string> &activeUsers) {
    std::vector<std::string> userInput = promptUser(clientSocket,
        {"Enter username: ", "Enter password: "});
    if (userInput.empty()) {
        return "~";
    }

    std::lock_guard<std::mutex> lock(authMutex);
    std::string output = authHandler->loginUser(userInput[0], userInput[1]);

    if (output.find("logged in successfully") != std::string::npos) {
        std::lock_guard<std::mutex> lock2(activeUsersMutex);
        if (activeUsers.contains(userInput[0])) {
            output = "You are already logged in.";
        } else {
            sendToClient(clientSocket, output);
            return userInput[0];
        }
    }

    sendToClient(clientSocket, output);
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
std::string ServerAuthenticator::loginRegistrationPhase(const SOCKET clientSocket,
                                std::mutex& activeUsersMutex, const std::unordered_set<std::string>& activeUsers) {
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
            std::string username = handleLogin(clientSocket, activeUsersMutex, activeUsers);
            if (username == "~") {
                break;
            }
            if (!username.empty()) {
                return username;
            }
        } else if (command == "EXIT") {
            const auto goodbye_message = "Goodbye!";
            sendToClient(clientSocket, goodbye_message);
            break;
        } else {
            const auto unknown_message = "unknown command: " + command;
            sendToClient(clientSocket, unknown_message);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return "";
}
