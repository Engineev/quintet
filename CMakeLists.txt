cmake_minimum_required(VERSION 3.5)

project(quintet)

# option
option(UNIT_TEST "unit test" ON)
option(RAFT_TEST "raft test" ON)

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
  include/quintet/rpc
  include/quintet/raft/rpc
)

if (UNIT_TEST)
  add_definitions(-D UNIT_TEST)
endif ()

set(Boost_USE_STATIC_LIBS FALSE)
find_package(Boost REQUIRED
  COMPONENTS thread log unit_test_framework serialization)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

# source
if (GEN_GRPC)
  add_subdirectory(protobuf)
endif ()
add_subdirectory(src)

# test
if(UNIT_TEST)
  add_subdirectory(test)
endif()
