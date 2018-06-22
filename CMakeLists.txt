cmake_minimum_required(VERSION 3.5)

project(quintet)

# option
option(UNIT_TEST "unit test"
        ON)
option(MISC_TEST "misc test"
        ON)
option(IDENTITY_TEST "test the identities resepectively"
        ON)
option(COMPREHENSIVE_TEST "test with normal identity transformation enabled"
        OFF)

if (${IDENTITY_TEST} AND ${COMPREHENSIVE_TEST})
  message(FATAL_ERROR
    "identity test and comprehensive can not be enabled simultaneously.")
endif ()

# common flags
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
    add_definitions(-D _WIN32_WINNT=0x600)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W4 /EHsc")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall -fno-limit-debug-info")
endif()
if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif ()

include_directories(
  ${CMAKE_SOURCE_DIR}/include/quintet
  ${CMAKE_SOURCE_DIR}/include/quintet/raft
)

if (UNIT_TEST)
    add_definitions(-D UNIT_TEST)
    if (IDENTITY_TEST)
        add_definitions(-D IDENTITY_TEST)
    endif ()
endif ()

# source
add_subdirectory(src)

# test
if(UNIT_TEST)
    add_subdirectory(test)
endif()
