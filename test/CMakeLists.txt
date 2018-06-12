include_directories(../src)

file(GLOB TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

if (MISC_TEST)
    file(GLOB_RECURSE TMP ${CMAKE_CURRENT_LIST_DIR}/misc/*.cpp)
    set(TEST_SOURCE ${TEST_SOURCE} ${TMP})
endif ()
if (IDENTITY_TEST)
    file(GLOB_RECURSE TMP ${CMAKE_CURRENT_LIST_DIR}/identity/*.cpp)
    set(TEST_SOURCE ${TEST_SOURCE} ${TMP})
endif ()

add_executable(${PROJECT_NAME}_test ${TEST_SOURCE})

# boost
find_package(Boost REQUIRED
  COMPONENTS unit_test_framework thread serialization log
)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

# link
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} ${Boost_LIBRARIES})

# config
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
