#!/bin/bash

CURRENT_PATH=$(pwd)

# apple
./scripts/build-osx-universal.sh
./scripts/build-ios-universal.sh
./scripts/build-libcxx.sh
./scripts/build-android.sh

# gcc
#./scripts/build-gcc.sh

# libcxx
#./scripts/build-libcxx.sh

# android
#./scripts/build-android.sh

# windows
#./scripts/build-wsa.sh

# linux
#./scripts/build-linux.sh