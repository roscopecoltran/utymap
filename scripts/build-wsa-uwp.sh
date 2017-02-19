#!/bin/bash

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

# 
# https://docs.microsoft.com/en-us/windows/uwp/get-started/whats-a-uwp

# toolchain os/arch list
TOOLCHAINS_LIST="vs-14-2015-arm,ARM;vs-14-2015-sdk-8-1,x86;vs-14-2015-win64-sdk-8-1,x64"
VALID_ARCH="Win32;Win64"

# platform
SYSTEM_PLATFORM="Windows Universal APP"

echo "Not ready yet"
exit 1

# build settings
BUILD_TYPE=Release
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
	         --fwd 	BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
			        HUNTER_CONFIGURATION_TYPES=${BUILD_TYPE} \
			        CMAKE_BUILD_TYPE=${BUILD_TYPE} \
	         --tail ${LOG_TAIL} \
	         --jobs ${WORKERS}

}

# build_all
echo "DIR: ${DIR}"
echo "ROOT: ${DIR}/.."
(cd ${DIR}/.. && build_all)

# list of toolchains:
# ls -l vs* | awk '{print $9}'

# visual studio / wsa toolchains available

#vs-14-2015-arm
#vs-14-2015-sdk-8-1
#vs-14-2015-win64-sdk-8-1
#vs-14-2015-win64
#vs-14-2015

