include(TestConfig)

include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB TEST_SOURCE *.cpp)

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

if (IDENTITY_TEST)
    add_subdirectory(identity)
endif ()