file(GLOB TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

if (RAFT_TEST)
  file(GLOB_RECURSE TMP ${CMAKE_CURRENT_LIST_DIR}/raft/*.cpp)
  set(TEST_SOURCE ${TEST_SOURCE} ${TMP})
endif ()

add_executable(${PROJECT_NAME}_test ${TEST_SOURCE})

# link
target_link_libraries(
  ${PROJECT_NAME}_test
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
)

# config
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
