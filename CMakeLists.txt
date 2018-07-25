cmake_minimum_required(VERSION 3.5)

project(quintet)

set(CMAKE_CXX_STANDARD 14)
if (APPLE)
  message(STATUS "macOS detected")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall")
elseif(UNIX AND NOT APPLE)
  message(STATUS "Linux detected")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2 -Wall -pthread")
endif ()

include_directories(include/quintet)

set(Boost_USE_STATIC_LIBS FALSE)
find_package(Boost REQUIRED
  COMPONENTS thread log unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

add_subdirectory(src)
add_subdirectory(test)

