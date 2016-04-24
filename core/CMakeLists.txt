cmake_minimum_required (VERSION 2.8.9)
project ("UtyMap")

set(CMAKE_CXX_STANDARD 11)

set(MAIN_SOURCE ${PROJECT_SOURCE_DIR}/src)
set(LIB_SOURCE ${PROJECT_SOURCE_DIR}/lib)
set(TEST_SOURCE ${PROJECT_SOURCE_DIR}/test)
set(SHARED_SOURCE ${PROJECT_SOURCE_DIR}/shared)

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

enable_testing ()
add_test (NAME Tests COMMAND Test)
