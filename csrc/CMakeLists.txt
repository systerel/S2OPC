cmake_minimum_required(VERSION 3.5)
project(S2OPC VERSION 1.0.0 LANGUAGES C) #TODO: manage version

add_subdirectory(Common)
#if(NOT S2OPC_CLIENTSERVER_ONLY)
#  add_subdirectory(pubsub)
#endif()
if(NOT S2OPC_PUBSUB_ONLY)
  add_subdirectory(ClientServer)
endif()

## generating relocatable cmake config (valid in build tree / local install / install)
#include(CMakePackageConfigHelpers)
## generate the config file that is includes the exports
#configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in
#  "${CMAKE_BINARY_DIR}/s2opc-config.cmake"
#  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/s2opc"
#  NO_SET_AND_CHECK_MACRO
#  NO_CHECK_REQUIRED_COMPONENTS_MACRO
#  )
## generate the version file for the config file
#write_basic_package_version_file(
#  "${CMAKE_BINARY_DIR}/s2opc-config-version.cmake"
#  VERSION "${s2opc_VERSION_MAJOR}.${s2opc_VERSION_MINOR}"
#  COMPATIBILITY AnyNewerVersion
#)
#
## install the configuration file
#install(FILES
#  ${CMAKE_BINARY_DIR}/s2opc-config.cmake
#  ${CMAKE_BINARY_DIR}/s2opc-config-version.cmake
#  DESTINATION cmake
#  )
