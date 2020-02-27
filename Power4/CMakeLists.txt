cmake_minimum_required(VERSION 3.15)
project(Power4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(ai)
include_directories(game)
include_directories(utils)

set(COMPILE_LIST
        ai/mcts.h
        ai/mcts.cpp
        game/power4.h
        game/power4.cpp
        utils/utils.h
        utils/utils.cpp
)

set(EXEC_LIST
        ${COMPILE_LIST}
        main.cpp)

add_executable(Power4 ${EXEC_LIST})
