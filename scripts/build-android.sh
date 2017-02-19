#!/bin/bash

# Important notes

# forexample
# 

# https://github.com/mixpanel/mixpanel-unity/blob/master/native/build/build_all.py
# ANDROID_NDK = os.environ.get('ANDROID_NDK', os.path.expanduser('~/Library/Android/ndk'))
# ANDROID_SDK = os.environ.get('ANDROID_SDK', os.path.expanduser('~/Library/Android/sdk'))
# os.environ['PATH'] += os.pathsep + os.path.join(ANDROID_SDK, 'platform-tools')

#				'-DANDROID_NDK=' + ANDROID_NDK,
#				'-DANDROID_ABI=' + self.abi,
#				'-DANDROID_STL=%s' % self.stl,
#				'-DANDROID_NOEXECSTACK=OFF',

# global variables
CURRENT_PATH=$(pwd)

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

EXTRA_ARGS=""
if [ $# -ge 1 ]; then
	EXTRA_ARGS="--clear --reconfig"
fi

# Android Studio v2.1.0 (April 2016)

# android-ndk-r11c-api-21-armeabi-v7a-neon
# android-ndk-r11c-api-21-x86
# android-ndk-r11c-api-21-x86-64
# android-ndk-r11c-api-21-arm64-v8a

# android-ndk-r10e-api-21-arm64-v8a-clang-35
# android-ndk-r10e-api-21-arm64-v8a-gcc-49-hid-sections
# android-ndk-r10e-api-21-arm64-v8a-gcc-49-hid
# android-ndk-r10e-api-21-arm64-v8a-gcc-49
# android-ndk-r10e-api-21-arm64-v8a
# android-ndk-r10e-api-21-armeabi-v7a-neon-clang-35
# android-ndk-r10e-api-21-armeabi-v7a-neon-hid-sections
# android-ndk-r10e-api-21-armeabi-v7a-neon
# android-ndk-r10e-api-21-armeabi-v7a
# android-ndk-r10e-api-21-armeabi
# android-ndk-r10e-api-21-mips
# android-ndk-r10e-api-21-mips64
# android-ndk-r10e-api-21-x86-64-hid-sections
# android-ndk-r10e-api-21-x86-64-hid
# android-ndk-r10e-api-21-x86-64
# android-ndk-r10e-api-21-x86

# Must be <= 19 for qt qml camera:
# TOOLCHAIN=android-ndk-r10e-api-21-armeabi-v7a-neon

# toolchain os/arch list
# android_abis = ('armeabi-v7a','arm64-v8a', 'x86', 'x64')
ANDROID_NDK_VERSION="r10e"
ANDROID_API_VERSION="21"

#ANDROID_ABI=arm64-v8a
#ANDROID_ABI=x86-64
#ANDROID_ABI=x86
ANDROID_ABI=armeabi-v7a
#ANDROID_ABI=mips64
#ANDROID_ABI=mips

# toolchain list
TOOLCHAINS_LIST="armeabi-v7a|android-ndk-${ANDROID_NDK_VERSION}-api-${ANDROID_API_VERSION}-armeabi-v7a-neon;\
				arm64-v8a|android-ndk-${ANDROID_NDK_VERSION}-api-${ANDROID_API_VERSION}-arm64-v8a;\
				x86|android-ndk-${ANDROID_NDK_VERSION}-api-${ANDROID_API_VERSION}-x86;\
				x64|android-ndk-${ANDROID_NDK_VERSION}-api-${ANDROID_API_VERSION}-x86-64"

# arch (4 archs)
VALID_ARCHS="armeabi-v7a;arm64-v8a;x86;x64"

rename_tab nativex $TOOLCHAIN

[ -n ${ANDROID_SDK_ROOT} ] && unset ANDROID_SDK_ROOT
[ -n ${ANDROID_SDK_ROOT} ] && unset ANDROID_HOME

echo "ANDROID_SDK_ROOT: ${ANDROID_SDK_ROOT}"
echo "ANDROID_HOME: ${ANDROID_HOME}"

printenv | grep -i android

printenv | grep -i DOCKER

TOOLCHAIN=android-ndk-${ANDROID_NDK_VERSION}-api-${ANDROID_API_VERSION}-${ANDROID_ABI}

# Must be <= 19 for qt qml camera:
#TOOLCHAIN=${ANDROID_TOOLCHAIN}

rename_tab utymap $TOOLCHAIN

[ -n ${ANDROID_SDK_ROOT} ] && unset ANDROID_SDK_ROOT
[ -n ${ANDROID_SDK_ROOT} ] && unset ANDROID_HOME

echo $ANDROID_SDK_ROOT

#echo "not ready yet"
#exit 1

# build settings 'Release'
BUILD_TYPE="Release"
# android_stls = ('c++_static', )
BUILD_SHARED_LIBS=ON

# output settings
LOG_TAIL=25
WORKERS=4

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
	         --fwd 	ANDROID=TRUE \
			        ANDROID_NDK_VERSION=${ANDROID_NDK_VERSION} \
			        ANDROID_API_VERSION=${ANDROID_API_VERSION} \
			        ANDROID_ABI=${ANDROID_ABI} \
	         		BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
			        HUNTER_CONFIGURATION_TYPES=${BUILD_TYPE} \
			        DLIB_GIF_SUPPORT=OFF \
	         --tail ${LOG_TAIL} \
	         --jobs ${WORKERS}

}

# build_all
echo "DIR: ${DIR}"
echo "ROOT: ${DIR}/.."
(cd ${DIR}/.. && build_all)

# list of android toolchains
# ls -l android* | awk '{print $9}'

# android toolchains available

# android-ndk-r10e-api-16-armeabi-v7a-neon-clang-35-hid-sections
# android-ndk-r10e-api-16-armeabi-v7a-neon-clang-35-hid
# android-ndk-r10e-api-16-armeabi-v7a-neon-clang-35
# android-ndk-r10e-api-16-armeabi-v7a-neon
# android-ndk-r10e-api-16-x86-hid-sections
# android-ndk-r10e-api-16-x86-hid
# android-ndk-r10e-api-16-x86

# android-ndk-r10e-api-19-armeabi-v7a-neon-hid-sections
# android-ndk-r10e-api-19-armeabi-v7a-neon

# android-ndk-r10e-api-21-arm64-v8a-clang-35
# android-ndk-r10e-api-21-arm64-v8a-gcc-49-hid-sections
# android-ndk-r10e-api-21-arm64-v8a-gcc-49-hid
# android-ndk-r10e-api-21-arm64-v8a-gcc-49
# android-ndk-r10e-api-21-arm64-v8a
# android-ndk-r10e-api-21-armeabi-v7a-neon-clang-35
# android-ndk-r10e-api-21-armeabi-v7a-neon-hid-sections
# android-ndk-r10e-api-21-armeabi-v7a-neon
# android-ndk-r10e-api-21-armeabi-v7a
# android-ndk-r10e-api-21-armeabi
# android-ndk-r10e-api-21-mips
# android-ndk-r10e-api-21-mips64
# android-ndk-r10e-api-21-x86-64-hid-sections
# android-ndk-r10e-api-21-x86-64-hid
# android-ndk-r10e-api-21-x86-64
# android-ndk-r10e-api-21-x86

# android-ndk-r10e-api-8-armeabi-v7a

# android-ndk-r11c-api-16-armeabi-v7a-neon-clang-35-hid
# android-ndk-r11c-api-16-armeabi-v7a-neon-clang-35
# android-ndk-r11c-api-16-armeabi-v7a-neon
# android-ndk-r11c-api-16-armeabi-v7a
# android-ndk-r11c-api-16-armeabi
# android-ndk-r11c-api-16-x86-hid
# android-ndk-r11c-api-16-x86
# android-ndk-r11c-api-19-armeabi-v7a-neon

# android-ndk-r11c-api-21-arm64-v8a-clang-35
# android-ndk-r11c-api-21-arm64-v8a-gcc-49-hid
# android-ndk-r11c-api-21-arm64-v8a-gcc-49
# android-ndk-r11c-api-21-arm64-v8a
# android-ndk-r11c-api-21-armeabi-v7a-neon-clang-35
# android-ndk-r11c-api-21-armeabi-v7a-neon
# android-ndk-r11c-api-21-armeabi-v7a
# android-ndk-r11c-api-21-armeabi
# android-ndk-r11c-api-21-mips
# android-ndk-r11c-api-21-mips64
# android-ndk-r11c-api-21-x86-64-hid
# android-ndk-r11c-api-21-x86-64
# android-ndk-r11c-api-21-x86

# android-ndk-r11c-api-8-armeabi-v7a
# android-ndk-r12b-api-19-armeabi-v7a-neon

# android-ndk-r13b-api-19-armeabi-v7a-neon

# android-vc-ndk-r10e-api-19-arm-clang-3-6
# android-vc-ndk-r10e-api-19-arm-gcc-4-9

# android-vc-ndk-r10e-api-19-x86-clang-3-6
# android-vc-ndk-r10e-api-21-arm-clang-3-6

# URL for ndks

# r10e
