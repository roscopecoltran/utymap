cmake_minimum_required (VERSION 3.0)
project ("UtyMap")

set(SOURCE ${PROJECT_SOURCE_DIR}/src)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (BOOST_ROOT /cygdrive/d/Dev/_libs/boost_1_59_0)
set (BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

set (PROTOBUF_INCLUDE_DIR /cygdrive/d/Dev/_libs/protobuf-2.6.1/src)
set (PROTOBUF_LIBRARY /cygdrive/d/Dev/_libs/protobuf-2.6.1/src)

set (ZLIB_INCLUDE_DIRS /cygdrive/d/Dev/_libs/zlib-1.2.8)

# initialize boost
find_package(Boost REQUIRED)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
ENDIF()

#initialize protobuf package
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

#initialize zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

add_subdirectory (src)
add_subdirectory (test)

add_definitions(-std=c++11)

enable_testing ()
add_test (NAME Tests COMMAND Test)
