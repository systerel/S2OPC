cmake_minimum_required(VERSION 3.12)
project(S2OPC_Samples LANGUAGES C)

if(NOT S2OPC_PUBSUB_ONLY)
  add_subdirectory(cli_client)
  if(NOT S2OPC_CLIENTSERVER_ONLY)
    add_subdirectory(cli_pubsub_server)
  endif()
endif()
