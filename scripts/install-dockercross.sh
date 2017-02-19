#!/bin/bash

################################################################################
#
# Copyright (c) 2017, Luc Michalski
# All rights reserved.
#
# DOCKERCROSS Plugin
# Repository: https://github.com/dockcross/dockcross.git
#
################################################################################

clear

CURRENT_DIR=$(pwd)
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo 
echo "CURRENT_DIR: ${CURRENT_DIR}"
echo "DIR: ${DIR}"
echo 

################################################################################
# install dockercross dependencies (Docker)
################################################################################

  #######################
  # for Mac 
  #######################
  # brew install docker

  #######################
  # for Ubuntu  
  #######################
  # apt-get install docker

################################################################################
# install dockercross locally
################################################################################

  # dockercross paths settings
  DOCKERCROSS_SHARED_VOLUME=${HUNTER_ROOT}/dockcross
  echo "DOCKERCROSS_SHARED_VOLUME: ${DOCKERCROSS_SHARED_VOLUME}"
  echo 

  # dockercross shared volume (required by docker-machine, docker-unison, crane)
  mkdir -p ${DOCKERCROSS_SHARED_VOLUME}
  DOCKERCROSS_INSTALL_PREFIX=${POLLY_ROOT}/bin/docker
  DOCKERCROSS_TESTS_PREFIX_PATH="${POLLY_ROOT}/tests"
  echo "DOCKERCROSS_INSTALL_PREFIX: ${DOCKERCROSS_INSTALL_PREFIX}"
  echo "DOCKERCROSS_TEST1_PREFIX_PATH: ${DOCKERCROSS_TEST1_PREFIX_PATH}"
  echo 

  mkdir -p ${DOCKERCROSS_INSTALL_PREFIX}
  ls -l $DOCKERCROSS_INSTALL_PREFIX

  mkdir -p $DOCKERCROSS_TESTS_PREFIX_PATH
  ls -l $DOCKERCROSS_TESTS_PREFIX_PATH

  # other cross-compilers
  # https://github.com/znly/docker-cross-compilers (aarch64, aarch64-linaro)
  # https://github.com/droboports/docker-cross-compiler
  # https://github.com/Dovetail-Automata/docker-cross-builder (i386, armhf using gcc --sysroot)
  # 

################################################################################
#
# cross-compilation tests (OpenCV, flatbuffers, dlib)
#
# light projects:
# - https://github.com/headupinclouds/local_laplacian_filters
# - https://github.com/headupinclouds/cvsteer
#
# medium projects:
# - https://github.com/headupinclouds/facelab
# - https://github.com/headupinclouds/dest
#
# long projects:
# - https://github.com/headupinclouds/gatherer
#
################################################################################

  # ┌──────────────────────────────────────────────────────────────────┐
  # │  Test 1 -  Dockercross + Hunter + CMake + Linux x86_64           |
  # └──────────────────────────────────────────────────────────────────┘

  # project vcs settings
  DOCKERCROSS_TEST1_GIT_REPOSITORY="https://github.com/headupinclouds/cvsteer"
  DOCKERCROSS_TEST1_GIT_REPOSITORY_CLONE_DEPTH=1
  DOCKERCROSS_TEST1_GIT_REPOSITORY_BRANCH="master"
  DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME=$(basename ${DOCKERCROSS_TEST1_GIT_REPOSITORY})

  # project settings
  DOCKERCROSS_TEST1_PROJECT_PATH=${DOCKERCROSS_TESTS_PREFIX_PATH}/${DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME}
  DOCKERCROSS_TEST1_PROJECT_BUILD_TYPE=Debug
  DOCKERCROSS_TEST1_PROJECT_PARALLEL_JOBS=4
  DOCKERCROSS_TEST1_BUILD_PATH=""

  #
  # toolchain: manylinux-x64
  # description: Docker manylinux image for building Linux x86_64 / amd64 Python wheel packages. 
  #              Also has support for the dockcross script, and it has installations of CMake, Ninja, and scikit-build
  #
  DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN=manylinux-x64

  echo "DOCKERCROSS_TEST1_GIT_REPOSITORY: ${DOCKERCROSS_TEST1_GIT_REPOSITORY}"
  echo "DOCKERCROSS_TEST1_GIT_REPOSITORY_CLONE_DEPTH: ${DOCKERCROSS_TEST1_GIT_REPOSITORY_CLONE_DEPTH}"
  echo "DOCKERCROSS_TEST1_GIT_REPOSITORY_BRANCH: ${DOCKERCROSS_TEST1_GIT_REPOSITORY_BRANCH}"
  echo "DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME: ${DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME}"
  echo 

  # ┌──────────────────────────────────────────────────────────────────┐
  # │  Test 2 -  Dockercross + Unity Plugin + CMake + Windows x86_64   |
  # └──────────────────────────────────────────────────────────────────┘

  # project vcs settings
  DOCKERCROSS_TEST2_GIT_REPOSITORY="https://github.com/mixpanel/mixpanel-unity"
  DOCKERCROSS_TEST2_GIT_REPOSITORY_CLONE_DEPTH=1
  DOCKERCROSS_TEST2_GIT_REPOSITORY_BRANCH="master"
  DOCKERCROSS_TEST2_GIT_REPOSITORY_NAME=$(basename ${DOCKERCROSS_TEST2_GIT_REPOSITORY})

  # project settings
  DOCKERCROSS_TEST2_PROJECT_PATH=${DOCKERCROSS_TESTS_PREFIX_PATH}/${DOCKERCROSS_TEST2_GIT_REPOSITORY_NAME}
  DOCKERCROSS_TEST2_PROJECT_BUILD_TYPE=Debug
  DOCKERCROSS_TEST2_PROJECT_PARALLEL_JOBS=4
  DOCKERCROSS_TEST2_BUILD_PATH="native/build/"
  DOCKERCROSS_TEST2_BUILD_DEPS="sh ${DOCKERCROSS_TEST2_PROJECT_PATH}/native/build/install_dependencies.sh"
  DOCKERCROSS_TEST2_BUILD_CMD="python build_all.py --platforms=windows"

  # note:
  # python build_all.py # on OS X
  # build_all.py # on Windows

  #
  # toolchain: windows-x64
  # description: 64-bit Windows cross-compiler based on MXE/MinGW-w64.
  #
  DOCKERCROSS_TEST2_PROJECT_CROSS_COMPILER_TOOLCHAIN=windows-x64

  echo "DOCKERCROSS_TEST2_GIT_REPOSITORY: ${DOCKERCROSS_TEST2_GIT_REPOSITORY}"
  echo "DOCKERCROSS_TEST2_GIT_REPOSITORY_CLONE_DEPTH: ${DOCKERCROSS_TEST2_GIT_REPOSITORY_CLONE_DEPTH}"
  echo "DOCKERCROSS_TEST2_GIT_REPOSITORY_BRANCH: ${DOCKERCROSS_TEST2_GIT_REPOSITORY_BRANCH}"
  echo "DOCKERCROSS_TEST2_GIT_REPOSITORY_NAME: ${DOCKERCROSS_TEST2_GIT_REPOSITORY_NAME}"
  echo 

# ┌──────────────────────────────────────────────────────────────────┐
# │  Download all images                                             │
# └──────────────────────────────────────────────────────────────────┘

  # These images are built using the "build implicit rule"
  DOCKERCROSS_STANDARD_IMAGES="android-arm \
                              linux-x86 \
                              linux-x64 \
                              linux-arm64 \
                              linux-armv5 \
                              linux-armv6 \
                              linux-armv7 \
                              linux-mipsel \
                              linux-ppc64le \
                              windows-x86 \
                              windows-x64"

  # These images are expected to have explicit rules for *both* build and testing
  DOCKERCROSS_NON_STANDARD_IMAGES="browser-asmjs \
                                  manylinux-x64 \
                                  manylinux-x86"

  echo 
  echo "Download all dockercross images"
  echo "DOCKERCROSS_STANDARD_IMAGES: ${DOCKERCROSS_STANDARD_IMAGES[*]}"
  echo "DOCKERCROSS_NON_STANDARD_IMAGES: ${DOCKERCROSS_NON_STANDARD_IMAGES[*]}"

  curl -s https://raw.githubusercontent.com/dockcross/dockcross/master/Makefile -o dockcross-Makefile
  for image in $(make -f dockcross-Makefile display_images); do
    echo "Pulling dockcross/$image"
    docker pull dockcross/$image
  done

# ┌──────────────────────────────────────────────────────────────────┐
# │  Install all dockcross scripts                                   │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Install all dockcross scripts"

  curl -s https://raw.githubusercontent.com/dockcross/dockcross/master/Makefile -o dockcross-Makefile
  #cat dockcross-Makefile
  for image in $(make -f dockcross-Makefile display_images); do
    if [[ $(docker images -q dockcross/$image) == "" ]]; then
      echo "${DOCKERCROSS_INSTALL_PREFIX}/dockcross-$image skipping: image not found locally"
      continue
    fi
    echo "${DOCKERCROSS_INSTALL_PREFIX}/dockcross-$image ok"
    docker run dockcross/$image > ${DOCKERCROSS_INSTALL_PREFIX}/dockcross-$image && \
    chmod u+x ${DOCKERCROSS_INSTALL_PREFIX}/dockcross-$image
  done

tree ${DOCKERCROSS_INSTALL_PREFIX}

################################################################################
# test1: dockercross + cmake + hunter
################################################################################

# ┌──────────────────────────────────────────────────────────────────┐
# │  Clone test project                                              │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Test dockcross compilation locally with hunterized project..."
  echo
  echo "Cloning \"${DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME}\" project (Hunterized)..."
  echo
  git clone 	--recursive \
  			      --depth=${DOCKERCROSS_TEST1_GIT_REPOSITORY_CLONE_DEPTH} \
  			      -b ${DOCKERCROSS_TEST1_GIT_REPOSITORY_BRANCH} \
  			      ${DOCKERCROSS_TEST1_GIT_REPOSITORY} \
  			      ${DOCKERCROSS_TESTS_PREFIX_PATH}/${DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME}
  echo
  echo "Checking cloned content in: ${DOCKERCROSS_TEST1_PREFIX_PATH}/${DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME}"
  echo
  tree ${DOCKERCROSS_TEST1_PROJECT_PATH}

# ┌──────────────────────────────────────────────────────────────────┐
# │  Generate project                                                │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Generating project..."
  cd ${DOCKERCROSS_TEST1_PROJECT_PATH}/${DOCKERCROSS_TEST1_BUILD_PATH}
  pwd
  echo "Delete previous builds temporary files..."
  rm -fR ${DOCKERCROSS_TEST1_PROJECT_PATH}/_*
  echo "$: dockcross-${DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN} cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=${DOCKERCROSS_TEST1_PROJECT_BUILD_TYPE}"
  dockcross-${DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN} cmake -H. -B_builds -DHUNTER_STATUS_DEBUG=ON -DCMAKE_BUILD_TYPE=${DOCKERCROSS_TEST1_PROJECT_BUILD_TYPE}

# ┌──────────────────────────────────────────────────────────────────┐
# │  Run build                                                       │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Running build..."
  cd ${DOCKERCROSS_TEST1_PROJECT_PATH}
  pwd
  echo "$: dockcross-${DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN} cmake --build _builds --config ${DOCKERCROSS_TEST1_PROJECT_BUILD_TYPE} -- -j ${DOCKERCROSS_TEST1_PROJECT_PARALLEL_JOBS}"
  dockcross-${DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN} cmake --build _builds --config ${DOCKERCROSS_TEST1_PROJECT_BUILD_TYPE} -- -j ${DOCKERCROSS_TEST1_PROJECT_PARALLEL_JOBS}

# ┌──────────────────────────────────────────────────────────────────┐
# │  Run tests                                                       │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Running tests..."
  pwd
  cd ${DOCKERCROSS_TEST1_PROJECT_PATH}/_builds 
  echo "$: dockcross-${DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN} ctest -C Debug -VV"
  dockcross-${DOCKERCROSS_TEST1_PROJECT_CROSS_COMPILER_TOOLCHAIN} ctest -C Debug -VV

################################################################################
# test2: dockercross + cmake + windows x64 + Unity3D
################################################################################

# ┌──────────────────────────────────────────────────────────────────┐
# │  Clone test project                                              │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Test dockcross compilation locally with hunterized project..."
  echo
  echo "Cloning \"${DOCKERCROSS_TEST1_GIT_REPOSITORY_NAME}\" project (Hunterized)..."
  echo
  git clone   --recursive \
              --depth=${DOCKERCROSS_TEST2_GIT_REPOSITORY_CLONE_DEPTH} \
              -b ${DOCKERCROSS_TEST2_GIT_REPOSITORY_BRANCH} \
              ${DOCKERCROSS_TEST2_GIT_REPOSITORY} \
              ${DOCKERCROSS_TESTS_PREFIX_PATH}/${DOCKERCROSS_TEST2_GIT_REPOSITORY_NAME}
  echo
  echo "Checking cloned content in: ${DOCKERCROSS_TEST2_PREFIX_PATH}/${DOCKERCROSS_TEST2_GIT_REPOSITORY_NAME}"
  echo
  tree ${DOCKERCROSS_TEST2_PROJECT_PATH}

# ┌──────────────────────────────────────────────────────────────────┐
# │  Run build                                                       │
# └──────────────────────────────────────────────────────────────────┘

  echo
  echo "Running build..."
  cd ${DOCKERCROSS_TEST2_PROJECT_PATH}/${DOCKERCROSS_TEST2_BUILD_PATH}
  pwd

  echo "$: dockcross ${DOCKERCROSS_TEST2_BUILD_CMD}"
  dockcross-${DOCKERCROSS_TEST2_PROJECT_CROSS_COMPILER_TOOLCHAIN} --args "-e HUNTER_ROOT=${HUNTER_ROOT}" ${DOCKERCROSS_TEST2_BUILD_CMD}
  # dockcross-windows-x64 --args "-e HUNTER_ROOT=${HUNTER_ROOT}" bash -c 'echo $HUNTER_ROOT;'
  # dockcross-windows-x64 --args "-e HUNTER_ROOT=${HUNTER_ROOT}" bash -c '$CC test/C/hello.c -o hello'
  # dockcross-windows-x64 --args "-e HUNTER_ROOT=${HUNTER_ROOT}" bash -c 'apt-get update && sudo apt-get install -y swig git; sh ./install_dependencies.sh; python build_all.py --platforms=windows'
dockcross-windows-x64 --args "-e HUNTER_ROOT=${HUNTER_ROOT}" su root apt-get upgrade && apt-get update && sudo apt-get install -y swig git
dockcross-windows-x64 --args "-e HUNTER_ROOT=${HUNTER_ROOT}" bash -c 'su visudo -i'

# ┌──────────────────────────────────────────────────────────────────┐
# │  Output install report                                           │
# └──────────────────────────────────────────────────────────────────┘
  echo



# dockcross make
# dockcross cmake -Bbuild -H. -GNinja
# dockcross ninja -Cbuild
# dockcross bash -c '$CC test/C/hello.c -o hello'
# dockcross bash
# 
