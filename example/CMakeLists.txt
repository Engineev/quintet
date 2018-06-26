include_directories(../src)

file(GLOB EXP_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

# todo if
add_executable(${PROJECT_NAME}_exp ${EXP_SOURCE})

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
target_link_libraries(${PROJECT_NAME}_exp ${PROJECT_NAME} ${Boost_LIBRARIES})

# config
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
