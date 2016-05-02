[![Build Status](https://travis-ci.org/reinterpretcat/utymap.svg?branch=master)](https://travis-ci.org/reinterpretcat/utymap)
<h2> Description </h2>

<p>Utymap is a library for building real city environment in 2D/3D dynamically using various map data sources, e.g. OpenStreetMap or Natural Earth.</p>

<h2> Project structure </h2>
Project consists of two sub-projects:
<ul>
    <li><b>core</b> contains essential logic written on C++11 to build library for constructing of map based apps: terrain/buildings/osm-objects mesh generators, mapcss parser, spatial geo index, etc. </li>
    <li><b>unity</b> contains examples written on C# which can be reused to build map oriented Unity apps using core library. It will demonstrate basic use cases: globe zoom level rendering, 3D scene with all details.</li>
</ul>

<h2> Project status </h2>
<p> Project is under development, more details will be given later. </p>

<h2> Install </h2>
Core library has some dependencies. The following versions are used in development environment:
<ul>
    <li> <b>CMake 3.1 or higher.  </b> Some specific flags (CMAKE_CXX_STANDARD, POSITION_INDEPENDENT_CODE) are used. </li>
    <li> <b>Boost 1.55 or higher. </b> Boost is used only for parsing mapcss, xml deserialization and unit testing.  </li>
    <li> <b> Protobuf library and compiler 2.6.1 or other compatible. </b> Protobuf is used for parsing osm pbf files.</li>
    <li> <b> zlib 1.2.8 or other compatible. </b> Used for osm pbf files decompression. </li>
</ul>

<h3>Linux</h3>
If you have some issues, you may check .travis.yml file for details.
<h5> 1. Install dependencies:</h5>

``` bash
#update repositories: you may skip this step
git clone https://github.com/reinterpretcat/utymap.git
sudo -E apt-add-repository -y "ppa:kalakris/cmake"
sudo -E apt-add-repository -y "ppa:george-edison55/precise-backports"
sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test"
sudo -E apt-add-repository -y "ppa:boost-latest/ppa"
sudo -E apt-get -yq update
#install
sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes
install cmake cmake-data zlib1g-dev libprotobuf-dev protobuf-compiler
libboost1.55-all-dev gcc-4.9 g++-4.9
```

<h5>2. Build core library:</h5>

``` bash
# ensure that correct compiler is selected
if [ "$CXX" = "g++" ]; then export CXX="g++-4.9" CC="gcc-4.9"; fi
cd core
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
<h5>3. Copy core library into Unity3d plugin folder: </h5>

``` bash
cp shared/libUtyMapLib.so ../../unity/Assets/Plugins/libUtyMapLib.so

```

<h5>4. Import Unity3d project from unity folder using Unity Editor. </h5>

<h3>Windows</h3>
Visual Studio 2013 is proven for usage on development environment.
<h5>1. Download and build/install dependencies. </h5>
See dependencies list above.
<h5>2. Make sure that Cmake can find dependencies. </h5>
You can register dependencies in your PATH variable or update main CMakeLists.txt, e.g.:

```
set(BOOST_ROOT c:/_libs/boost_1_55_0)
set(BOOST_DIR ${BOOST_ROOT})
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)
set(Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG ${BOOST_LIBRARYDIR}/libboost_unit_test_framework-mgw46-mt-d-1_55.a)
set(Boost_USE_STATIC_LIBS TRUE)

set(PROTOBUF_ROOT  c:/_libs/protobuf-2.6.1)
set(PROTOBUF_INCLUDE_DIR $(PROTOBUF_ROOT)/src)
set(PROTOBUF_LIBRARY $(PROTOBUF_ROOT)/vsprojects/Debug/libprotobuf.lib)
set(PROTOBUF_PROTOC_EXECUTABLE $(PROTOBUF_ROOT)/vsprojects/Debug/protoc.exe)

set(ZLIB_INCLUDE_DIR c:/_libs/zlib-1.2.8)
set(ZLIB_LIBRARY $(ZLIB_INCLUDE_DIR)/zlibd.lib)
```

<h5>3. Create Visual Studio project by make:</h5>

``` shell
cd core
mkdir build
cd build
<path_to_cmake>\cmake.exe -G "Visual Studio 12 Win64" ..
```
<h5>4. Open created solution and build all projects.</h5>

<h5>5. Copy UtyMapLib.dll </h5>

This file is build artefact in build/shared directory and should be copied into unity/Assets/Plugins directory. Ensure that zlib.dll (or zlibd.dll) is also there or in your PATH variable.

<h5>6. Import Unity3d example project. </h5>
Launch Unity Editor and open unity folder as existing project.

<h3>OSX</h3>

Should work as well, but not tested yet.
