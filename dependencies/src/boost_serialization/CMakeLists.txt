if (POLICY CMP0054)
    cmake_policy (SET CMP0054 NEW)
endif (POLICY CMP0054)

if (POLICY CMP0063)
    cmake_policy (SET CMP0063 NEW)
endif (POLICY CMP0063)

if(Boost_USE_STATIC_LIBS)
    project("Serialization-Static")
else()
    project("Serialization-Shared")
endif()

#
# Compiler settings
#

message(STATUS "compiler is ${CMAKE_CXX_COMPILER_ID}" )
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})

if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    add_definitions( -ftemplate-depth=300 )
    # we use gcc to test for C++03 compatibility
    add_definitions( -std=c++03 )
    message(STATUS "compiler is g++ c++03")
    set(COMPILER_SUPPORTS_CXX11 FALSE)
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
    add_definitions( /wd4996 )
    message(STATUS "compiler is MSVC")
    set(COMPILER_SUPPORTS_CXX11 TRUE)
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=300")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -dead_strip")
    set(COMPILER_SUPPORTS_CXX11 TRUE)
endif()

#
# IDE settings
#

if( CMAKE_HOST_APPLE )
    set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link to Boost static libraries")
    set(Boost_USE_MULTITHREADED ON)
else()
    set(Boost_ADDRESS_MODEL 64 CACHE INTEGER "32/64 bits")
    set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link to Boost static libraries")
    set(Boost_USE_MULTITHREADED ON)
endif()

set(Boost_DEBUG true)
set(Boost_DETAILED_FAILURE_MSG true)
set(Boost_FOUND true)

find_package(Boost REQUIRED COMPONENTS system filesystem)
include(CheckIncludeFileCXX)

message(STATUS "Boost_FOUND is ${Boost_FOUND}")

if(Boost_FOUND)
    message(STATUS "Boost Found!")
    message(STATUS "Boost is ${BOOST_ROOT}")
    message(STATUS "Boost directories found at ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries found at ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries prefix is  ${Boost_LIB_PREFIX}")
    message(STATUS "Boost component libraries to be linked are ${Boost_LIBRARIES}")
    message(STATUS "Boost version found is ${Boost_VERSION}")
    #include_directories("../include" "${Boost_INCLUDE_DIRS}")
    #link_directories("${Boost_LIBRARY_DIRS}")
else()
    message(STATUS "Boost NOT Found!")
endif()

if(Boost_USE_STATIC_LIBS)
    message(STATUS "Use static libraries")
    set(LINK_TYPE "STATIC")
else()
    message(STATUS "Building shared libraries")
    set(LINK_TYPE "SHARED")
    add_definitions( "-DBOOST_ALL_DYN_LINK=1")
    add_definitions( "-DBOOST_ALL_NO_LIB=1")
    add_definitions( "-DBOOST_LIB_DIAGNOSTICS=1")
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(VISIBILITY_INLINES_HIDDEN)
endif()

file(GLOB_RECURSE LIB_SOURCE *.cpp)
add_library(boost_serialization ${LINK_TYPE} ${LIB_SOURCE})
