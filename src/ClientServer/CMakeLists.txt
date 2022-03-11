cmake_minimum_required(VERSION 3.5)
project(S2OPC_ClientServer LANGUAGES C)

option(WITH_NANO_EXTENDED "Use Nano profile with additional services out of Nano scope" OFF)

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
    "api_toolkit/*.c"
    "configuration/*.c"
    "sockets/*.c"
    "secure_channels/*.c"
    "services/*.c"
)

# Define S2OPC library includes

set(S2OPC_CLIENTSERVER_PUBLIC_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/api_toolkit"
    "${CMAKE_CURRENT_SOURCE_DIR}/configuration"
    "${CMAKE_CURRENT_SOURCE_DIR}/services/b2c"
)

set(S2OPC_CLIENTSERVER_PRIVATE_INCLUDES
    "sockets"
    "secure_channels"
    "services"
    "services/bgenc"
)

### Configure S2OPC client/server library and its properties ###

add_library(s2opc_clientserver ${S2OPC_CLIENTSERVER_SRCS} ${S2OPC_TOOLKIT_BUILD_INFO_C})
target_compile_options(s2opc_clientserver PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_clientserver PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(s2opc_clientserver PUBLIC s2opc_common)

target_include_directories(s2opc_clientserver
  PUBLIC
  "$<BUILD_INTERFACE:${S2OPC_CLIENTSERVER_PUBLIC_INCLUDES}>" # when building (quotes needed to interpret list correclty)
  $<INSTALL_INTERFACE:include/s2opc/clientserver> # when installed
  PRIVATE ${S2OPC_CLIENTSERVER_PRIVATE_INCLUDES})

### Installation and export of S2OPC client/server toolkit library ###
foreach(dir ${S2OPC_CLIENTSERVER_PUBLIC_INCLUDES})
    file(GLOB temp_files "${dir}/*.h")
    list(APPEND S2OPC_CLIENTSERVER_INCLUDE_FILES ${temp_files})
endforeach()
# set public header property
set_target_properties(s2opc_clientserver PROPERTIES PUBLIC_HEADER "${S2OPC_CLIENTSERVER_INCLUDE_FILES}")
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

# XML addresss space loader, XML server configuration loader and XML server users configuration loader
if (expat_FOUND)
  message(STATUS "Expat library found: XML server config loaders compilation active")
  add_library(s2opc_clientserver-xml-loaders-expat loaders/helpers/sopc_helper_expat.c
                                             loaders/address_space_loaders/xml_expat/sopc_uanodeset_loader.c
                                             loaders/config_loaders/xml_expat/sopc_config_loader.c
                                             loaders/config_loaders/xml_expat/sopc_users_loader.c)
  target_compile_options(s2opc_clientserver-xml-loaders-expat PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_clientserver-xml-loaders-expat PRIVATE ${S2OPC_DEFINITIONS} "UANODESET_LOADER_LOG")
  target_link_libraries(s2opc_clientserver-xml-loaders-expat PUBLIC s2opc_clientserver PRIVATE expat::expat)
  set(S2OPC_XML_LOADERS_PUBLIC_INCLUDES
      "${CMAKE_CURRENT_SOURCE_DIR}/loaders/address_space_loaders"
      "${CMAKE_CURRENT_SOURCE_DIR}/loaders/config_loaders"
      )
  target_include_directories(s2opc_clientserver-xml-loaders-expat
                             PUBLIC
                             "$<BUILD_INTERFACE:${S2OPC_XML_LOADERS_PUBLIC_INCLUDES}>"
                             $<INSTALL_INTERFACE:include/s2opc/clientserver> # when installed
                             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/loaders/helpers)
  set_property(TARGET s2opc_clientserver-xml-loaders-expat PROPERTY
               PUBLIC_HEADER "loaders/address_space_loaders/xml_expat/sopc_uanodeset_loader.h"
                             "loaders/config_loaders/xml_expat/sopc_config_loader.h"
                             "loaders/config_loaders/xml_expat/sopc_users_loader.h")
  install(TARGETS s2opc_clientserver-xml-loaders-expat
          EXPORT s2opc_clientserver-xml-loaders-expat-export # export our project to be importable after install
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/s2opc/clientserver/xml_expat)

  # install the export as a cmake file
  install(EXPORT s2opc_clientserver-xml-loaders-expat-export
          FILE s2opc_clientserver-xml-loaders-expat-export.cmake
          DESTINATION lib/cmake/s2opc)

  # export cmake file for use in build tree without install
  export(EXPORT s2opc_clientserver-xml-loaders-expat-export
         FILE "${CMAKE_BINARY_DIR}/s2opc_clientserver-xml-loaders-expat-export.cmake"
         )
else()
  message("-- Expat library NOT found: XML config loaders will not be available")
endif()

## Common wrapper library ##
set(COMMONWRAPPER_PATH "frontend/common_wrapper")
set(HELPERWRAPPER_PATH "frontend/helper_wrapper")
# Set empty implementation and dependency if Expat not available

add_library(s2opc_commonwrapper "${COMMONWRAPPER_PATH}/libs2opc_common_config.c"
                                "${HELPERWRAPPER_PATH}/libs2opc_request_builder.c"
                                )
target_link_libraries(s2opc_commonwrapper PUBLIC s2opc_clientserver)
target_compile_options(s2opc_commonwrapper PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_commonwrapper PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(s2opc_commonwrapper
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${COMMONWRAPPER_PATH};${CMAKE_CURRENT_SOURCE_DIR}/${HELPERWRAPPER_PATH}>"
  $<INSTALL_INTERFACE:include/s2opc/clientserver/frontend> # when installed
  )
list(APPEND COMMON_WRAPPER_INCLUDE_FILES "${COMMONWRAPPER_PATH}/libs2opc_common_config.h"
                                         "${HELPERWRAPPER_PATH}/libs2opc_request_builder.h")
set_target_properties(s2opc_commonwrapper PROPERTIES PUBLIC_HEADER "${COMMON_WRAPPER_INCLUDE_FILES}")

install(TARGETS s2opc_commonwrapper
        EXPORT s2opc_clientserver-commonwrapper-export # export our project to be importable after install
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/s2opc/clientserver/frontend)

# install the export as a cmake file
install(EXPORT s2opc_clientserver-commonwrapper-export
        FILE s2opc_clientserver-commonwrapper-export.cmake
        DESTINATION lib/cmake/s2opc)

# export cmake file for use in build tree without install
export(EXPORT s2opc_clientserver-commonwrapper-export
       FILE "${CMAKE_BINARY_DIR}/s2opc_clientserver-commonwrapper-export.cmake")


## Client wrapper library ##
set(LIBSUBSCRIBE_PATH "frontend/client_wrapper")
add_library(s2opc_clientwrapper "${LIBSUBSCRIBE_PATH}/libs2opc_client.c"
                                "${LIBSUBSCRIBE_PATH}/libs2opc_client_cmds.c"
                                "${LIBSUBSCRIBE_PATH}/libs2opc_client_common.c"
                                "${LIBSUBSCRIBE_PATH}/toolkit_helpers.c"
                                "${LIBSUBSCRIBE_PATH}/state_machine.c"
                                "${LIBSUBSCRIBE_PATH}/pki_permissive.c"
                                )
target_link_libraries(s2opc_clientwrapper PUBLIC s2opc_commonwrapper)
target_compile_options(s2opc_clientwrapper PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_clientwrapper PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(s2opc_clientwrapper
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${LIBSUBSCRIBE_PATH}>"
  $<INSTALL_INTERFACE:include/s2opc/clientserver/frontend> # when installed
  )

list(APPEND CLIENT_WRAPPER_INCLUDE_FILES "${LIBSUBSCRIBE_PATH}/libs2opc_client.h"
                                         "${LIBSUBSCRIBE_PATH}/libs2opc_client_cmds.h"
                                         "${LIBSUBSCRIBE_PATH}/libs2opc_client_common.h")
set_target_properties(s2opc_clientwrapper PROPERTIES PUBLIC_HEADER "${CLIENT_WRAPPER_INCLUDE_FILES}")

install(TARGETS s2opc_clientwrapper
        EXPORT s2opc_clientserver-clientwrapper-export # export our project to be importable after install
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/s2opc/clientserver/frontend)

# install the export as a cmake file
install(EXPORT s2opc_clientserver-clientwrapper-export
        FILE s2opc_clientserver-clientwrapper-export.cmake
        DESTINATION lib/cmake/s2opc)

# export cmake file for use in build tree without install
export(EXPORT s2opc_clientserver-clientwrapper-export
       FILE "${CMAKE_BINARY_DIR}/s2opc_clientserver-clientwrapper-export.cmake")

## Server wrapper library ##
set(SERVERWRAPPER_PATH "frontend/server_wrapper")
# Set empty implementation and dependency if Expat not available
set(SERVERWRAPPER_XML_CONFIG "")
set(SERVERWRAPPER_XML_DEP "")
if (expat_FOUND)
  set(SERVERWRAPPER_XML_CONFIG "${SERVERWRAPPER_PATH}/libs2opc_server_config_xml.c")
  set(SERVERWRAPPER_XML_DEP s2opc_clientserver-xml-loaders-expat)
endif()

add_library(s2opc_serverwrapper "${SERVERWRAPPER_PATH}/libs2opc_server.c"
                                "${SERVERWRAPPER_PATH}/libs2opc_server_config.c"
                                "${SERVERWRAPPER_PATH}/libs2opc_server_config_custom.c"
                                "${SERVERWRAPPER_PATH}/libs2opc_server_runtime_variables.c"
                                ${SERVERWRAPPER_XML_CONFIG}
                                )
target_link_libraries(s2opc_serverwrapper PUBLIC s2opc_commonwrapper ${SERVERWRAPPER_XML_DEP})
target_compile_options(s2opc_serverwrapper PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_serverwrapper PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(s2opc_serverwrapper
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${SERVERWRAPPER_PATH}>"
  $<INSTALL_INTERFACE:include/s2opc/clientserver/frontend> # when installed
  )
list(APPEND SERVER_WRAPPER_INCLUDE_FILES "${SERVERWRAPPER_PATH}/libs2opc_server.h"
                                         "${SERVERWRAPPER_PATH}/libs2opc_server_config.h"
                                         "${SERVERWRAPPER_PATH}/libs2opc_server_config_custom.h")
set_target_properties(s2opc_serverwrapper PROPERTIES PUBLIC_HEADER "${SERVER_WRAPPER_INCLUDE_FILES}")

install(TARGETS s2opc_serverwrapper
        EXPORT s2opc_clientserver-serverwrapper-export # export our project to be importable after install
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/s2opc/clientserver/frontend)

# install the export as a cmake file
install(EXPORT s2opc_clientserver-serverwrapper-export
        FILE s2opc_clientserver-serverwrapper-export.cmake
        DESTINATION lib/cmake/s2opc)

# export cmake file for use in build tree without install
export(EXPORT s2opc_clientserver-serverwrapper-export
       FILE "${CMAKE_BINARY_DIR}/s2opc_clientserver-serverwrapper-export.cmake")

# Add the PyS2OPC folder lately, as it requires the above library
add_subdirectory(frontend/pys2opc)
