#!/bin/bash

# clang / LLVM Standard C++ Library (libc++) / c++11 support

# global variables

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

EXTRA_ARGS=""
if [ $# -ge 1 ] && [ $1 -gt 0 ]; then
    EXTRA_ARGS="${EXTRA_ARGS} --clear "
fi

if [ $# -ge 2 ]; then
    EXTRA_ARGS="${EXTRA_ARGS} --home ${2} "
fi

CURRENT_PATH=$(pwd)

# platform
SYSTEM_PLATFORM=Linux

# os
TOOLCHAIN="gcc"
#TOOLCHAIN="linux-gcc-x64"

# arch
VALID_ARCHS="x86;x86_64"

# build settings
BUILD_TYPE="Release"
BUILD_SHARED_LIBS=ON

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
             --test \
             --home ./core \
             --fwd  HUNTER_CONFIGURATION_TYPES=${BUILD_TYPE} \
                    BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
             --tail ${LOG_TAIL} \
             --jobs ${WORKERS}

}

# build_all
echo "DIR: ${DIR}"
echo "ROOT: ${DIR}/.."
(cd ${DIR}/.. && build_all)

# list of libcxx toolchains
# ls -l libcxx* | awk '{print $9}'

# libcxx toolchains available

#libcxx-fpic-hid-sections
#libcxx-hid-fpic
#libcxx-hid-sections
#libcxx-hid
#libcxx-omp

#libcxx

