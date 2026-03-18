cmake_minimum_required(VERSION 3.30)
project(pea)

set(CMAKE_CXX_STANDARD 20)

add_executable(pea
        src/main.cpp
        src/algorithms/tsp_bruteforce.cpp
        src/algorithms/tsp_rand.cpp
        src/algorithms/tsp_nn.cpp
        src/algorithms/tsp_rnn.cpp
        src/utilities/utils.cpp
        src/algorithms/tsp_bruteforce.h
        src/utilities/utils.h
        src/utilities/tsp_result.h
        src/algorithms/tsp_nn.h
        src/utilities/tsp_nn_equal.h
        src/utilities/tsp_nn_equal.cpp
        src/algorithms/tsp_rand.h
        src/utilities/read_config.cpp
        src/utilities/read_config.h
        src/algorithms/tsp_rnn.h)
