cmake_minimum_required(VERSION 3.5)
project(S2OPC_PubSub LANGUAGES C)

##                                    ##
#S2OPC client / server toolkit library #
##                                    ##

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

### Dependencies ###

#EXPAT dependency managed in CommonDefs

if (eclipse-paho-mqtt-c_FOUND)
message ( "-- MQTT PUBSUB: Paho library found, it will be used by MQTT Pubsub Protocol extension")
add_definitions(-DUSE_MQTT_PAHO=1)
#Choose static library in priority since Paho export only static or shared version but never both
if (TARGET eclipse-paho-mqtt-c::paho-mqtt3c-static)
  set(PAHO_LIBRARIES eclipse-paho-mqtt-c::paho-mqtt3c-static eclipse-paho-mqtt-c::paho-mqtt3a-static)
else()
  set(PAHO_LIBRARIES eclipse-paho-mqtt-c::paho-mqtt3c eclipse-paho-mqtt-c::paho-mqtt3a)
endif()
else ()
message ( "-- MQTT PUBSUB: no supported MQTT library found (Paho)")
add_definitions(-DUSE_MQTT_PAHO=0)
endif ()

### Compilation of S2OPC client/server library source files ###

## Build info source file generation (version, commit checksum, docker id, date) ##
#Define how to generate build info file
set(S2OPC_PUBSUB_BUILD_INFO_C ${CMAKE_CURRENT_BINARY_DIR}/sopc_pubsub_build_info.c)
s2opc_gen_build_info(${CMAKE_CURRENT_SOURCE_DIR}/common/sopc_pubsub_build_info.c_
                     ${S2OPC_PUBSUB_BUILD_INFO_C})

## C99 compliant source code ##

#Define non - platform dependent source files
file(GLOB_RECURSE S2OPC_PUBSUB_SRCS "*.c")

#Define S2OPC library includes

set(S2OPC_PUBSUB_PUBLIC_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/common"
    "${CMAKE_CURRENT_SOURCE_DIR}/config_loaders/xml_expat"
    "${CMAKE_CURRENT_SOURCE_DIR}/dataset"
    "${CMAKE_CURRENT_SOURCE_DIR}/network"
    "${CMAKE_CURRENT_SOURCE_DIR}/protocol"
    "${CMAKE_CURRENT_SOURCE_DIR}/publisher"
    "${CMAKE_CURRENT_SOURCE_DIR}/security"
    "${CMAKE_CURRENT_SOURCE_DIR}/subscriber"
)

### Configure S2OPC client/server library and its properties ###

add_library(s2opc_pubsub ${S2OPC_PUBSUB_SRCS} ${S2OPC_PUBSUB_BUILD_INFO_C})
target_compile_options(s2opc_pubsub PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_pubsub PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(s2opc_pubsub PUBLIC s2opc_common expat::expat ${PAHO_LIBRARIES})

target_include_directories(s2opc_pubsub
  PUBLIC
  "$<BUILD_INTERFACE:${S2OPC_PUBSUB_PUBLIC_INCLUDES}>" # when building (quotes needed to interpret list correclty)
  $<INSTALL_INTERFACE:include/s2opc/pubsub}> # when installed
  )

### Installation and export of S2OPC client/server toolkit library ###
foreach(dir ${S2OPC_PUBSUB_PUBLIC_INCLUDES})
    file(GLOB temp_files "${dir}/*.h")
    list(APPEND S2OPC_PUBSUB_INCLUDE_FILES ${temp_files})
endforeach()
#set public header property
set_target_properties(s2opc_pubsub PROPERTIES PUBLIC_HEADER "${S2OPC_PUBSUB_INCLUDE_FILES}")
#install files
install(TARGETS s2opc_pubsub
        EXPORT s2opc_pubsub-export # export our project to be importable after install
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/s2opc/pubsub)

install(FILES ${S2OPC_ROOT_PATH}/scripts/generate-s2opc-address-space.py DESTINATION bin)

#install the export as a cmake file
install(EXPORT s2opc_pubsub-export
  FILE s2opc_pubsub-export.cmake
  DESTINATION lib/cmake/s2opc)

#export cmake file for use in build tree without install
export(EXPORT s2opc_pubsub-export
  FILE "${CMAKE_BINARY_DIR}/s2opc_pubsub-export.cmake"
  )
