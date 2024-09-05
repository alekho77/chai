cmake_minimum_required(VERSION 3.10)

project(ChessEngineGreedyTest LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(SOURCES TestGreedyEngine.cpp)

add_executable(ChessEngineGreedyTest ${SOURCES})

target_link_libraries(ChessEngineGreedyTest PRIVATE Boost::unit_test_framework ChessEngineGreedy ChessMachine)

#target_compile_options(ChessEngineGreedyTest PRIVATE
#    $<$<CONFIG:Debug>:-Wall -Wextra -Werror>
#)

add_test(NAME ChessEngineGreedyTest COMMAND ChessEngineGreedyTest)
