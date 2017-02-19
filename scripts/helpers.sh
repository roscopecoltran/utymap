#!/bin/bash

# Copyright (c) 2017, Luc Michalski
# All rights reserved.

# ref: https://github.com/NeoGeographyToolkit/BinaryBuilder/tree/3215a60536b8c814a104c292e643b0c81fc7a16f
# auto_build: https://github.com/NeoGeographyToolkit/BinaryBuilder/tree/master/auto_build
# polly extension: https://github.com/NeoGeographyToolkit/BinaryBuilder/blob/master/build.py

hunter_needed_libs() {
    file ${1:-.}/* | grep ELF | cut -d : -f 1 | xargs readelf -d | grep NEEDED | sed -e 's/.*\[//g' -e 's/\].*//g' | sort -u
}

hunter_found_libs() {
    ldd ${1:-.}/*
}

hunter_filter_found() {
    LC_ALL=C fgrep --color=auto -f ${1:?[expected needed lib file]} ${2:?[expected found lib file]} | sed -e 's/(0x00.*//g' | sort -u | less
}

