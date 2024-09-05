cmake_minimum_required(VERSION 3.10)

project(ChessEngineGreedy LANGUAGES CXX)

add_library(ChessEngineGreedy STATIC
    engine.cpp
)

target_include_directories(ChessEngineGreedy
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
)

find_package(Boost REQUIRED COMPONENTS system filesystem thread)

target_link_libraries(ChessEngineGreedy
    PUBLIC
        Boost::system
        Boost::filesystem
        Boost::thread
)

target_compile_options(ChessEngineGreedy PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Werror>
)
