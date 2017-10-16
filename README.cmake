Building using CMake
********************
Commands to build using CMake:

--------------------------------------------------------------------------------

mkdir build
cd build

cmake .. && make

Cross compiling using CMake
***************************

To cross compile for Windows using MinGW, define the CMAKE_TOOLCHAIN_FILE CMake
configuration variable to point at the toolchain-mingw32-w64.cmake:

--------------------------------------------------------------------------------

# In the build directory
cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../toolchain-mingw32-w64.cmake .. && make


Installing library using CMake
******************************

To install toolkit library and header files using CMake, you should provide
the path in which lib/*.a and include/*.h directories will be installed:

--------------------------------------------------------------------------------
# In the build directory
cmake -DCMAKE_INSTALL_PREFIX=<INSTALL_PATH> ..
cmake --build . --target install