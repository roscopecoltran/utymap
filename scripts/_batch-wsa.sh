#!/bin/bash

# # http://www.cmake.org/cmake/help/v3.1/release/3.1.0.html (WindowsPhone, WindowsStore)

CURRENT_PATH=$(pwd)

echo "Not ready yet"
exit 1

# scripts to execute
./scripts/build-wsa-sdk81-phone.sh
./scripts/build-wsa-sdk81.sh
./scripts/buil-wsa-uwp.sh

# list of toolchains:
# ls -l vs* | awk '{print $9}'

# visual studio / wsa toolchains available

#vs-14-2015-arm
#vs-14-2015-sdk-8-1
#vs-14-2015-win64-sdk-8-1
#vs-14-2015-win64
#vs-14-2015

#vs-12-2013-arm
#vs-12-2013-mt
#vs-12-2013-win64
#vs-12-2013-xp
#vs-12-2013

#vs-10-2010
#vs-11-2012-arm
#vs-11-2012-win64
#vs-11-2012

#vs-8-2005
#vs-9-2008