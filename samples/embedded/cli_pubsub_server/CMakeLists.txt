cmake_minimum_required (VERSION 3.5)

# Demo CLI-PubSub-Server
set(SAMPLE_PROJECT_NAME S2OPC_CLI_PubSub_Server)
SET(SAMPLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(../platform_dep/CMake/CMakeLists.txt)

project (S2OPC_CLI_PubSub_Server LANGUAGES C)

find_package(PythonInterp 3 REQUIRED)

######################################################
# Generate test_address_space.c from pubsub_server.xml
set(S2OPC_ROOT_PATH ${SAMPLE_SRC_DIR}/../../..)
set(c_aspace_file_name ${SAMPLE_SRC_DIR}/src/test_address_space.c)
set(xml_uanodeset_path ${SAMPLE_SRC_DIR}/xml/pubsub_server.xml)

set(const_addspace "--const_addspace")
add_custom_command(
    OUTPUT ${c_aspace_file_name}
    DEPENDS ${xml_uanodeset_path}
    COMMAND ${PYTHON_EXECUTABLE} ${S2OPC_ROOT_PATH}/scripts/generate-s2opc-address-space.py ${xml_uanodeset_path} ${c_aspace_file_name} ${const_addspace}
    COMMENT "Generating address space ${c_aspace_file_name}"
    VERBATIM
    )

set_source_files_properties(${c_aspace_file_name} PROPERTIES GENERATED TRUE)
if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set_source_files_properties(${c_aspace_file_name} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
endif()

