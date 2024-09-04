cmake_minimum_required(VERSION 3.10)

project(ChessMachine LANGUAGES CXX)

add_library(ChessMachine STATIC
    machine.cpp
    state.cpp
)

target_include_directories(ChessMachine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
)

find_package(Boost REQUIRED COMPONENTS system filesystem)

target_link_libraries(ChessMachine
    PUBLIC
        Boost::system
        Boost::filesystem
)

target_compile_options(ChessMachine PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Werror>
)
