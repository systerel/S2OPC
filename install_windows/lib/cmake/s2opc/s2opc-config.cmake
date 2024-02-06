

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../users/vincent/git/S2OPC_clone/install_windows" ABSOLUTE)

####################################################################################

if(POLICY CMP0012)
  # Make push/pop policy default behavior
  cmake_policy(SET CMP0011 NEW)
  # Make if() recognizes numbers and boolean constants
  cmake_policy(SET CMP0012 NEW)
endif()

include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_common-export.cmake")
if(1)
else()
  include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_pubsub-export.cmake")
endif()
if(OFF)
else()
  include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_clientserver-export.cmake")
  include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_clientserver-loader-embedded-export.cmake")
endif()

# Trigger find_package(expat ...) when using find_package(s2opc CONFIG)
if(1)
  include(CMakeFindDependencyMacro)
  find_dependency(expat)
endif()

if()
  include(CMakeFindDependencyMacro)
  find_dependency(eclipse-paho-mqtt-c)
endif()
