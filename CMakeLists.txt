cmake_minimum_required(VERSION 3.13)
project(c4test)

set(CMAKE_CXX_STANDARD 14)

add_executable(c4test main.cpp C4Game.cpp C4AI.cpp C4AI.cpp C4Bot.cpp)