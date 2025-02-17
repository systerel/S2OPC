cmake_minimum_required(VERSION 3.5)
project(S2OPC_ClientServer LANGUAGES C)

##                                    ##
# S2OPC client/server toolkit library  #
##                                    ##

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

### Dependencies ###

# EXPAT dependency managed in CommonDefs

### Compilation of S2OPC client/server library source files ###

## Build info source file generation (version, commit checksum, docker id, date) ##
# Define how to generate build info file
set(S2OPC_TOOLKIT_BUILD_INFO_C ${CMAKE_CURRENT_BINARY_DIR}/sopc_toolkit_build_info.c)
s2opc_gen_build_info(${CMAKE_CURRENT_SOURCE_DIR}/configuration/sopc_toolkit_build_info.c_
                     ${S2OPC_TOOLKIT_BUILD_INFO_C})

## C99 compliant source code ##

# Define non-platform dependent source files
file(GLOB_RECURSE S2OPC_CLIENTSERVER_SRCS
    "address_space/*.c"
    "api_toolkit/*.c"
    "configuration/*.c"
    "sockets/*.c"
    "secure_channels/*.c"
    "services/*.c"
)

# Define S2OPC library includes

set(S2OPC_CLIENTSERVER_PUBLIC_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/address_space"
    "${CMAKE_CURRENT_SOURCE_DIR}/api_toolkit"
    "${CMAKE_CURRENT_SOURCE_DIR}/configuration"
)

set(S2OPC_CLIENTSERVER_PRIVATE_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/address_space/internal"
    "${CMAKE_CURRENT_SOURCE_DIR}/sockets"
    "${CMAKE_CURRENT_SOURCE_DIR}/secure_channels"
    "${CMAKE_CURRENT_SOURCE_DIR}/services"
    "${CMAKE_CURRENT_SOURCE_DIR}/services/bgenc"
    "${CMAKE_CURRENT_SOURCE_DIR}/services/b2c"
)

# define S2OPC wrappers source files

set(XML_LOADERS_PATH "loaders")
set(COMMONWRAPPER_PATH "frontend/common_wrapper")
set(HELPERWRAPPER_PATH "frontend/helper_wrapper")
set(CLIENTWRAPPER_PATH "frontend/client_wrapper")
set(SERVERWRAPPER_PATH "frontend/server_wrapper")

set(WRAPPERS_XML_CONFIG "")
set(LOADER_XML_PRIVATE_DEP "")
if (expat_FOUND)
  set(WRAPPERS_XML_CONFIG
      "${XML_LOADERS_PATH}/helpers/sopc_helper_expat.c"
      "${XML_LOADERS_PATH}/address_space_loaders/xml_expat/sopc_uanodeset_loader.c"
      "${XML_LOADERS_PATH}/config_loaders/xml_expat/sopc_config_loader_internal.c"
      "${XML_LOADERS_PATH}/config_loaders/xml_expat/sopc_config_server_loader.c"
      "${XML_LOADERS_PATH}/config_loaders/xml_expat/sopc_config_client_loader.c"
      "${XML_LOADERS_PATH}/config_loaders/xml_expat/sopc_users_loader.c"
      "${CLIENTWRAPPER_PATH}/libs2opc_client_config_xml.c"
      "${SERVERWRAPPER_PATH}/libs2opc_server_config_xml.c")
  set(LOADER_XML_PRIVATE_DEP expat::expat)
  else()
  message("-- Expat library NOT found: XML config loaders will not be available")
  message(WARNING "Without the Expat library, the XML config loaders will not be compiled.")
endif()

set(S2OPC_WRAPPERS_SRC
    "${COMMONWRAPPER_PATH}/libs2opc_common_config.c"
    "${COMMONWRAPPER_PATH}/libs2opc_common_monitoring.c"
    "${HELPERWRAPPER_PATH}/libs2opc_request_builder.c"
    "${CLIENTWRAPPER_PATH}/libs2opc_client.c"
    "${CLIENTWRAPPER_PATH}/libs2opc_client_config.c"
    "${CLIENTWRAPPER_PATH}/libs2opc_client_config_custom.c"
    "${CLIENTWRAPPER_PATH}/internal/state_machine.c"
    "${SERVERWRAPPER_PATH}/libs2opc_server.c"
    "${SERVERWRAPPER_PATH}/libs2opc_server_config.c"
    "${SERVERWRAPPER_PATH}/libs2opc_server_config_custom.c"
    "${SERVERWRAPPER_PATH}/internal/libs2opc_server_runtime_variables.c"
    "${SERVERWRAPPER_PATH}/file_transfer/libs2opc_file_transfer.c"
    ${WRAPPERS_XML_CONFIG}
    )

list(APPEND WRAPPERS_INCLUDE_FILES
    "${COMMONWRAPPER_PATH}/libs2opc_common_config.h"
    "${HELPERWRAPPER_PATH}/libs2opc_request_builder.h"
    "${CLIENTWRAPPER_PATH}/libs2opc_client.h"
    "${CLIENTWRAPPER_PATH}/libs2opc_client_config.h"
    "${CLIENTWRAPPER_PATH}/libs2opc_client_config_custom.h"
    "${SERVERWRAPPER_PATH}/libs2opc_server.h"
    "${SERVERWRAPPER_PATH}/libs2opc_server_config.h"
    "${SERVERWRAPPER_PATH}/libs2opc_server_config_custom.h"
    "${SERVERWRAPPER_PATH}/file_transfer/libs2opc_file_transfer.h")

set(S2OPC_XML_LOADERS_PUBLIC_INCLUDES "")
set(S2OPC_XML_LOADERS_PRIVATE_INCLUDES "")
if(expat_FOUND)
  list(APPEND WRAPPERS_INCLUDE_FILES
       "${XML_LOADERS_PATH}/address_space_loaders/xml_expat/sopc_uanodeset_loader.h"
       "${XML_LOADERS_PATH}/config_loaders/xml_expat/sopc_config_loader.h"
       "${XML_LOADERS_PATH}/config_loaders/xml_expat/sopc_users_loader.h")
  set(S2OPC_XML_LOADERS_PUBLIC_INCLUDES
      "${CMAKE_CURRENT_SOURCE_DIR}/loaders/address_space_loaders"
      "${CMAKE_CURRENT_SOURCE_DIR}/loaders/config_loaders")
  set(S2OPC_XML_LOADERS_PRIVATE_INCLUDES
      "${CMAKE_CURRENT_SOURCE_DIR}/loaders/helpers")
endif()


### Configure S2OPC client/server library and its properties ###

add_library(s2opc_clientserver
            ${S2OPC_CLIENTSERVER_SRCS}
            ${S2OPC_TOOLKIT_BUILD_INFO_C}
            ${S2OPC_WRAPPERS_SRC})
target_compile_options(s2opc_clientserver PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_clientserver PRIVATE ${S2OPC_DEFINITIONS} "UANODESET_LOADER_LOG")
target_link_libraries(s2opc_clientserver PUBLIC s2opc_common PRIVATE ${LOADER_XML_PRIVATE_DEP})

target_include_directories(s2opc_clientserver
  PUBLIC
  "$<BUILD_INTERFACE:${S2OPC_CLIENTSERVER_PUBLIC_INCLUDES}>" # when building (quotes needed to interpret list correclty)
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${COMMONWRAPPER_PATH};${CMAKE_CURRENT_SOURCE_DIR}/${HELPERWRAPPER_PATH}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${CLIENTWRAPPER_PATH}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${SERVERWRAPPER_PATH}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${SERVERWRAPPER_PATH}/file_transfer>"
  "$<BUILD_INTERFACE:${S2OPC_XML_LOADERS_PUBLIC_INCLUDES}>"
  $<INSTALL_INTERFACE:include/s2opc/clientserver> # when installed
  PRIVATE
  ${S2OPC_CLIENTSERVER_PRIVATE_INCLUDES}
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${COMMONWRAPPER_PATH}/internal;${CMAKE_CURRENT_SOURCE_DIR}/${CLIENTWRAPPER_PATH}/internal;${CMAKE_CURRENT_SOURCE_DIR}/${SERVERWRAPPER_PATH}/internal>"
  "$<BUILD_INTERFACE:${S2OPC_XML_LOADERS_PRIVATE_INCLUDES}>")

### Installation and export of S2OPC client/server toolkit library ###
foreach(dir ${S2OPC_CLIENTSERVER_PUBLIC_INCLUDES})
    file(GLOB temp_files "${dir}/*.h")
    list(APPEND S2OPC_CLIENTSERVER_INCLUDE_FILES ${temp_files})
endforeach()
list(APPEND S2OPC_CLIENTSERVER_INCLUDE_FILES "${WRAPPERS_INCLUDE_FILES}")

# set public header property
set_target_properties(s2opc_clientserver PROPERTIES PUBLIC_HEADER
                      "${S2OPC_CLIENTSERVER_INCLUDE_FILES}")
# install files
install(TARGETS s2opc_clientserver
        EXPORT s2opc_clientserver-export # export our project to be importable after install
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/s2opc/clientserver)

# install the export as a cmake file
install(EXPORT s2opc_clientserver-export
  FILE s2opc_clientserver-export.cmake
  DESTINATION lib/cmake/s2opc)

# export cmake file for use in build tree without install
export(EXPORT s2opc_clientserver-export
  FILE "${CMAKE_BINARY_DIR}/s2opc_clientserver-export.cmake"
  )

##                                        ##
# S2OPC client/server extension libraries  #
##                                        ##

### Compilation and configuration of S2OPC client/server extension libraries ###

## Configuration loaders backends (OPC UA address space, server configuration) ##

# C generated structure address space loader
# (see s2opc_embed_address_space in CommonDefs.cmake to generate the C structure from XML)
# Note: this library is never compiled as a dynamic library due to the use of extern variables not defined during library linking
add_library(s2opc_clientserver-loader-embedded STATIC loaders/address_space_loaders/embedded/sopc_addspace_loader.c)
target_compile_options(s2opc_clientserver-loader-embedded PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_clientserver-loader-embedded PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(s2opc_clientserver-loader-embedded PUBLIC s2opc_clientserver)
target_include_directories(s2opc_clientserver-loader-embedded
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/loaders/address_space_loaders>
  $<INSTALL_INTERFACE:include/s2opc/clientserver> # when installed
  )
set_target_properties(s2opc_clientserver-loader-embedded PROPERTIES PUBLIC_HEADER "loaders/address_space_loaders/embedded/sopc_addspace_loader.h")
install(TARGETS s2opc_clientserver-loader-embedded
        EXPORT s2opc_clientserver-loader-embedded-export # export our project to be importable after install
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/s2opc/clientserver/embedded)

# install the export as a cmake file
install(EXPORT s2opc_clientserver-loader-embedded-export
  FILE s2opc_clientserver-loader-embedded-export.cmake
  DESTINATION lib/cmake/s2opc)

# export cmake file for use in build tree without install
export(EXPORT s2opc_clientserver-loader-embedded-export
  FILE "${CMAKE_BINARY_DIR}/s2opc_clientserver-loader-embedded-export.cmake"
  )

# Add the PyS2OPC folder lately, as it requires the above library
add_subdirectory(frontend/pys2opc)
