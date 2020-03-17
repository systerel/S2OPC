

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../users/vincent/git/S2OPC_dup/install_windows" ABSOLUTE)

####################################################################################
include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_common-export.cmake")
if(1)
else()
  include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_pubsub-export.cmake")
endif()
if()
else()
  # Config loaders
  include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_clientserver-export.cmake")
  include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_clientserver-loader-embedded-export.cmake")
  if(1)
    include ("${CMAKE_CURRENT_LIST_DIR}/s2opc_clientserver-xml-loaders-expat-export.cmake")
  endif()
  # Client wrapper
  include("${CMAKE_CURRENT_LIST_DIR}/s2opc_clientserver-clientwrapper-export.cmake")
endif()
