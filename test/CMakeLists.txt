include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

if (MISC_TEST)
    file(GLOB_RECURSE TMP ${CMAKE_CURRENT_LIST_DIR}/Misc/*.cpp)
    set(TEST_SOURCE ${TEST_SOURCE} ${TMP})
endif ()
if (IDENTITY_TEST)
    file(GLOB_RECURSE TMP ${CMAKE_CURRENT_LIST_DIR}/Identity/*.cpp)
    set(TEST_SOURCE ${TEST_SOURCE} ${TMP})
endif ()
if (COMPREHENSIVE_TEST)
    file(GLOB_RECURSE TMP ${CMAKE_CURRENT_LIST_DIR}/Comprehensive/*.cpp)
    set(TEST_SOURCE ${TEST_SOURCE} ${TMP})
endif ()

add_executable(${PROJECT_NAME}_test ${TEST_SOURCE})

# boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED
        COMPONENTS unit_test_framework thread serialization
        )
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_THREAD_VERSION=4)

# link
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} ${Boost_LIBRARIES})

# config
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
