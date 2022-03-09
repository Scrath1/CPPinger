cmake_minimum_required(VERSION 3.18)
project(CPPinger)

find_package(Threads)

#set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS 11)

add_executable(CPPinger src/main.cpp src/icmplib/icmplib.h src/Logger/Logger.cpp src/Logger/Logger.h src/Logger/LogLevels.h)

# add dependencies
include(cmake/CPM.cmake)
CPMAddPackage(
        NAME yaml-cpp
        GITHUB_REPOSITORY jbeder/yaml-cpp
        GIT_TAG origin/master
        VERSION 0.7.0
)
if(yaml-cpp_ADDED)
#    add_library(yaml-cpp INTERFACE)
    target_include_directories(yaml-cpp INTERFACE ${yaml-cpp_SOURCE_DIR})
endif()
CPMAddPackage(
        NAME cpp-channel
        GITHUB_REPOSITORY andreiavrammsd/cpp-channel
        VERSION 0.7.0
)
if(cpp-channel_ADDED)
    add_library(cpp-channel INTERFACE)
    target_include_directories(cpp-channel INTERFACE ${cpp-channel_SOURCE_DIR})
endif()

target_link_libraries( CPPinger
        ${CMAKE_THREAD_LIBS_INIT}
        cpp-channel
        yaml-cpp)
