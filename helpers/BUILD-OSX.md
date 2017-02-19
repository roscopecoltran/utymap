Tested on 10.11.6

## Build with Polly+Hunter

## 1 - Requirements:
- Git
- CMake 3.7 

```
$ brew install cmake git
```

## 2 - Clone/Fork the repository
```
$ git clone https://github.com/reinterpretcat/utymap.git
$ cd utymap
```

## 3 - Build
With bash script:
```
$ ./script/build-build-osx-universal.sh
```
Nb. You can or set different variables for all the available toolchains in [Polly](https://github.com/ruslo/polly).

or directly with [Polly](https://github.com/ruslo/polly):
```
$  build.py --toolchain=${TOOLCHAIN} \
            --verbosity-level full \
            --config ${BUILD_TYPE} \
            --reconfig \
            --clear \
            --install \
            --home ./core \
            --framework \
    	    --universal-apple \
            --test \
            --fwd CMAKE_XCODE_ATTRIBUTE_OSX_DEPLOYMENT_TARGET=10.10 \
                  CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
                  BUILD_SHARED_LIBS=OFF \
                  CMAKE_OSX_DEPLOYMENT_TARGET=10.10 \
    	          HUNTER_CONFIGURATION_TYPES=Release \
    	          HUNTER_UNITY3D_PLUGIN_PREFIX_PATH=../../unity/demo/Assets/Plugins/ \
             --tail ${LOG_TAIL} \
             --jobs ${WORKERS}

```


# in ./core/build use xcode's command line to build the dylib
xcodebuild -project UtyMap.xcodeproj -target ALL_BUILD -configuration Release

# change the libUtyMap.Shared.dylib  to UtyMap.Shared.bundle and more to the project folder
cp shared/Release/libUtyMap.Shared.dylib ../../unity/demo/Assets/Plugins/UtyMap.Shared.bundle

# switch te the unity/library directory
cd ../../unity/library/

# build with mono's xbuild
xbuild /p:Configuration=Release UtyMap.Unity.sln

# copy all the Uty dll files
cp UtyMap.Unity/bin/Release/Uty*.dll ../../unity/demo/Assets/Plugins/

