cmake_minimum_required(VERSION 3.21)
project(Ping_test)

find_package(Threads)

set(CMAKE_CXX_STANDARD 23)

add_executable(Ping_test main.cpp icmplib/icmplib.h)
target_link_libraries( Ping_test ${CMAKE_THREAD_LIBS_INIT})

include(FetchContent)
if (NOT channel_POPULATED)
    FetchContent_Declare(channel URL https://github.com/andreiavrammsd/cpp-channel/archive/v0.7.0.zip)
    FetchContent_Populate(channel)
    include_directories(${channel_SOURCE_DIR}/include)
    # OR
    # add_subdirectory(${channel_SOURCE_DIR}/)
endif ()