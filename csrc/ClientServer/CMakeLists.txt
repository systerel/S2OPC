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

find_package(EXPAT) # if not found XML loaders will not be compiled

### Compilation of S2OPC client/server library source files ###

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

add_library(s2opc_clientserver ${S2OPC_CLIENTSERVER_SRCS})
target_compile_options(s2opc_clientserver PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_clientserver PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(s2opc_clientserver PUBLIC s2opc_common)

target_include_directories(s2opc_clientserver
  PUBLIC
  "$<BUILD_INTERFACE:${S2OPC_CLIENTSERVER_PUBLIC_INCLUDES}>" # when building (quotes needed to interpret list correclty)
  $<INSTALL_INTERFACE:include}> # when installed
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
        PUBLIC_HEADER DESTINATION include)

install(FILES ${S2OPC_ROOT_PATH}/scripts/generate-s2opc-address-space.py DESTINATION bin)

# install the export as a cmake file
install(EXPORT s2opc_clientserver-export
  FILE s2opc_clientserver-export.cmake
  DESTINATION cmake)

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
add_library(s2opc_clientserver-loader-embedded STATIC loaders/address_space_loaders/embedded/sopc_addspace_loader.c)
target_compile_options(s2opc_clientserver-loader-embedded PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_clientserver-loader-embedded PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(s2opc_clientserver-loader-embedded PRIVATE s2opc_clientserver)
target_include_directories(s2opc_clientserver-loader-embedded PUBLIC "loaders/address_space_loaders")
set_target_properties(s2opc_clientserver-loader-embedded PROPERTIES PUBLIC_HEADER "loaders/address_space_loaders/embedded/sopc_addspace_loader.h")
install(TARGETS s2opc_clientserver-loader-embedded
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION "include/embedded")

# XML addresss space loader and XML server configuration loader
if (EXPAT_FOUND)
  message("-- Expat library found: XML config loaders compilation active")
  add_library(s2opc_clientserver-xml-loaders-expat STATIC loaders/helpers/sopc_helper_expat.c
                                             loaders/address_space_loaders/xml_expat/sopc_uanodeset_loader.c
                                             loaders/config_loaders/xml_expat/sopc_config_loader.c)
  target_compile_options(s2opc_clientserver-xml-loaders-expat PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_clientserver-xml-loaders-expat PRIVATE ${S2OPC_DEFINITIONS} "UANODESET_LOADER_LOG")
  target_link_libraries(s2opc_clientserver-xml-loaders-expat PUBLIC ${EXPAT_LIBRARIES} PRIVATE s2opc_clientserver)
  target_include_directories(s2opc_clientserver-xml-loaders-expat PRIVATE ${EXPAT_INCLUDE_DIRS}
                             PUBLIC "loaders/address_space_loaders"
                                    "loaders/config_loaders"
                                    "loaders/helpers")
  set_property(TARGET s2opc_clientserver-xml-loaders-expat PROPERTY
               PUBLIC_HEADER "loaders/address_space_loaders/xml_expat/sopc_uanodeset_loader.h"
                             "loaders/config_loaders/xml_expat/sopc_config_loader.h")
  install(TARGETS s2opc_clientserver-xml-loaders-expat
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION "include/xml_expat")
else()
  message("-- Expat library NOT found: XML config loaders will not be available")
endif()

## Subscribing client library ##

set(LIBSUBSCRIBE_PATH "${S2OPC_ROOT_PATH}/tests/ClientServer/client_subscription") # move into csrc ?
add_library(client_subscription "${LIBSUBSCRIBE_PATH}/libs2opc_client.c"
                                "${LIBSUBSCRIBE_PATH}/libs2opc_client_cmds.c"
                                "${LIBSUBSCRIBE_PATH}/libs2opc_client_common.c"
                                "${LIBSUBSCRIBE_PATH}/toolkit_helpers.c"
                                "${LIBSUBSCRIBE_PATH}/state_machine.c"
                                "${LIBSUBSCRIBE_PATH}/pki_permissive.c"
                                )
target_link_libraries(client_subscription PUBLIC s2opc_clientserver)
target_compile_options(client_subscription PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(client_subscription PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(client_subscription PRIVATE ${LIBSUBSCRIBE_PATH})
set_target_properties(client_subscription PROPERTIES PUBLIC_HEADER "${LIBSUBSCRIBE_PATH}/libs2opc_client.h")
install(TARGETS client_subscription
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

## PyS2OPC library: Python 3 binding for S2OPC client ##

# The library is built by python. It may use the shared library but it must not be built with ASan.
# The problem with ASan is to embed a dynamic library compiled with ASan into Python, which is not compiled with ASan.
# The compilation is not done with clang-analyzer either, as this would analyze CFFI-generated code.
if(WITH_PYS2OPC)
  set(PYS2OPC_SRC_PATH "${S2OPC_ROOT_PATH}/tests/ClientServer/pys2opc") #TODO: move into csrcs ?
  set(PYS2OPC_BUILD_PATH "${CMAKE_BINARY_DIR}/pys2opc")
  file(MAKE_DIRECTORY ${PYS2OPC_BUILD_PATH}) # Note: create build directory to use it as working dir
  set(PYS2OPC_FILES "${PYS2OPC_BUILD_PATH}/pys2opc/__init__.py"
                    "${PYS2OPC_BUILD_PATH}/pys2opc/connection.py"
                    "${PYS2OPC_BUILD_PATH}/pys2opc/pys2opc_build.py"
                    "${PYS2OPC_BUILD_PATH}/pys2opc/responses.py"
                    "${PYS2OPC_BUILD_PATH}/pys2opc/s2opc.py"
                    "${PYS2OPC_BUILD_PATH}/pys2opc/types.py"
                    )
  set(PYS2OPC_PLACEHOLDER ${CMAKE_BINARY_DIR}/pys2opc_touch)
  add_custom_command(OUTPUT ${PYS2OPC_PLACEHOLDER}
                     # Note: sources copied to build dir to avoid relative path issues in pys2opc_build.py
                     COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYS2OPC_SRC_PATH} ${PYS2OPC_BUILD_PATH} 
                     COMMAND ${PYTHON_EXECUTABLE} ${PYS2OPC_BUILD_PATH}/setup.py build_ext -b ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -t ${CMAKE_BINARY_DIR}
                     COMMAND ${CMAKE_COMMAND} -E touch ${PYS2OPC_PLACEHOLDER}
                     WORKING_DIRECTORY ${PYS2OPC_BUILD_PATH})
  add_custom_target(pys2opc ALL
                    DEPENDS ${PYS2OPC_PLACEHOLDER}
                    DEPENDS ${PYS2OPC_FILES})
  add_dependencies(pys2opc client_subscription)
endif()
