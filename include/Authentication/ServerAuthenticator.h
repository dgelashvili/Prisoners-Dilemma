#ifndef SERVERAUTHENTICATOR_H
#define SERVERAUTHENTICATOR_H

#include <winsock2.h>
#include <string>
#include <mutex>
#include "AuthHandler.h"
#include <unordered_set>

/**
 * This class handles login and registration of a client using authHandler object
 * Uses authMutex so that authHandler method calls are atomically
 */
class ServerAuthenticator {
public:
    explicit ServerAuthenticator(std::shared_ptr<AuthHandler> authHandler);
    ~ServerAuthenticator() = default;

    bool handleRegistration(SOCKET clientSocket);
    std::string handleLogin(SOCKET clientSocket,
                            std::mutex& activeUsersMutex, const std::unordered_set<std::string> &activeUsers);
    std::string loginRegistrationPhase(SOCKET clientSocket,
                            std::mutex& activeUsersMutex, const std::unordered_set<std::string>& activeUsers);

private:
    std::mutex authMutex;
    std::shared_ptr<AuthHandler> authHandler;
};

#endif //SERVERAUTHENTICATOR_H
