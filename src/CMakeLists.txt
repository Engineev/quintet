include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE LIB_SOURCE *.cpp *.cc *.h)


add_library(${PROJECT_NAME} STATIC ${LIB_SOURCE} ${INCLUDE_DIRECTORIES})

# dependecncies
#set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR};${CMAKE_MODULE_PATH}")
#
#find_package(Protobuf REQUIRED)
#include_directories(${PROTOBUF_INCLUDE_DIR})

# link
#find_package(GRPC REQUIRED)
#include_directories(${GRPC_INCLUDE_DIR})
target_link_libraries(
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${CMAKE_SOURCE_DIR}/dependencies/theatre/include
#  ${PROTOBUF_LIBRARIES}
#  ${GRPC_LIBRARIES}
)
