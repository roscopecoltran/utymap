cmake_minimum_required (VERSION 3.0)
project ("UtyMap")

set(SOURCE ${PROJECT_SOURCE_DIR}/src)
set(TEST_SOURCE ${PROJECT_SOURCE_DIR}/test)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (BOOST_ROOT g:/__ASM/cpp/boost)
set (BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

# TODO improve the way how libs are included
set (PROTOBUF_INCLUDE_DIR g:/__ASM/cpp/protobuf/src)
set (PROTOBUF_LIBRARY g:/__ASM/cpp/protobuf/vsprojects/Debug/libprotobuf.lib)

set (ZLIB_INCLUDE_DIR g:/__ASM/cpp/zlib)
set (ZLIB_LIBRARY g:/__ASM/cpp/zlib/zlibd.lib)

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
include_directories(${ZLIB_INCLUDE_DIR})

add_subdirectory (src)
add_subdirectory (test)

add_definitions(-std=c++11)

enable_testing ()
add_test (NAME Tests COMMAND Test)
