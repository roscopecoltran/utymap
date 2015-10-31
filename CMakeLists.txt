cmake_minimum_required (VERSION 3.0)
project ("UtyMap")

set(SOURCE ${PROJECT_SOURCE_DIR}/src)

set (BOOST_ROOT g:/__ASM/cpp/boost)
set (BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

# initialize boost package
find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
ENDIF()

add_subdirectory (src)
add_subdirectory (test)

add_definitions(-std=c++11)

enable_testing ()
add_test (NAME Tests COMMAND Test)
