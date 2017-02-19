#!/bin/sh

CURRENT_PATH=$(pwd)
export BUILD_PREFIX_PATH=./_builds

function lookup_file
{
	#pwd
	#echo "find "${BUILD_PREFIX_PATH}/$1"* $2 -type f"

	#echo "${BUILD_PREFIX_PATH}/${1}*/${2}"

	for file in `find "${BUILD_PREFIX_PATH}" -name "osx*.dylib"`; do echo "$file"; done

}


#lookup_file2 
#exit 1


lookup_file "osx" "*.dylib"
#lookup_file "ios" "*.a"
#lookup_file "libcxx" "*.so"
#lookup_file "android" "*.so"
#lookup_file "vs" "*.dll"