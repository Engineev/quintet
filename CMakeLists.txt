cmake_minimum_required(VERSION 3.5)
project(quintet)

set(CMAKE_CXX_STANDARD 14)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall")
endif()

include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/test/*.cpp
        ${CMAKE_SOURCE_DIR}/quintet/*.cpp
        ${CMAKE_SOURCE_DIR}/quintet/raft/*.cpp

        ${CMAKE_SOURCE_DIR}/test/*.h
        ${CMAKE_SOURCE_DIR}/quintet/*.h
        ${CMAKE_SOURCE_DIR}/quintet/raft/*.h
        )

add_executable(quintet ${SOURCE_FILES})

# boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS
        unit_test_framework
        thread
        serialization
        )
include_directories(${Boost_INCLUDE_DIR})

# librpc
find_package(rpclib REQUIRED)
include_directories(${Rpclib_INCLUDE_DIR})

# link
target_link_libraries(quintet
        rpclib::rpc
        ${Boost_LIBRARIES}
        )

# config
add_definitions( -DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}" )