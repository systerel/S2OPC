cmake_minimum_required (VERSION 3.5)

project (S2OPC_PubSub_Samples LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC PubSub samples ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and sample separately
find_package(s2opc)


if(ENABLE_SAMPLES)
  # Demo PubSub
  add_executable(pubsub "pubsub/main.c"
                        "pubsub/cache.c"
                        )
  target_compile_options(pubsub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(pubsub PRIVATE ${S2OPC_DEFINITIONS} "-D_GNU_SOURCE")

  target_link_libraries(pubsub PRIVATE s2opc_pubsub s2opc_clientserver-xml-loaders-expat)
  target_include_directories(pubsub PRIVATE pubsub)

  # Demo TSN PubSub server
  add_definitions(-D_GNU_SOURCE)
  add_executable(udp_rt_pub "tsn/udp_rt_pub.c")
  target_compile_options(udp_rt_pub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(udp_rt_pub PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(udp_rt_pub PRIVATE s2opc_pubsub)

  # Copy data
  add_custom_command(
    TARGET pubsub POST_BUILD
    # Reuse configurations and security from PubSub_ClientServer samples
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/signingKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/encryptKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/keyNonce.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    # Configurations
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_rtt_emitter.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_rtt_loopback.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    )

endif(ENABLE_SAMPLES)
