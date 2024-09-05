#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <winsock2.h>
#include <string>
#include <vector>


/**
 * Wrapper of send function
 */
inline void sendToClient(const SOCKET clientSocket, const std::string& message) {
    send(clientSocket, message.c_str(), (int)message.size(), 0);
}

/**
 * Wrapper of receive function
 */
inline std::string receiveFromClient(const SOCKET clientSocket) {
    char buffer[512];
    if (const int result = recv(clientSocket, buffer, 512, 0); result > 0) {
        buffer[result] = '\0';
        std::string res = buffer;
        return res;
    }
    return "";
}

/**
 * Takes vector of strings as a parameter and makes the simplest dialogue between server and user
 * Server starts the dialogue with the first string
 * Client responds
 * Server responds etc.
 *
 * Returns all answers of user
 */
inline std::vector<std::string> promptUser(const SOCKET clientSocket, const std::vector<std::string>& messages) {
    std::vector<std::string> userInput;
    for (const auto & message : messages) {
        sendToClient(clientSocket, message);
        std::string receiveString = receiveFromClient(clientSocket);
        if (receiveString.empty()) {
            return {};
        }
        userInput.emplace_back(receiveString);
    }
    return userInput;
}

#endif //HELPERFUNCTIONS_H
