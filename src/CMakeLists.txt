cmake_minimum_required(VERSION 3.5)
project(S2OPC VERSION 1.2.0)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../CommonDefs.cmake)
endif()

add_subdirectory(Common)
if(NOT S2OPC_PUBSUB_ONLY)
  add_subdirectory(ClientServer)
else()
  message("-- S2OPC_PUBSUB_ONLY set")
  if(WITH_PYS2OPC)
    # When not adding ClientServer subdirectory, pys2opc is never included,
    #  and the script cannot express its requirements.
    message(SEND_ERROR "-- WITH_PYS2OPC requires Client and Server capabilities")
  endif()
endif()
if(NOT S2OPC_CLIENTSERVER_ONLY)
  add_subdirectory(PubSub)
else()
  message("-- S2OPC_CLIENTSERVER_ONLY set")
endif()

# generating relocatable cmake config (valid in build tree / local install / install)
include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in
  "${CMAKE_BINARY_DIR}/s2opc-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/s2opc"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/s2opc-config-version.cmake"
  VERSION "${S2OPC_VERSION}"
  COMPATIBILITY AnyNewerVersion
)

# install the configuration file, version and custom find modules
install(FILES
  ${CMAKE_BINARY_DIR}/s2opc-config.cmake
  ${CMAKE_BINARY_DIR}/s2opc-config-version.cmake
  ${S2OPC_ROOT_PATH}/CMake/FindMBEDTLS.cmake
  DESTINATION lib/cmake/s2opc
  )

# install the common S2OPC tool scripts
install(FILES
  ${S2OPC_ROOT_PATH}/scripts/generate-s2opc-address-space.py
  ${S2OPC_ROOT_PATH}/scripts/generate-s2opc_pubsub-static-config.py
  ${S2OPC_ROOT_PATH}/scripts/gen-reciprocal-refs-address-space.xslt
  DESTINATION bin
  )
