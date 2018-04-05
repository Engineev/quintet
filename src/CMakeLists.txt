file(GLOB_RECURSE LIB_SOURCE *.cpp)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCE})

# dependecncies
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED
        COMPONENTS thread serialization
        )
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_THREAD_VERSION=4)

# librpc
find_package(rpclib REQUIRED)
include_directories(${Rpclib_INCLUDE_DIR})

# link
target_link_libraries(${PROJECT_NAME} rpclib::rpc ${Boost_LIBRARIES})
