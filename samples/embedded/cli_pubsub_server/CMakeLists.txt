cmake_minimum_required (VERSION 3.5)

# Demo CLI-PubSub-Server
set(SAMPLE_PROJECT_NAME S2OPC_CLI_PubSub_Server)
SET(SAMPLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(../platform_dep/CMake/CMakeLists.txt)

project (S2OPC_CLI_PubSub_Server LANGUAGES C)

# Project specific steps

# TODO : test_address_space.c must be generated
# set_source_files_properties(${SAMPLE_SRC_DIR}/src/test_address_space.c PROPERTIES GENERATED TRUE)
if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  set_source_files_properties(${SAMPLE_SRC_DIR}/src/test_address_space.c PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
endif()
