cmake_minimum_required(VERSION 3.28)

project(Bookstore)

include_directories(include)

add_executable(code main.cpp src/command.cpp src/manager.cpp src/models.cpp)
