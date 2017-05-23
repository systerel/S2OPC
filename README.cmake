Building using CMake
********************
Commands to build using CMake:

--------------------------------------------------------------------------------

mkdir build
cd build

cmake .. && make

# or, to build using Ninja

cmake -G Ninja .. && ninja

--------------------------------------------------------------------------------

This picks the default options for the stack version and wrapper reception
thread. To configure the build, one can either:

1. Pass the variables using `cmake -DFOO=BAR` (the list of variables can be
   printed using `cmake -L ..`).
2. Use the ncurses configuration UI ccmake: `ccmake ..` (you need to then press
   'c' to run the configuration, set the variables, and press 'g' to generate
   the Makefile/Ninja file).
3. Use the Qt configuration UI cmake-gui (similar to ccmake but prettier).

The `-G` option can be passed to all configuration tools the same way as for
`cmake` to specify the generator to use.

Cross compiling using CMake
***************************

To cross compile for Windows using MinGW, define the CMAKE_TOOLCHAIN_FILE CMake
configuration variable to point at the toolchain-mingw32-w64.cmake, for example:

--------------------------------------------------------------------------------

# In the build directory
cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../toolchain-mingw32-w64.cmake .. && make

--------------------------------------------------------------------------------
