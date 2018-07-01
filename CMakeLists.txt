cmake_minimum_required(VERSION 3.5)

project(quintet)

# option
option(UNIT_TEST "unit test"
        ON)

# common flags
set(CMAKE_CXX_STANDARD 14)
if (MSVC)
  add_definitions(-D _WIN32_WINNT=0x600)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W4 /Zi /EHsc")
elseif (UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall -pthread")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall")
endif ()

include_directories(
  include/quintet
  dependencies/include
)

if (UNIT_TEST)
  add_definitions(-D UNIT_TEST)
endif ()

set(Boost_USE_STATIC_LIBS FALSE)
find_package(Boost REQUIRED COMPONENTS thread log unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

# source
add_subdirectory(dependencies/src)
add_subdirectory(src)

# test
if(UNIT_TEST)
  add_subdirectory(test)
endif()
