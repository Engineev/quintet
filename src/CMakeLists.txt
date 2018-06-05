file(GLOB_RECURSE LIB_SOURCE *.cpp)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCE} ${INCLUDE_DIRECTORIES})

# dependecncies
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR};${CMAKE_MODULE_PATH}")

find_package(Boost REQUIRED
  COMPONENTS thread serialization log
)
include_directories(${Boost_INCLUDE_DIR})
add_definitions(
  -D BOOST_THREAD_VERSION=4
  -D BOOST_LOG_DYN_LINK
)

# link
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
