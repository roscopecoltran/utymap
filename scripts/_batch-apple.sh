#!/bin/bash

CURRENT_PATH=$(pwd)

./scripts/build-osx-universal.sh
./scripts/build-ios-universal.sh
