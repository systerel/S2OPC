cmake_minimum_required (VERSION 3.5)

project (S2OPC_Safety_Demo LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC_Safety_Demo samples ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and sample separately
find_package(s2opc)


if(ENABLE_SAMPLES)
  # Demo NonSafe Producer
  add_executable(prod_ns "src/prod_ns.c"
                        )

  target_compile_options(prod_ns PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(prod_ns PRIVATE ${S2OPC_DEFINITIONS} "-D_GNU_SOURCE")

  target_link_libraries(prod_ns PRIVATE s2opc_pubsub s2opc_clientserver-xml-loaders-expat)
  target_include_directories(prod_ns PRIVATE safety_demo)

  # Demo PubSub
  add_executable(safety_demo "src/main.c"
                        "src/safetyDemo.c"
                        "src/interactive.c"
                        )
  target_compile_options(safety_demo PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(safety_demo PRIVATE ${S2OPC_DEFINITIONS} "-D_GNU_SOURCE")

  target_link_libraries(safety_demo PRIVATE s2opc_pubsub s2opc_clientserver-xml-loaders-expat)
  target_include_directories(safety_demo PRIVATE safety_demo)

  # Copy data
  add_custom_command(
    TARGET safety_demo POST_BUILD
    # Reuse configurations and security from PubSub_ClientServer samples
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/signingKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/encryptKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/security/keyNonce.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    # Configurations
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/safety_demo_cons.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/safety_demo_prov.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    )

endif(ENABLE_SAMPLES)
