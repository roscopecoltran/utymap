cmake_minimum_required (VERSION 3.0)
project ("UtyMap")

set(MAIN_SOURCE ${PROJECT_SOURCE_DIR}/src)
set(LIB_SOURCE ${PROJECT_SOURCE_DIR}/lib)
set(TEST_SOURCE ${PROJECT_SOURCE_DIR}/test)
set(SHARED_SOURCE ${PROJECT_SOURCE_DIR}/shared)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(shared)

add_definitions(-std=c++11)

enable_testing ()
add_test (NAME Tests COMMAND Test)
