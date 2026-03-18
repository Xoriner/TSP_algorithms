cmake_minimum_required(VERSION 3.30)
project(pea)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

set(CMAKE_CXX_STANDARD 20)

# Lista plików źródłowych
set(SOURCES
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
        src/algorithms/tsp_rnn.h
)

add_executable(pea ${SOURCES})

add_custom_command(TARGET pea POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/config.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/config.txt"
        COMMENT "Kopiowanie config.txt do folderu z exe"
)

add_custom_command(TARGET pea POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/instances"
        "${CMAKE_CURRENT_BINARY_DIR}/instances"
        COMMENT "Kopiowanie folderu instances do folderu z exe"
)