cmake_minimum_required(VERSION 3.12)

project (S2OPC_PubSub_Samples LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC PubSub samples ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and sample separately
find_package(s2opc)

# Demo PubSub
add_executable(pubsub "pubsub/main.c"
                      "pubsub/cache.c"
                      )
target_compile_options(pubsub PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(pubsub PRIVATE ${S2OPC_DEFINITIONS} "-D_GNU_SOURCE")

target_link_libraries(pubsub PRIVATE s2opc_pubsub)
target_include_directories(pubsub PRIVATE pubsub)

if(UNIX) # Presence of include/code specific to Linux in pubsub RT and benchmark
  # Demo TSN PubSub server
  add_definitions(-D_GNU_SOURCE)
  add_executable(udp_rt_pub "tsn/udp_rt_pub.c")
  target_compile_options(udp_rt_pub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(udp_rt_pub PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(udp_rt_pub PRIVATE ${S2OPC_ROOT_PATH}/src/Common/helpers_platform_dep/linux)
  target_link_libraries(udp_rt_pub PRIVATE s2opc_pubsub)

  # Benchmark Pubsub
  add_executable(bench_pubsub "benchmark/main.c"
                              "benchmark/pubsub.c"
                              "benchmark/cache.c")

  target_compile_options(bench_pubsub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(bench_pubsub PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(bench_pubsub PRIVATE s2opc_pubsub)
endif()

# Copy data
add_custom_command(
  TARGET pubsub POST_BUILD
  # Reuse configurations and security from PubSub_ClientServer samples
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/signingKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/encryptKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/keyNonce.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  # SKS server config
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/sks_server_2k_cert.der
  # Configurations
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_rtt_emitter.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_rtt_loopback.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_bench_publisher.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_bench_subscriber.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )
