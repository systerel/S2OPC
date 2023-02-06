cmake_minimum_required (VERSION 3.5)

# Demo CLI-Client
set(SAMPLE_PROJECT_NAME S2OPC_CLI_Client)
SET(SAMPLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(../platform_dep/CMake/CMakeLists.txt)

project (${SAMPLE_PROJECT_NAME} LANGUAGES C)
