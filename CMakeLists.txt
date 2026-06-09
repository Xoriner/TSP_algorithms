cmake_minimum_required(VERSION 3.27)
project(pea)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pea
        src/main.cpp
        src/utilities/utils.cpp
        src/algorithms/tsp_nn.cpp
        src/algorithms/tsp_aco.cpp
        src/algorithms/tsp_aco.h
)

target_include_directories(pea PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)