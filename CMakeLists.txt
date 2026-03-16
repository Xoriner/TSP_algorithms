cmake_minimum_required(VERSION 3.30)
project(pea)

set(CMAKE_CXX_STANDARD 20)

add_executable(pea
        src/main.cpp
        src/tsp_bruteforce.cpp
        src/rand.cpp
        src/tsp_nn.cpp
        src/rnn.cpp
        src/utilities/utils.cpp
        src/tsp_bruteforce.h
        src/utilities/utils.h
        src/utilities/tsp_result.h
        src/tsp_nn.h)
