cmake_minimum_required(VERSION 3.29)
project(PrisonersDilemma)

set(CMAKE_CXX_STANDARD 20)

include_directories(include sqlite include/Checkers include/Authentication include/Game)

add_executable(PrisonersDilemma main.cpp
                sqlite/sqlite3.h sqlite/sqlite3.c
                src/Authentication/UserDao.cpp src/Authentication/User.cpp src/Authentication/AuthHandler.cpp
                src/Checkers/LengthChecker.cpp src/Checkers/UpperCaseChecker.cpp src/Checkers/LowerCaseChecker.cpp
                src/Server.cpp src/Authentication/ServerAuthenticator.cpp
                src/Game/Match.cpp src/Game/MatchDao.cpp src/Game/GameSession.cpp
)

target_link_libraries(PrisonersDilemma Ws2_32)
