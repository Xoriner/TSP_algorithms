cmake_minimum_required(VERSION 3.30)
project(pea)

set(CMAKE_CXX_STANDARD 20)

add_executable(pea
        src/main.cpp
        src/tsp_bruteforce.cpp
        src/rand.cpp
        src/nn.cpp
        src/rnn.cpp
        src/utils.cpp
        src/tsp_bruteforce.h)
