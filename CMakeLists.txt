cmake_minimum_required(VERSION 3.29)
project(PrisonersDilemma)

set(CMAKE_CXX_STANDARD 20)

include_directories(include sqlite include/Checkers)

add_executable(PrisonersDilemma main.cpp
                sqlite/sqlite3.h sqlite/sqlite3.c
                src/UserDao.cpp src/User.cpp src/AuthHandler.cpp
                src/Checkers/LengthChecker.cpp src/Checkers/UpperCaseChecker.cpp src/Checkers/LowerCaseChecker.cpp
                src/Server.cpp src/ServerAuthenticator.cpp
                src/GameSession.cpp
)

target_link_libraries(PrisonersDilemma Ws2_32)
