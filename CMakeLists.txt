cmake_minimum_required(VERSION 3.10)
project(RawInputPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(RawInputPlugin SHARED
    RawInputPlugin.cpp
    RawInputStateTracker.cpp
    RawInputUnityForwarder.cpp
    RawInputWindow.cpp
)

target_include_directories(RawInputPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
