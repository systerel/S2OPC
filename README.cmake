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

--------------------------------------------------------------------------------
