#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <winsock2.h>
#include <string>
#include <vector>
#include <queue>

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

/**
 * Takes arguments 'username' and queue 'matchmakingQueue' and returns true if and only if there exists a pair in
 * the queue which contains 'username'. Does not change 'matchmakingQueue'
 */
inline bool findInQueue(const std::string& username, std::queue<std::pair<std::string, SOCKET>>& matchmakingQueue) {
    std::queue<std::pair<std::string, SOCKET>> queue;
    bool found = false;
    while (!matchmakingQueue.empty()) {
        auto pair = matchmakingQueue.front();
        found |= pair.first == username;
        matchmakingQueue.pop();
        queue.push(pair);
    }
    matchmakingQueue = queue;
    return found;
}

/**
 * Takes arguments 'username' and queue 'matchmakingQueue' and removes the pair in
 * the queue which contains 'username'. Changes 'matchmakingQueue'
 */
inline void removeFromQueue(const std::string& username, std::queue<std::pair<std::string, SOCKET>>& matchmakingQueue) {
    std::queue<std::pair<std::string, SOCKET>> queue;
    while (!matchmakingQueue.empty()) {
        auto pair = matchmakingQueue.front();
        matchmakingQueue.pop();
        if (pair.first != username) {
            queue.push(pair);
        }
    }
    matchmakingQueue = queue;
}

inline std::string border = "--------------------\n";

//This function makes a single row of a leaderboard table using 'info' pair
inline std::string makeRow(const std::pair<std::string, double>& info) {
    std::string username = info.first;
    if (username.size() > 8) {
        username = username.substr(0, 8);
    } else {
        int size = 8 - (int)username.size();
        while (size--) {
            username += " ";
        }
    }
    std::string score = std::to_string(info.second);
    if (score.size() > 5) {
        score = score.substr(0, 5);
    } else {
        int size = 5 - (int)score.size();
        while (size--) {
            score += " ";
        }
    }
    return  "| " + username + " | " + score + " |\n";
}

//This function makes a leaderboard table using 'info' vector of pairs
inline std::string makeTable(const std::vector<std::pair<std::string, double>>& info) {
    std::string result;
    result += border;
    result += "| Username |  Avg  |\n";
    result += border;
    for (const auto & singleInfo : info) {
        result += makeRow(singleInfo);
    }
    result += border;
    return result;
}

#endif //HELPERFUNCTIONS_H
