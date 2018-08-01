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

# dependecncies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

set(Boost_USE_STATIC_LIBS FALSE)
find_package(Boost REQUIRED
  COMPONENTS thread log unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

# link
find_package(GRPC REQUIRED)
include_directories(${GRPC_INCLUDE_DIR})

if (GEN_GRPC)
  message(STATUS "generating gRPC files...")
  execute_process(
    COMMAND bash ${CMAKE_SOURCE_DIR}/protobuf/gen_grpc_files.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/protobuf)
endif ()

add_subdirectory(src)
add_subdirectory(test)

