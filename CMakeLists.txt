cmake_minimum_required(VERSION 3.12)
project(S2OPC_Sources_Tests LANGUAGES C)

# include common definitions here to share it with sources and tests
if(NOT S2OPC_COMMON_DEFS_SET)
  include(CommonDefs.cmake)
endif()

# Build S2OPC libraries, S2OPC libraries extensions and S2OPC tests

# overwrite find_package to inhibit find_package(s2opc) in samples when built here
macro(find_package)
  if("${ARGV0}" STREQUAL "s2opc")
    # ignore find_package since we build s2opc at same time
  else()
    _find_package(${ARGV})
  endif()
endmacro()

# S2OPC code documentation #

find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E env DOXYGEN_SOURCE_PREFIX=${CMAKE_SOURCE_DIR}/ ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxygen/s2opc-toolkit.doxyfile
        COMMENT "Generating API documentation"
        VERBATIM
    )
endif()

# S2OPC #

# Options
option(ENABLE_SAMPLES "Build the S2OPC library samples" ON)
option(ENABLE_TESTING "Build the S2OPC library tests" ON)

add_subdirectory(src)

if(ENABLE_TESTING)
 add_subdirectory(tests)
endif()
if(ENABLE_SAMPLES)
  add_subdirectory(samples)
endif()
