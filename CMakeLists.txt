cmake_minimum_required(VERSION 3.5)
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

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(samples)
