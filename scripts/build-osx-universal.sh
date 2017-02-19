#!/bin/bash

# Copyright (c) 2017, Luc Michalski
# All rights reserved.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

EXTRA_ARGS=""
[ $# -ge 1 ] && EXTRA_ARGS="--clear"

# global variables
CURRENT_PATH=$(pwd)

# platform
SYSTEM_PLATFORM="MacOSX"

# os
TOOLCHAIN="osx-10-12"
DEPLOYMENT_TARGET="10.10"

# arch
VALID_ARCHS="i386;x86_64"

# build settings
BUILD_ONLY_ACTIVE_ARCH=NO
BUILD_TYPE="Release"
BUILD_SHARED_LIBS=OFF

# output settings
LOG_TAIL=25
WORKERS=4

rename_tab utymap $SYSTEM_PLATFORM

echo "Start build for ${TOOLCHAIN} (${VALID_ARCHS})"

function build_all
{

    build.py --toolchain=${TOOLCHAIN} \
             --verbosity-level full \
             --config ${BUILD_TYPE} \
             --reconfig \
             --clear \
             --home ./core \
    	     --universal-apple \
             --test \
             --fwd CMAKE_XCODE_ATTRIBUTE_OSX_DEPLOYMENT_TARGET=${DEPLOYMENT_TARGET} \
                   CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=${BUILD_ONLY_ACTIVE_ARCH} \
                   BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
                   CMAKE_OSX_DEPLOYMENT_TARGET=${DEPLOYMENT_TARGET} \
    	           HUNTER_CONFIGURATION_TYPES=${BUILD_TYPE} \
             --tail ${LOG_TAIL} \
             --jobs ${WORKERS}

}

# build_all
echo "DIR: ${DIR}"
echo "ROOT: ${DIR}/.."
(cd ${DIR}/.. && build_all)

#(cd ${DIR}/.. && build_all)

# list of osx toolchains
# ls -l osx* | awk '{print $9}'

# osx-10-12-cxx98
# osx-10-12-make
# osx-10-12-ninja
# osx-10-12-sanitize-address-hid-sections
# osx-10-12-sanitize-address
# osx-10-12

# osx-10-10-dep-10-7
# osx-10-10-dep-10-9-make
# osx-10-10

# osx-10-11-hid-sections
# osx-10-11-make
# osx-10-11-sanitize-address
# osx-10-11

# osx-10-7
# osx-10-8
# osx-10-9
