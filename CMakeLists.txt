cmake_minimum_required(VERSION 3.5)

project(quintet)

# option
option(UNIT_TEST "unit test"
        ON)
option(FAULT_INJECTION "random sleep"
        ON)
option(MISC_TEST "test components and services"
        OFF)
option(IDENTITY_TEST "test the identities resepectively"
        ON)
option(COMPREHENSIVE_TEST "test with normal identity transformation enabled"
        OFF)

if (${IDENTITY_TEST} AND ${COMPREHENSIVE_TEST})
    message(FATAL_ERROR "identity test and comprehensive can not be enabled simultaneously.")
endif ()

# common flags
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W4 /EHsc")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall")
endif()
if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif ()

include_directories(
    ${CMAKE_SOURCE_DIR}/include/quintet
    ${CMAKE_SOURCE_DIR}/include/quintet/component
    ${CMAKE_SOURCE_DIR}/include/quintet/raft
    ${CMAKE_SOURCE_DIR}/include/quintet/raft/identity
    ${CMAKE_SOURCE_DIR}/include/quintet/raft/service
)

if (IDENTITY_TEST)
    add_definitions(-D IDENTITY_TEST)
endif ()
if (FAULT_INJECTION)
    add_definitions(-D FAULT_INJECTION)
endif ()

# config
configure_file (
    ${CMAKE_SOURCE_DIR}/QuintetConfig.h.in
    ${CMAKE_SOURCE_DIR}/include/quintet/QuintetConfig.h
)

file(GLOB_RECURSE INCLUDE_DIRECTORIES *.h)

# source
add_subdirectory(src)

# test
if(UNIT_TEST)
    add_subdirectory(test)
endif()
