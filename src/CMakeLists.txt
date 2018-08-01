include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_SOURCE_DIR}/dependencies/theatre/include
)

file(GLOB_RECURSE LIB_SOURCE *.cpp *.cc *.h)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCE} ${INCLUDE_DIRECTORIES})

target_link_libraries(
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  ${GRPC_LIBRARIES}
)
