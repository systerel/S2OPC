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

# Python3 is mandatory for compilation
if(POLICY CMP0148)
  cmake_policy(SET CMP0148 OLD)
endif()
find_package(PythonInterp 3 REQUIRED)

#### Samples for S2OPC PubSub library ####

# Options reserved to samples
option(PUBSUB_STATIC_CONFIG "Load PubSub configuration statically" OFF)

## Functions used to generate config data for samples ##

# S2OPC PubSub XML config static parser
function(pubsub_embed_config c_file_name xml_file_path)

  add_custom_command(
        OUTPUT ${c_file_name}
        DEPENDS ${xml_file_path}
        COMMAND ${PYTHON_EXECUTABLE} ${S2OPC_ROOT_PATH}/scripts/generate-s2opc_pubsub-static-config.py ${xml_file_path} ${c_file_name}
        COMMENT "Generating pubsub config ${var_name}"
        VERBATIM
    )

    set_source_files_properties(${c_file_name} PROPERTIES GENERATED TRUE)
    if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
        set_source_files_properties(${c_file_name} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
    endif()

endfunction()

# if static configuration, set flags and generate static address space
if(PUBSUB_STATIC_CONFIG)
  message(STATUS "Use of static pubsub configuration activated")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPUBSUB_STATIC_CONFIG")
  set(STATIC_ADDRESS_SPACE_C "pubsub_server_address_space.c")
  set(STATIC_PUBSUB_CONFIG_C "pubsub_server_config_static.c")
  s2opc_embed_address_space(${STATIC_ADDRESS_SPACE_C} "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_pubsub_nodeset.xml")
  pubsub_embed_config(${STATIC_PUBSUB_CONFIG_C} "${CMAKE_CURRENT_SOURCE_DIR}/data/config/config_pubsub_server.xml")
endif()

# Demo PubSub server
add_executable(pubsub_server "pubsub_server/main.c"
                             "pubsub_server/server.c"
                             "pubsub_server/client.c"
                             "pubsub_server/pubsub.c"
                             "pubsub_server/helpers.c"
                             "${STATIC_PUBSUB_CONFIG_C}"
                             "${STATIC_ADDRESS_SPACE_C}"
                             )
target_compile_options(pubsub_server PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(pubsub_server PRIVATE ${S2OPC_DEFINITIONS})

if (WITH_STATIC_SECURITY_DATA)
  target_compile_definitions(pubsub_server PRIVATE "WITH_STATIC_SECURITY_DATA")
  #include static data headers from tests
  target_include_directories(pubsub_server PRIVATE
                                           ${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/server
                                           ${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/client)
endif()


target_link_libraries(pubsub_server PRIVATE s2opc_pubsub s2opc_clientserver s2opc_clientserver-loader-embedded)
target_include_directories(pubsub_server PRIVATE pubsub_server)

# Copy data
add_custom_command(
  TARGET pubsub_server POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public
# Reuse certs of ClientServer samples
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/crl
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/certs
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/crl
  # Add S2OPC application certs in S2OPC_Demo_PKI
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/crl
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/certs/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/crl/
  # Reuse certs of ClientServer samples
  COMMAND ${CMAKE_COMMAND} -E copy  ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
  COMMAND ${CMAKE_COMMAND} -E copy  ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/sks_server_2k_cert.der
  # PubSub samples specific data
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/security/signingKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/security/encryptKey.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/security/keyNonce.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_pubsub_nodeset.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/config_pubsub_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/config_sub_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/config_pub_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )

# Controller Demo PubSub (server) with push SKS client
add_executable(controller_sks_client "sks_push_model/controller_sks_client/controller_main.c"
                             "sks_push_model/controller_sks_client/controller_server.c"
                             "sks_push_model/controller_sks_client/controller_client.c"
                             "sks_push_model/controller_sks_client/controller_pubsub.c"
                             "pubsub_server/helpers.c"
                             )
target_compile_options(controller_sks_client PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(controller_sks_client PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(controller_sks_client PRIVATE s2opc_pubsub s2opc_clientserver)
target_include_directories(controller_sks_client PRIVATE pubsub_server controller_sks_client)
add_custom_command(
  TARGET controller_sks_client POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/controller_pubsub_SKS_client.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/controller_client_config.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/controller_server_config.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/pubsub_server_users_config.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )

# Device Demo PubSub server with SKS push method
add_executable(device_sks_server_push "sks_push_model/device_sks_server_push/device_main.c"
                                      "sks_push_model/device_sks_server_push/device_server.c"
                                      "sks_push_model/device_sks_server_push/device_pubsub.c"
                                      "sks_push_model/device_sks_server_push/sks_device_push_methods.c"
                                      "pubsub_server/helpers.c"
                                      )
target_compile_options(device_sks_server_push PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(device_sks_server_push PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(device_sks_server_push PRIVATE s2opc_pubsub s2opc_clientserver)
target_include_directories(device_sks_server_push PRIVATE pubsub_server device_sks_server_push)
add_custom_command(
  TARGET device_sks_server_push POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_pubsub_nodeset_with_SKS_push.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/device_pubsub_SKS_push_server.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config/device_server_config.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )
