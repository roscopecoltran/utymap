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
SYSTEM_PLATFORM="iOS"

# toolchain
TOOLCHAIN="ios-10-2"
DEPLOYMENT_TARGET="8.2"

# os
VALID_OS=iphoneos
#VALID_OS=iphoneos;iphonesimulator

# arch
VALID_ARCHS="armv7;arm64"
VALID_ARCHS_SIM="i386;x86_64"

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

    build.py    --toolchain=${TOOLCHAIN} \
                --config ${BUILD_TYPE} \
                --verbosity-level full \
                --jobs 4 \
                --reconfig \
                --clear \
                --home ./core \
                --tail ${LOG_TAIL} \
                --ios-combined \
                --ios-multiarch \
                --fwd   BUILD_SHARED_LIBS=OFF \
                        ENABLE_UNITYPLUGIN=ON \
                        BUILD_TESTS=ON \
                        CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET=${DEPLOYMENT_TARGET} \
                        CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=${BUILD_ONLY_ACTIVE_ARCH} \
                        HUNTER_CONFIGURATION_TYPES=${BUILD_TYPE}

}

# build_all
echo "DIR: ${DIR}"
echo "ROOT: ${DIR}/.."
(cd ${DIR}/.. && build_all)

# list of ios toolchains
# ls -l ios* | awk '{print $9}'

# iOS toolchains available

# ios-10-0-arm64-dep-8-0-hid-sections
# ios-10-0-arm64
# ios-10-0-armv7
# ios-10-0-dep-8-0-hid-sections
# ios-10-0-wo-armv7s
# ios-10-0

# ios-10-1-arm64-dep-8-0-hid-sections
# ios-10-1-arm64
# ios-10-1-armv7

# ios-10-1-dep-8-0-hid-sections
# ios-10-1-dep-8-0-libcxx-hid-sections
# ios-10-1-wo-armv7s
# ios-10-1

# ios-10-2

# ios-7-0
# ios-7-1
# ios-8-0
# ios-8-1

# ios-8-2-arm64-hid
# ios-8-2-arm64
# ios-8-2-cxx98
# ios-8-2-i386-arm64
# ios-8-2

# ios-8-4-arm64
# ios-8-4-armv7
# ios-8-4-armv7s
# ios-8-4-hid
# ios-8-4

# ios-9-0-armv7
# ios-9-0-dep-7-0-armv7
# ios-9-0-i386-armv7
# ios-9-0-wo-armv7s
# ios-9-0

# ios-9-1-arm64
# ios-9-1-armv7
# ios-9-1-dep-7-0-armv7
# ios-9-1-dep-8-0-hid
# ios-9-1-hid
# ios-9-1

# ios-9-2-arm64
# ios-9-2-armv7
# ios-9-2-hid-sections
# ios-9-2-hid
# ios-9-2

# ios-9-3-arm64
# ios-9-3-armv7
# ios-9-3-wo-armv7s
# ios-9-3

# ios-nocodesign-10-0-arm64
# ios-nocodesign-10-0-armv7
# ios-nocodesign-10-0-wo-armv7s
# ios-nocodesign-10-0

# ios-nocodesign-10-1-arm64
# ios-nocodesign-10-1-armv7
# ios-nocodesign-10-1-wo-armv7s
# ios-nocodesign-10-1

# ios-nocodesign-8-4

# ios-nocodesign-9-1-arm64
# ios-nocodesign-9-1-armv7
# ios-nocodesign-9-1

# ios-nocodesign-9-2-arm64
# ios-nocodesign-9-2-armv7
# ios-nocodesign-9-2

# ios-nocodesign-9-3-arm64
# ios-nocodesign-9-3-armv7
# ios-nocodesign-9-3-device-hid-sections
# ios-nocodesign-9-3-device
# ios-nocodesign-9-3-wo-armv7s
# ios-nocodesign-9-3

# ios-nocodesign-arm64
# ios-nocodesign-armv7

# ios-nocodesign-hid-sections
# ios-nocodesign-wo-armv7s
# ios-nocodesign
