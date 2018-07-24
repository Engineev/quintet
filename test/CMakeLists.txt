file(GLOB TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_executable(${PROJECT_NAME}_test ${TEST_SOURCE})

# link
target_link_libraries(
  ${PROJECT_NAME}_test
  ${Boost_LIBRARIES}
)

# config
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
