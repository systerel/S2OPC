cmake_minimum_required(VERSION 3.5)
project(S2OPC_Tests LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../CommonDefs.cmake)
endif()

if(NOT S2OPC_PUBSUB_ONLY)
  add_subdirectory(ClientServer)
endif()
if(NOT S2OPC_CLIENTSERVER_ONLY)
  add_subdirectory(PubSub)
endif()
