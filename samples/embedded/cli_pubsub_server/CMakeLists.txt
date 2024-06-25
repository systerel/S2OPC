cmake_minimum_required (VERSION 3.5)

# Demo CLI-PubSub-Server
set(SAMPLE_PROJECT_NAME S2OPC_CLI_PubSub_Server)
SET(SAMPLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(../platform_dep/CMake/CMakeLists.txt)

project (${SAMPLE_PROJECT_NAME} LANGUAGES C)

if(POLICY CMP0148)
  cmake_policy(SET CMP0148 OLD)
endif()
find_package(PythonInterp 3 REQUIRED)

######################################################
# Generate test_address_space.c from s2opc_pubsub_embedded_nodeset.xml
set(c_aspace_file_name ${SAMPLE_SRC_DIR}/src/test_address_space.c)
set(xml_uanodeset_path ${SAMPLE_SRC_DIR}/xml/s2opc_pubsub_embedded_nodeset.xml)

if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set_source_files_properties(${c_aspace_file_name} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
endif()
