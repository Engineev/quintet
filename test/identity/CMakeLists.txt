include(${CMAKE_SOURCE_DIR}/test/TestConfig)

include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB IDENTITY_TEST_SOURCE *.cpp)

add_executable(${PROJECT_NAME}_identity_test ${IDENTITY_TEST_SOURCE})

# link
target_link_libraries(${PROJECT_NAME}_identity_test ${PROJECT_NAME} ${Boost_LIBRARIES})

add_definitions(-D IDENTITY_TEST)