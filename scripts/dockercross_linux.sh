#!/bin/bash

# Copyright (c) 2017, Luc Michalski
# All rights reserved.

clear

# global variables
CURRENT_MACHINE=$(uname -a)
CURRENT_PLATFORM=$(uname)
CURRENT_ARCH=$(uname -m)
CURRENT_OS=$(cat /etc/redhat-release)
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CURRENT_PATH=$(pwd)
CMAKE_EXECUTABLE=$(which cmake)
CMAKE_VERSION=$(cmake --version)

#WORKDIR="/work"
WORKDIR="/work/forks/dlib-hunter"
BUILD_PREFIX_PATH="${CURRENT_PATH}/_builds"

BUILD_TARGETS="install"
BUILD_SHARED_LIBS=ON
CMAKE_BUILD_TYPE="Release"

# generic:
# $ cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=`pwd`/_install
# $ cmake --build _builds/ --target install

# dlib:
# $ cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=`pwd`/_install -DDLIB_USE_CUDA=OFF

# dlib-hunter:
# $ rm -fR ./_* && cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=`pwd`/_install -DDLIB_USE_CUDA=OFF -DDLIB_PNG_SUPPORT=OFF

# hunter
# clear + re-install locally hunter
HUNTER_CLEAR_ALL=TRUE

echo
echo " ** machine info:"
echo "    - CURRENT_OS: ${CURRENT_OS}"
echo "    - CURRENT_ARCH: ${CURRENT_ARCH}"
echo "    - CURRENT_PLATFORM: ${CURRENT_PLATFORM}"
echo "    - CURRENT_MACHINE: ${CURRENT_MACHINE}"

echo
echo " ** workdir info:"
echo "    - DIR: ${DIR}"
echo "    - CURRENT_PATH: ${CURRENT_PATH}"

echo
echo " ** Dockercross info:"
echo "    - CURRENT_PLATFORM: ${CURRENT_PLATFORM}"
echo "    - CURRENT_ARCH: ${CURRENT_ARCH}"

echo
echo " ** CMake info"
echo "    - CMAKE_EXECUTABLE: ${CMAKE_EXECUTABLE}"
echo "    - CMAKE_VERSION: ${CMAKE_VERSION}"

echo
echo " ** CMake user-defined vars:"
echo "    - BUILD_TARGETS: ${BUILD_TARGETS}"
echo "    - BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}"
echo "    - CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}"

# /home/Luccio/hunter/_Base/

echo
echo " ** Hunter info"
HUNTER_ROOT="/home/Luccio/hunter"

if [ "${HUNTER_CLEAR_ALL}" ]
  then
	echo "    - HUNTER_CLEAR_ALL: $HUNTER_CLEAR_ALL"
	echo "        - deleting previous hunter install now..."
	rm -fR ${HUNTER_ROOT}
fi

mkdir -p ${HUNTER_ROOT}
export HUNTER_ROOT=${HUNTER_ROOT}
echo "    - HUNTER_ROOT: ${HUNTER_ROOT}"
echo "        - create folder ${HUNTER_ROOT}..."
echo "        - export env var \${HUNTER_ROOT}..."

echo
echo " ** Polly info"
echo "    - POLLY_ROOT: ${POLLY_ROOT}"

pwd

DOCKERCROSS_OUTSIDE_CMD="rm -fR ./$BUILD_PREFIX_PATH/$CURRENT_PLATFORM-$BUILD_ARCH/ pwd && cmake -B${BUILD_PREFIX_PATH}/$(basename ${WORKDIR})/$CURRENT_PLATFORM-$BUILD_ARCH -H${WORKDIR} -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} -DHUNTER_CONFIGURATION_TYPES=$CMAKE_BUILD_TYPE -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE && cmake --build $BUILD_PREFIX_PATH/$(basename ${WORKDIR})/$CURRENT_PLATFORM-$BUILD_ARCH --target $BUILD_TARGETS"

function build_project_outside
{
	BUILD_ARCH=$1
	echo
	echo "build_project_dockcross info: "
	echo "BUILD_ARCH: ${BUILD_ARCH}"
	echo
	./bin/dockercross/dockcross-manylinux-$BUILD_ARCH bash -c $DOCKERCROSS_OUTSIDE_CMD

}

function build_project_inside
{
	BUILD_ARCH=$1
	echo
	echo "build_project info: "
	echo "BUILD_ARCH: ${BUILD_ARCH}"
	echo
	echo "removing previous build files..."
	echo "located at: ${CURRENT_PATH}/${BUILD_PREFIX_PATH}/$(basename ${WORKDIR})/$CURRENT_PLATFORM-$BUILD_ARCH/"
	rm -fR ${CURRENT_PATH}/$BUILD_PREFIX_PATH/$(basename ${WORKDIR})/$CURRENT_PLATFORM-$BUILD_ARCH/ 
	echo
	echo "preparing project: "
	cmake -B${BUILD_PREFIX_PATH}/$(basename ${WORKDIR})/${CURRENT_PLATFORM}-$BUILD_ARCH -H${WORKDIR} -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS -DHUNTER_CONFIGURATION_TYPES=$CMAKE_BUILD_TYPE -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
	echo
	echo "building project: "
	echo " - all install targets"
	cmake --build $BUILD_PREFIX_PATH/$(basename ${WORKDIR})/$CURRENT_PLATFORM-$BUILD_ARCH --target $BUILD_TARGETS
	# ?!
	# eval(${DOCKERCROSS_OUTSIDE_CMD})
	echo
	echo "project build finsihed..."
	echo
}

# Inside (inside DOCKER CONTAINER with BASH command)
build_project_inside x86
#build_project_inside x86_64

# Ouside (from DOCKER RUN command)
#build_project_outside x86
#build_project_outside x86_64



