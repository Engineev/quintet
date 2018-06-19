file(GLOB_RECURSE LIB_SOURCE *.cpp *.cc)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCE} ${INCLUDE_DIRECTORIES})

# dependecncies
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR};${CMAKE_MODULE_PATH}")

find_package(Boost REQUIRED
  COMPONENTS thread serialization log
)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

find_package(GRPC REQUIRED)
include_directories(${GRPC_INCLUDE_DIR})

# link
if(VCPKG_TOOLCHAIN)
  target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES} protobuf::libprotobuf
    gRPC::grpc++_unsecure)
else()
  target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} ${GRPC_LIBRARIES})
endif()
