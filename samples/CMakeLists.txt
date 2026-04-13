cmake_minimum_required(VERSION 3.12)
project(S2OPC_Samples LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../CommonDefs.cmake)
endif()

if(NOT S2OPC_PUBSUB_ONLY)
  add_subdirectory(ClientServer)
endif()

if(NOT S2OPC_CLIENTSERVER_ONLY)
  add_subdirectory(PubSub)
endif()

if(NOT S2OPC_PUBSUB_ONLY AND NOT S2OPC_CLIENTSERVER_ONLY)
  add_subdirectory(PubSub_ClientServer)
endif()

add_subdirectory(embedded)
