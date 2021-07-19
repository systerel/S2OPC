cmake_minimum_required(VERSION 3.5)
project(S2OPC_Samples LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../CommonDefs.cmake)
endif()

if(NOT S2OPC_PUBSUB_ONLY)
  add_subdirectory(ClientServer)
  if(NOT S2OPC_CLIENTSERVER_ONLY)
    add_subdirectory(PubSub_ClientServer)
  endif()
endif()
if(NOT S2OPC_CLIENTSERVER_ONLY)
  # Note: only PubSub_Server samples available for now
  add_subdirectory(PubSub)
endif()
