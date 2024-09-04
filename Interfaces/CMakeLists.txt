cmake_minimum_required(VERSION 3.10)

project(Interfaces LANGUAGES CXX)

add_library(Interfaces INTERFACE)

target_include_directories(Interfaces
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(Interfaces INTERFACE cxx_std_17)
