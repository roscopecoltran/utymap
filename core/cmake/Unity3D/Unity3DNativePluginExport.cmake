# Copyright (c) 2017, Luc Michalski
# All rights reserved.

############################################################################################
# Ensure compilation flags settings per platform
############################################################################################

# for windows app store, we need to check the proper architecture
#  - PhoneSDK81: ARM, x86
#  - SDK81: ARM, x86, x64
#  - UWP:  ARM, x86, x64
macro(hunter_unity3d_set_target_properties)

	set(h_append_value SDK_NAME LIBRARY_NAME)
	# parse args
	cmake_parse_arguments(h "" "${h_append_value}" "" ${ARGN})
	if(h_UNPARSED_ARGUMENTS)
		MESSAGE(FATAL_ERROR "unexpected argument: ${h_UNPARSED_ARGUMENTS}")
	endif()
	# check required args no-empty
	if(NOT h_SDK_NAME)
		MESSAGE(FATAL_ERROR "SDK_NAME can't be empty.")
	endif()	
	if(NOT h_LIBRARY_NAME)
		MESSAGE(FATAL_ERROR "LIBRARY_NAME can't be empty.")
	endif()	

	# Win32 
	IF(is_msvc_32)

	  MESSAGE("******* WIN32")
	  SET(TARGET_PLATFORM "Windows")
	  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
	  # shared libraries
	  SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
	  SET(CMAKE_SHARED_LIBRARY_PREFIX ".dll")
	  # static libraries
	  SET(CMAKE_STATIC_LIBRARY_SUFFIX "")
	  SET(CMAKE_STATIC_LIBRARY_PREFIX ".dll")
	  # nb. static .dll(s), does it exist ? :-) need to check that point as I never used windows for development... ^^
	  add_definitions("-D_CRT_SECURE_NO_WARNINGS") #  warning C4996

	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME "${h_SDK_NAME}.dll")
	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH "${HUNTER_UNITY3D_PLUGIN_PREFIX_PATH}/Assets/${h_LIBRARY_NAME}/Plugins/x86/")

	# Win64 
	ELSEIF(is_msvc_64)
	  MESSAGE("******* WIN64")
	  SET(TARGET_PLATFORM "Windows")
	  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
	  # shared libraries
	  SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
	  SET(CMAKE_SHARED_LIBRARY_PREFIX ".dll")
	  # static libraries
	  SET(CMAKE_STATIC_LIBRARY_SUFFIX "")
	  SET(CMAKE_STATIC_LIBRARY_PREFIX ".dll")
	  add_definitions("-D_CRT_SECURE_NO_WARNINGS") #  warning C4996

	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME "${h_SDK_NAME}.dll")
	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH "${HUNTER_UNITY3D_PLUGIN_PREFIX_PATH}/Assets/${h_LIBRARY_NAME}/Plugins/x86_64/")

	# iOS
	ELSEIF(is_ios)
	  MESSAGE("***** iOS")
	  SET(TARGET_PLATFORM "iOS")
	  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -fembed-bitcode")
	  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fembed-bitcode")
	  # only static libraries for iOS
	  SET(BUILD_SHARED_LIBS OFF)
	  SET(CMAKE_STATIC_LIBRARY_SUFFIX "")
	  SET(CMAKE_STATIC_LIBRARY_PREFIX ".a")
	  SET(TARGET_EXTRA_ARG "MACOSX_BUNDLE")
	  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "${CONFIG_BUNDLE_GUI_IDENTIFIER}.\${PRODUCT_NAME:identifier}")

	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME "${h_SDK_NAME}.a")
	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH "${HUNTER_UNITY3D_PLUGIN_PREFIX_PATH}/Assets/${h_LIBRARY_NAME}/Plugins/iOS/")
	  SET(STRIP_ARGS -S -x)

	# Apple - MacOSX
	ELSEIF(is_osx)
	  MESSAGE("***** MacOSX")
	  SET(TARGET_PLATFORM "MacOSX")
	  SET(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
	  # shared libraries
	  SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
	  SET(CMAKE_SHARED_LIBRARY_PREFIX ".bundle")
	  # static libraries
	  SET(CMAKE_STATIC_LIBRARY_SUFFIX "")
	  SET(CMAKE_STATIC_LIBRARY_PREFIX ".a") # instead of .dylib for Unity3D
	  SET(CMAKE_OSX_ARCHITECTURES "x86_64;i386")

	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME "${h_SDK_NAME}")
	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH "${HUNTER_UNITY3D_PLUGIN_PREFIX_PATH}/Assets/${h_LIBRARY_NAME}/Plugins/${h_SDK_NAME}.bundle/Contents/MacOS/")
	  #  
	  MESSAGE(STATUS "CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT}")
	  SET(STRIP_ARGS -S -x)

	# Android
	ELSEIF(is_android)
	  MESSAGE("***** Android")
	  MESSAGE("ndk: ${ANDROID_NDK_VERSION}, api: ${ANDROID_API_VERSION}, abi: ${ANDROID_ABI}")
	  SET(TARGET_PLATFORM "Android")
	  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fvisibility=default -fPIE")
	  # prevent linking against libstdc++
	  SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -nodefaultlibs -lm -lc -ldl -lgcc")
	  SET(CMAKE_CXX_CREATE_SHARED_MODULE  "${CMAKE_CXX_CREATE_SHARED_MODULE} -nodefaultlibs -lm -lc -ldl -lgcc")
	  SET(CMAKE_CXX_LINK_EXECUTABLE       "${CMAKE_CXX_LINK_EXECUTABLE} -nodefaultlibs -lm -lc -ldl -lgcc -rdynamic -fPIE -pie")
	  # shared libraries
	  SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
	  SET(CMAKE_SHARED_LIBRARY_PREFIX ".so")
	  # static libraries
	  SET(CMAKE_STATIC_LIBRARY_SUFFIX "")
	  SET(CMAKE_STATIC_LIBRARY_PREFIX ".a") # do not think that Android NDK deals with static libraries

	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME "lib${h_SDK_NAME}.so")
	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH "${HUNTER_UNITY3D_PLUGIN_PREFIX_PATH}/Assets/${h_LIBRARY_NAME}/Plugins/Android/libs/${ANDROID_ABI}/")


	  SET(STRIP_ARGS --strip-unneeded)

	ELSEIF(is_linux)

	  MESSAGE("***** Linux")
	  SET(TARGET_PLATFORM Linux)

	  # check if variables provided by dockercross toolchain.cmake file and runned from a docker container
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "x86_64"     is_x64)   # dockercross-manylinux-x64
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "i686"       is_x86)   # dockercross-manylinux-x86
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "arm"        is_arm)   # dockercross-linux-armv5, dockercross-linux-armv6, dockercross-linux-armv7
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "aarch64"    is_arm64) # dockercross-linux-arm64
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "ppc64le"    is_ppc64le) # dockercross-linux-ppc64le  
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "mipsel"     is_mipsel)  # dockercross-linux-mipsel

	  # ref: http://www.itgo.me/a/x4974333274279667672/cross-compile-opencv-for-mips-processor  
	  # check Buildroot or Crosstool-NG for mips32el
	  STRING(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "mips"       is_mip) 

	  # set arch prefix for unity3d native plugins output dir
	  # 64bits
	  IF(is_x64)
	    SET(UNITY_PLUGIN_ARCH "x86_64")
	    MESSAGE(STATUS "is_x64 ? ${is_x64}, arch: ${UNITY_PLUGIN_ARCH}")

	  # 32bits
	  ELSEIF(is_x86)
	    SET(UNITY_PLUGIN_ARCH "x86")
	    MESSAGE(STATUS "is_x86 ? ${is_x86}, arch: ${UNITY_PLUGIN_ARCH}")

	  ELSEIF(is_arm)
	    SET(UNITY_PLUGIN_ARCH "arm")
	    MESSAGE(STATUS "is_arm ? ${is_arm}, arch: ${UNITY_PLUGIN_ARCH}")

	  ELSEIF(is_arm64)
	    SET(UNITY_PLUGIN_ARCH "arm64")
	    MESSAGE(STATUS "is_arm64 ? ${is_arm64}, arch: ${UNITY_PLUGIN_ARCH}")

	  ELSEIF(is_ppc64le)
	    SET(UNITY_PLUGIN_ARCH "ppc64le")
	    MESSAGE(STATUS "is_ppc64le ? ${is_ppc64le}, arch: ${UNITY_PLUGIN_ARCH}")

	  ELSEIF(is_mipsel)
	    SET(UNITY_PLUGIN_ARCH "mipsel")
	    MESSAGE(STATUS "is_mipsel ? ${is_mipsel}, arch: ${UNITY_PLUGIN_ARCH}")

	  # Unkown
	  ELSE()
	    MESSAGE(FATAL_ERROR "no architecture/system processor defined in CMake")

	  ENDIF(is_x64)

	  # CMake compilation flags
	  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fvisibility=default -fPIC")
	  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fvisibility=default -fPIC")
	  # prevent linking against libstdc++
	  SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -nodefaultlibs -lm -lc -ldl -lgcc")
	  SET(CMAKE_CXX_CREATE_SHARED_MODULE  "${CMAKE_CXX_CREATE_SHARED_MODULE} -nodefaultlibs -lm -lc -ldl -lgcc")
	  SET(CMAKE_CXX_LINK_EXECUTABLE       "${CMAKE_CXX_LINK_EXECUTABLE} -nodefaultlibs -lm -lc -ldl -lgcc -rdynamic -fPIC -pie")
	  SET(CMAKE_SHARED_LIBRARY_SUFFIX "")
	  SET(CMAKE_SHARED_LIBRARY_PREFIX ".a")
	  # static libraries
	  SET(CMAKE_STATIC_LIBRARY_SUFFIX "")
	  SET(CMAKE_STATIC_LIBRARY_PREFIX ".so") 
	  # nb.
	  # - weird linking shared libraries stuff happens on dockercross/manylinus/centos  
	  # - eg. dlib + PNG through hunter_add_package (requires to add -fPIC flags but present)

	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME "lib${h_SDK_NAME}.so")
	  SET(HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH "${HUNTER_UNITY3D_PLUGIN_PREFIX_PATH}/Assets/${h_LIBRARY_NAME}/Plugins/Linux/${UNITY_PLUGIN_ARCH}/")

	  # prevent linking against libstdc++
	  #SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY} -nodefaultlibs -lm -lc -ldl -lgcc")
	  #SET(CMAKE_CXX_CREATE_SHARED_MODULE  "${CMAKE_CXX_CREATE_SHARED_MODULE} -nodefaultlibs -lm -lc -ldl -lgcc")
	  #SET(CMAKE_CXX_LINK_EXECUTABLE       "${CMAKE_CXX_LINK_EXECUTABLE} -nodefaultlibs -lm -lc -ldl -lgcc -rdynamic -fPIE -pie")
	  #SET(STRIP_ARGS --strip-unneeded)

	# unkown/not detected
	ELSE()

	  SET(CXX11_FLAGS "-std=c++11 -stdlib=libc++")
	  SET(CXX11_LINKER_FLAGS "-stdlib=libc++")

	  # C++11 is required for the `base` and `util` modules provided. You can safely
	  # remove the following lines if you do not intend to use these modules.
	  SET(CMAKE_CXX_FLAGS "${CXX11_FLAGS} ${CMAKE_CXX_FLAGS}")
	  SET(LINKER_FLAGS "${CXX11_LINKER_FLAGS}")
	  SET(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}")
	  SET(CMAKE_SHARED_LINKER_FLAGS "${LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")

	ENDIF()

	hunter_unity3d_summary_target_properties()

endmacro(hunter_unity3d_set_target_properties)

macro(hunter_unity3d_deploy_target)

	set(h_append_value SDK_NAME LIBRARY_NAME)
	# parse args
	cmake_parse_arguments(h "" "${h_append_value}" "" ${ARGN})
	if(h_UNPARSED_ARGUMENTS)
		MESSAGE(FATAL_ERROR "unexpected argument: ${h_UNPARSED_ARGUMENTS}")
	endif()
	# check required args no-empty
	if(NOT h_SDK_NAME)
		MESSAGE(FATAL_ERROR "SDK_NAME can't be empty.")
	endif()	
	if(NOT h_LIBRARY_NAME)
		MESSAGE(FATAL_ERROR "LIBRARY_NAME can't be empty.")
	endif()	

	if(is_osx)
	    if(${LIBRARY_NAME}_ENABLE_OSX_BUNDLE)
	        SET_TARGET_PROPERTIES(${LIBRARY_NAME} ${PROTOBUF_LIBRARY} ${ZLIB_LIBRARY} PROPERTIES BUNDLE ON)
	        #SET_TARGET_PROPERTIES(usdi usdiRT PROPERTIES BUNDLE ON)
	    else()
	        SET_TARGET_PROPERTIES(${LIBRARY_NAME} ${PROTOBUF_LIBRARY} ${ZLIB_LIBRARY} PROPERTIES PREFIX "")
	        SET_TARGET_PROPERTIES(${LIBRARY_NAME} ${PROTOBUF_LIBRARY} ${ZLIB_LIBRARY} PROPERTIES SUFFIX ".bundle")
	    endif()
	endif(is_osx)


endmacro(hunter_unity3d_deploy_target)	

macro(hunter_unity3d_summary_target_properties)

	message( STATUS "|   																					   ")
	message( STATUS "|-- Polly variables:")
	message( STATUS "|   |_____________________________________________________________________________________")
	message( STATUS "|   *** POLLY_TOOLCHAIN_NAME: ${POLLY_TOOLCHAIN_NAME}" )
	message( STATUS "|   *** POLLY_TOOLCHAIN_TAG: ${POLLY_TOOLCHAIN_TAG}" )
	message( STATUS "|   *** POLLY_STATUS_PRINT: ${POLLY_STATUS_PRINT}" )
	message( STATUS "|   *** POLLY_STATUS_DEBUG: ${POLLY_STATUS_DEBUG}" )
	message( STATUS "|   																					   ")
	message( STATUS "|-- CMake flags:")
	message( STATUS "|   |_____________________________________________________________________________________")
	message( STATUS "|   *** CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}" )
	message( STATUS "|   *** CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}" )
	message( STATUS "|   *** CMAKE_CXX_CREATE_SHARED_LIBRARY: ${CMAKE_CXX_CREATE_SHARED_LIBRARY}" )
	message( STATUS "|   *** CMAKE_CXX_CREATE_SHARED_MODULE: ${CMAKE_CXX_CREATE_SHARED_MODULE}" )
	message( STATUS "|   *** CMAKE_CXX_LINK_EXECUTABLE: ${CMAKE_CXX_LINK_EXECUTABLE}" )
	message( STATUS "|   *** CMAKE_SHARED_LIBRARY_SUFFIX: ${CMAKE_SHARED_LIBRARY_SUFFIX}" )
	message( STATUS "|   *** CMAKE_SHARED_LIBRARY_PREFIX: ${CMAKE_SHARED_LIBRARY_PREFIX}" )
	message( STATUS "|   *** CMAKE_STATIC_LIBRARY_SUFFIX: ${CMAKE_STATIC_LIBRARY_SUFFIX}" )
	message( STATUS "|   *** CMAKE_STATIC_LIBRARY_PREFIX: ${CMAKE_STATIC_LIBRARY_PREFIX}" )
	message( STATUS "|   *** CMAKE_CXX_CREATE_SHARED_MODULE: ${CMAKE_CXX_CREATE_SHARED_MODULE}" )
	message( STATUS "|   																					   ")
	message( STATUS "|-- Unity3D export variables:")
	message( STATUS "|   |_____________________________________________________________________________________")
	message( STATUS "|   *** HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME: ${HUNTER_UNITY3D_PLUGIN_LIBRARY_FILENAME}" )
	message( STATUS "|   *** HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH: ${HUNTER_UNITY3D_PLUGIN_LIBRARY_OUTPUT_PATH}" )
	message( STATUS "|   																					   ")
	#message(FATAL_ERROR "")

endmacro(hunter_unity3d_summary_target_properties)


