cmake_minimum_required(VERSION 3.10)

project(ChessMachineTest LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(SOURCES TestChessMachine.cpp)

add_executable(ChessMachineTest ${SOURCES})

target_link_libraries(ChessMachineTest PRIVATE Boost::unit_test_framework ChessMachine)

target_compile_options(ChessMachineTest PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Werror>
)

add_test(NAME ChessMachineTest COMMAND ChessMachineTest)
