cmake_minimum_required(VERSION 3.12)
project(S2OPC_Tests LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../CommonDefs.cmake)
endif()

# Not embedded test are compatible with Linux and Windows
if (UNIX OR WIN32)
  if(NOT S2OPC_PUBSUB_ONLY)
    add_subdirectory(ClientServer)
  endif()
  if(NOT S2OPC_CLIENTSERVER_ONLY)
    add_subdirectory(PubSub)
  endif()
endif()
