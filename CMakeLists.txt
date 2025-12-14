cmake_minimum_required(VERSION 3.28)

project(Bookstore)

include_directories(include)
include_directories(include/file_io)
include_directories(include/core)

add_executable(code main.cpp src/command.cpp src/manager.cpp src/models.cpp src/journal.cpp)
