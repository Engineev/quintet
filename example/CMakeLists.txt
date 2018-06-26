include_directories(../src)

add_executable(chatroom chatRoom.cpp)
add_executable(example example.cpp)

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
target_link_libraries(chatroom ${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(example ${PROJECT_NAME} ${Boost_LIBRARIES})

# config
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
