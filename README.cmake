Building and installing using CMake
===================================

Build library and binaries
__________________________

```bash
mkdir build
cd build

cmake <options> .. && make
```

Note1: it will build the library as static by default and the tests binaries statically linked
Note2: binaries are created into bin/ directory at same level than build/ directory

Build library and binaries with project shared library
______________________________________________________

In the build directory:
```bash
cmake -DBUILD_SHARED_LIBS=true && make
```

Cross compiling for Windows
___________________________

To cross compile for Windows using MinGW-W64, define the CMAKE_TOOLCHAIN_FILE CMake configuration variable to point at the toolchain-mingw32-w64.cmake (in the build directory):

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../toolchain-mingw32-w64.cmake .. && make
```

Note1: combine it with -DBUILD_SHARED_LIBS=true to build a DLL for windows
Note2: mbedtls and libcheck should be installed into the cross compiler paths using -DCMAKE_INSTALL_PREFIX=/usr/i686-w64-mingw32 option when building and installing mbetld and libcheck


Installing library using CMake
______________________________

To install toolkit library and header files using CMake, you should provide the path in which lib/*.a and include/*.h directories will be installed (in the build directory):

```bash
cmake -DCMAKE_INSTALL_PREFIX=<INSTALL_PATH> ..
cmake --build . --target install
```
