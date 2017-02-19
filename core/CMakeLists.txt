cmake_minimum_required (VERSION 3.1)

# IMPORTANT: just need to create 2 new variables: ${PROJECT_NAME}, ${PROJECT_VERSION}, as they cannot be declared before HunterGate is included
set(current_project_name "utymap") # PROJECT_NAME: ${current_project_name}
set(current_project_version v0.1.0) # PROJECT_VERSION: ${current_project_version}
set(current_project_root_path ${CMAKE_CURRENT_LIST_DIR})

string(COMPARE NOTEQUAL "$ENV{TRAVIS_TAG}" "" travis_deploy)
string(COMPARE EQUAL "$ENV{APPVEYOR_REPO_TAG}" "true" appveyor_deploy)

if(travis_deploy)
  set(version "$ENV{TRAVIS_TAG}")
elseif(appveyor_deploy)
  set(version "$ENV{APPVEYOR_REPO_TAG_NAME}")
else()
  set(version "${current_project_version}")
endif()

string(REGEX REPLACE "^v" "" version "${version}")

MESSAGE(STATUS "version: ${version}")

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.18.1.tar.gz"
    SHA1 "c1aa0ef5191e4484889866c62263a64f1ee61d3e"
)

MESSAGE(STATUS "version: ${version}")
project (UtyMap VERSION ${version})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(HUNTER_ENABLED 		"Enable Hunter package manager support" ON)
option(ENABLE_STATIC 		"Build static library" 					ON)
option(ENABLE_SHARED 		"Build shared library" 					ON)
option(ENABLE_UNITYPLUGIN 	"Build plugin for Unity" 				ON)
option(BUILD_TESTS 			"Build tests" 							OFF)

set(is_android ${ANDROID}) # syntax compatibility
string(COMPARE EQUAL "${CMAKE_OSX_SYSROOT}" "iphoneos" is_ios)
string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Linux" is_linux)
string(COMPARE EQUAL "${CMAKE_GENERATOR}" "Xcode" is_xcode)

# force static library for iOS
if(is_ios)
	SET(ENABLE_SHARED OFF)
endif(is_ios)

# See: cmake/Hunter/config.cmake
# First call to 'hunter_add_package' will contains read of Hunter/config.cmake
# Next variables will be used:
#   * is_ios
#   * is_linux
hunter_add_package(Sugar)
include("${SUGAR_ROOT}/cmake/Sugar")
include(sugar_include)

if(is_xcode)
  option(${PROJECT_NAME}_DISABLE_DSYM "Disable generation of dSYM information" ON)
  if(${PROJECT_NAME}_DISABLE_DSYM)
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf")
  endif()
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo" CACHE STRING "Configuration types")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE NO)
endif()

set(CMAKE_CXX_STANDARD 11)
set(MAIN_SOURCE ${PROJECT_SOURCE_DIR}/src)
set(LIB_SOURCE ${PROJECT_SOURCE_DIR}/lib)
set(TEST_SOURCE ${PROJECT_SOURCE_DIR}/test)
set(SHARED_SOURCE ${PROJECT_SOURCE_DIR}/shared)

MESSAGE(STATUS " - Project: ")
MESSAGE(STATUS "   - HUNTER_ENABLED: ${SHARED_SOURCE}")
MESSAGE(STATUS "   - ENABLE_STATIC: ${SHARED_SOURCE}")
MESSAGE(STATUS "   - ENABLE_SHARED: ${SHARED_SOURCE}")
MESSAGE(STATUS "   - ENABLE_UNITYPLUGIN: ${ENABLE_UNITYPLUGIN}")
MESSAGE(STATUS "   - CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
MESSAGE(STATUS "   - MAIN_SOURCE: ${MAIN_SOURCE}")
MESSAGE(STATUS "   - LIB_SOURCE: ${LIB_SOURCE}")
MESSAGE(STATUS "   - SHARED_SOURCE: ${SHARED_SOURCE}")
MESSAGE(STATUS "   - BUILD_TESTS: ${BUILD_TESTS}")
MESSAGE(STATUS "   - TEST_SOURCE: ${TEST_SOURCE}")

# ---[ Boost
IF(HUNTER_ENABLED)
	hunter_add_package(Boost)
	find_package(Boost CONFIG REQUIRED)
	list(APPEND ${PROJECT_NAME}_LINKER_LIBS Boost::boost)
	IF(Boost_FOUND)
		add_definitions(-DHAS_BOOST_HUNTER)
		add_definitions(-DHAS_BOOST)
	ENDIF(Boost_FOUND)
ELSE()
	find_package(Boost REQUIRED)
	IF (Boost_FOUND)
	    include_directories(${Boost_INCLUDE_DIRS})
	    link_directories(${Boost_LIBRARY_DIRS})
	    add_definitions("-DHAS_BOOST")
	ENDIF()
ENDIF()

# ---[ ZLIB
IF(HUNTER_ENABLED)
	hunter_add_package(ZLIB)
	find_package(ZLIB CONFIG REQUIRED) # ZLIB::zlib
	list(APPEND ${PROJECT_NAME}_LINKER_LIBS ZLIB::zlib)
	IF(ZLIB_FOUND)
		add_definitions(-DHAS_ZLIB_HUNTER)
		add_definitions(-DHAS_ZLIB)
	ENDIF(ZLIB_FOUND)
else()
	find_package(ZLIB REQUIRED)
	include_directories(${ZLIB_INCLUDE_DIR})
endif()

# ---[ Protobuf
IF(HUNTER_ENABLED)
	# If we cross compile for Android or iOS build a separate protoc executable on host to compile .proto files in CMake
	if(IOS OR ANDROID)
	  include(hunter_experimental_add_host_project)
	  hunter_experimental_add_host_project(cmake/host)
	  add_executable(protobuf::protoc IMPORTED)
	  set_target_properties(protobuf::protoc PROPERTIES IMPORTED_LOCATION "${HUNTER_HOST_ROOT}/bin/protoc")
	endif(IOS OR ANDROID)
	hunter_add_package(Protobuf)
	find_package(Protobuf CONFIG REQUIRED)	
	IF(Protobuf_FOUND)
		add_definitions(-DHAS_PROTOBUF_HUNTER)
		add_definitions(-DHAS_PROTOBUF)
	ENDIF(Protobuf_FOUND)
else()
	find_package(Protobuf REQUIRED)
	include_directories(${PROTOBUF_INCLUDE_DIR})
endif()

add_subdirectory(src)

if(BUILD_TESTS)
	add_subdirectory(test)
endif(BUILD_TESTS)

add_subdirectory(shared)
