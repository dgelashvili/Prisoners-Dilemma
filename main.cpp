#include <iostream>
#include "UserDAO.h"
#include <memory>

#include "AuthHandler.h"
#include "LengthChecker.h"
#include "UpperCaseChecker.h"
#include "LowerCaseChecker.h"
#include "Server.h"
#include <csignal>

Server* globalServer = nullptr;

void signalHandler(int signal) {
    if (globalServer) {
        globalServer->stop();
    }
}

int main() {
    std::signal(SIGINT, signalHandler);

    std::string dbPath = "../database.sqlite";

    const auto userDao = std::make_shared<UserDAO>(dbPath);
    const auto matchDao = std::make_shared<MatchDAO>(dbPath);

    auto const lowerCaseChecker = std::make_shared<LowerCaseChecker>();
    auto const upperCaseChecker = std::make_shared<UpperCaseChecker>(lowerCaseChecker);
    auto const lengthChecker = std::make_shared<LengthChecker>(8, upperCaseChecker);

    const auto authHandler = std::make_shared<AuthHandler>(userDao, lengthChecker);
    const auto serverAuthenticator = std::make_shared<ServerAuthenticator>(authHandler);

    Server server("127.0.0.1", 54000, serverAuthenticator, matchDao);
    globalServer = &server;
    server.start();

    return 0;
}

