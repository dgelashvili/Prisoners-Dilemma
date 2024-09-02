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

    const auto userDao =
        std::make_shared<UserDAO>("C:/Users/demet/Desktop/PrisonersDilemma/database.sqlite");

    auto const lowerCaseChecker = std::make_shared<LowerCaseChecker>();
    auto const upperCaseChecker = std::make_shared<UpperCaseChecker>(lowerCaseChecker);
    auto const lengthChecker = std::make_shared<LengthChecker>(8, upperCaseChecker);

    const auto authHandler = std::make_shared<AuthHandler>(userDao, lengthChecker);

    Server server("127.0.0.1", 54000, authHandler);
    globalServer = &server;
    server.start();

    return 0;
}

