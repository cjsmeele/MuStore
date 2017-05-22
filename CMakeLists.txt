cmake_minimum_required(VERSION 2.6)

project(mustore)

set(sources
    src/scalestore.cc
    src/fsnode.cc
    src/memstore.cc
    src/fs.cc
    src/fatfs.cc)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    # Assume we are cross-compiling for a mcu.
    #message(${CMAKE_SYSTEM_NAME})
else()
    # Assume we are compiling for a PC platform - add cstdio store
    # backend.
    set(sources ${sources}
        src/filestore.cc)

    enable_testing()
    # TODO: Port tests to cmake.
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -Wall -Wextra -pedantic")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#add_library(mustore SHARED ${sources})
#add_library(mustore-static ${sources})

add_library(mustore ${sources})
