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

### Manage options for S2OPC compilation ###

# S2OPC library scope option
option(WITH_NANO_EXTENDED "Use Nano profile with additional services out of Nano scope" OFF)

# Add WITH_NANO_EXTENDED to compilation definition if option activated
list(APPEND S2OPC_CLIENTSERVER_DEFINITIONS $<$<BOOL:${WITH_NANO_EXTENDED}>:WITH_NANO_EXTENDED>)

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
target_compile_definitions(s2opc_clientserver PRIVATE ${S2OPC_DEFINITIONS} ${S2OPC_CLIENTSERVER_DEFINITIONS})
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
