cmake_minimum_required (VERSION 3.5)

project (S2OPC_ClientServer_Tests LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC client/server tests ####

# We always link check statically, so that our test binaries are standalone

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and tests separately

# Python3 is mandatory for compilation
if(POLICY CMP0148)
  cmake_policy(SET CMP0148 OLD)
endif()
find_package(PythonInterp 3 REQUIRED)

find_package(check REQUIRED)

#### Tests for S2OPC library and extension libraries ####

if(WIN32)
  set(ENV_PATH_SEP "\;")
else()
  set(ENV_PATH_SEP ":")
endif()

### Tests and validation of S2OPC library ###

# Get S2OPC necessary internal header for unit and validation tests
set(S2OPC_CLIENTSERVER_INTERNAL_INCLUDES
  "${S2OPC_ROOT_PATH}/src/ClientServer/address_space"
  "${S2OPC_ROOT_PATH}/src/ClientServer/api_toolkit"
  "${S2OPC_ROOT_PATH}/src/ClientServer/configuration"
  "${S2OPC_ROOT_PATH}/src/ClientServer/secure_channels"
  "${S2OPC_ROOT_PATH}/src/ClientServer/services"
  "${S2OPC_ROOT_PATH}/src/ClientServer/services/b2c"
  "${S2OPC_ROOT_PATH}/src/ClientServer/services/bgenc"
  "${S2OPC_ROOT_PATH}/src/ClientServer/sockets"
  )

# Get the helpers for unit and validation tests
set(S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES ${S2OPC_ROOT_PATH}/tests/ClientServer/helpers)
file(GLOB S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES "${S2OPC_ROOT_PATH}/tests/ClientServer/helpers/*.c")

enable_testing()

# Function to add a unit test to the set of tests: define a test generating a TAP file from a binary name
function(s2opc_unit_test target_name)
  set(test_name "unit::${target_name}")
  set(test_cmd $<TARGET_FILE:${target_name}>)

  foreach(arg ${ARGN})
    if("${arg}" STREQUAL "GENERATE_TAP")
      set(test_cmd "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py" "${test_cmd}")
    else()
      message(FATAL_ERROR "Unknown parameter for s2opc_unit_test: ${arg}")
    endif()
  endforeach()

  add_test(NAME "${test_name}" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${test_cmd})
  set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${target_name}.tap")
endfunction()

set(TEST_CONFIG_XML              "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Test_XML_Config.xml")
set(TEST_USERS_XML               "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Test_Users.xml")
set(TEST_SERVER_UACTT_CONFIG_XML "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Server_UACTT_Config.xml")
set(TEST_USERS_UACTT_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_UACTT_Users.xml")
set(TEST_FAULTY_PACKET_CONFIG_XML "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Server_Test_Faulty_Packet_Config.xml")

set(TEST_LIGHT_ADDRESS_SPACE_XML "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc_no_base_info.xml")

if (S2OPC_NANO_PROFILE)
  set(TEST_ADDRESS_SPACE_XML_ORIGIN "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc_nano.xml")
else()
  if(S2OPC_NODE_MANAGEMENT)
    set(TEST_ADDRESS_SPACE_XML_ORIGIN "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc_node_mgt.xml")
  else()
    set(TEST_ADDRESS_SPACE_XML_ORIGIN "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc.xml")
  endif()
endif()
set(TEST_ADDRESS_SPACE_XML_FILE "S2OPC_Test_NodeSet.xml")

set(TEST_SERVER_ADDRESS_SPACE_C "test_server_address_space.c")
set(TEST_LIGHT_ADDRESS_SPACE_C "s2opc_no_base_info_address_space.c")

s2opc_embed_address_space(${TEST_LIGHT_ADDRESS_SPACE_C} ${TEST_LIGHT_ADDRESS_SPACE_XML})
add_custom_target(make-light-address-space DEPENDS ${TEST_LIGHT_ADDRESS_SPACE_C})

s2opc_embed_address_space(${TEST_SERVER_ADDRESS_SPACE_C} ${TEST_ADDRESS_SPACE_XML_ORIGIN})
add_custom_target(make-server-address-space DEPENDS ${TEST_SERVER_ADDRESS_SPACE_C})

## S2OPC unit tests ##

# Helpers tests

file(GLOB INTERNAL_TESTS_SRCS "unit_tests/helpers/check_*.c")
add_executable(check_helpers ${INTERNAL_TESTS_SRCS}
                             unit_tests/helpers/custom_types.c
                             unit_tests/helpers/custom2_types.c
                             ${TEST_SERVER_ADDRESS_SPACE_C})
add_dependencies(check_helpers make-server-address-space)
target_include_directories(check_helpers PRIVATE ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}
                                                 "validation_tests/server") # Reuse data of test server

target_link_libraries(check_helpers PRIVATE Check::check s2opc_clientserver s2opc_clientserver-loader-embedded)
target_compile_options(check_helpers PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_helpers PRIVATE ${S2OPC_DEFINITIONS})
if (S2OPC_CRYPTO_CYCLONE)
  target_compile_definitions(check_helpers PRIVATE "S2OPC_CRYPTO_CYCLONE")
endif()
if(expat_FOUND)
  target_compile_definitions(check_helpers PRIVATE "WITH_EXPAT")
endif()
if(WITH_CONST_ADDSPACE)
  target_compile_definitions(check_helpers PRIVATE "WITH_CONST_ADDSPACE")
endif()
s2opc_unit_test(check_helpers)

add_custom_command(
  TARGET check_helpers POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logAnother.ref ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logCircular.ref0 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logCircular.ref1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logCircular.ref2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logTest.ref1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logTest.ref2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/logger/logTest.ref3 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_USERS_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_SERVER_UACTT_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_USERS_UACTT_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_FAULTY_PACKET_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_ADDRESS_SPACE_XML_ORIGIN} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_ADDRESS_SPACE_XML_FILE}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/invalid_pki_data
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/invalid_pki/cacrl_not_renewed.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/invalid_pki_data/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/invalid_pki/ca_selfsigned_pathLen1.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/invalid_pki_data/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/invalid_pki/ca_selfsigned_missingPathLen.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/invalid_pki_data/)

# Ask password binary for test
add_executable(ask_password unit_tests/helpers/ask_password.c)
target_include_directories(ask_password PRIVATE ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
target_link_libraries(ask_password PRIVATE s2opc_common)
target_compile_options(ask_password PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_helpers PRIVATE ${S2OPC_DEFINITIONS})
add_test(NAME "unit:ask_password"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/ask_password_tests.py"
  )
set_tests_properties("unit:ask_password" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ask_password.tap")

# SKS tests
file(GLOB INTERNAL_TESTS_SKS_SRCS "unit_tests/sks/*.c")
add_executable(check_sks ${INTERNAL_TESTS_SKS_SRCS})
target_include_directories(check_sks PRIVATE ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
target_compile_options(check_sks PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_sks PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(check_sks PRIVATE Check::check s2opc_clientserver)
s2opc_unit_test(check_sks)

# Sockets layer tests

add_executable(check_sockets "unit_tests/sockets/check_sockets.c")
target_include_directories(check_sockets PRIVATE ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
target_compile_options(check_sockets PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_sockets PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(check_sockets PRIVATE Check::check s2opc_clientserver)
s2opc_unit_test(check_sockets)

# Security policy configuration tests

add_executable(check_security_policy_config "unit_tests/secure_channels/check_security_policy_config.c")
target_compile_options(check_security_policy_config PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_security_policy_config PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(check_security_policy_config PRIVATE Check::check s2opc_clientserver)
s2opc_unit_test(check_security_policy_config)

# Secure channels (stubbed on services side) tests
file(GLOB SECURE_CHANNELS_SRC "${S2OPC_ROOT_PATH}/src/ClientServer/configuration/sopc_ieee_check.c"
  "${S2OPC_ROOT_PATH}/src/ClientServer/secure_channels/*.c")

# Secure channels tests: tests on received buffer
file(GLOB SECURE_CHANNELS_BUFFER_TEST
  "unit_tests/secure_channels/event_recorder.c"
  "unit_tests/secure_channels/check_sc_rcv_buffer.c"
  "unit_tests/secure_channels/check_sc_rcv_helpers.c"
  "unit_tests/secure_channels/event_helpers.c"
  "${S2OPC_ROOT_PATH}/src/Common/helpers/sopc_helper_encode.c"
  "unit_tests/secure_channels/stubs/*.c")

add_executable(check_sc_rcv_buffer ${SECURE_CHANNELS_SRC}
  ${SECURE_CHANNELS_BUFFER_TEST})
target_link_libraries(check_sc_rcv_buffer PRIVATE Check::check s2opc_common)
target_compile_options(check_sc_rcv_buffer PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_sc_rcv_buffer PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(check_sc_rcv_buffer PRIVATE "unit_tests/secure_channels/stubs"
  "unit_tests/secure_channels"
  "unit_tests/helpers"
  ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
s2opc_unit_test(check_sc_rcv_buffer)

# Secure channels tests: tests on received encrypted buffer
file(GLOB SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST
  "unit_tests/secure_channels/event_recorder.c"
  "unit_tests/secure_channels/check_sc_rcv_helpers.c"
  "unit_tests/secure_channels/check_sc_rcv_encrypted_buffer.c"
  "unit_tests/secure_channels/event_helpers.c"
  "${S2OPC_ROOT_PATH}/src/Common/helpers/sopc_helper_encode.c"
  "unit_tests/secure_channels/stubs/*.c")

add_executable(check_sc_rcv_encrypted_buffer ${SECURE_CHANNELS_SRC}
  ${SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
target_link_libraries(check_sc_rcv_encrypted_buffer PRIVATE Check::check s2opc_common)
target_compile_options(check_sc_rcv_encrypted_buffer PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_sc_rcv_encrypted_buffer PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(check_sc_rcv_encrypted_buffer PRIVATE "unit_tests/secure_channels/stubs"
  "unit_tests/secure_channels"
  "unit_tests/helpers"
  ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
add_custom_command(
  TARGET check_sc_rcv_encrypted_buffer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/check_sc_test/encrypted_check_sc_client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/check_sc_test/check_sc_client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/check_sc_test/check_sc_server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/)
s2opc_unit_test(check_sc_rcv_encrypted_buffer)

# Sockets (stubbed on services side) tests
file(GLOB SOCKETS_SRC "${S2OPC_ROOT_PATH}/src/ClientServer/sockets/*.c")

file(GLOB SECURE_CHANNELS_CLIENT_TEST
  "unit_tests/secure_channels/event_helpers.c"
  "unit_tests/secure_channels/event_recorder.c"
  "unit_tests/secure_channels/test_secure_channels_client.c"
  "unit_tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c"
  "unit_tests/secure_channels/stubs/stub_sc_sopc_services_api.c")

file(GLOB SECURE_CHANNELS_SERVER_TEST
  "unit_tests/secure_channels/event_helpers.c"
  "unit_tests/secure_channels/event_recorder.c"
  "unit_tests/secure_channels/test_secure_channels_server.c"
  "unit_tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c"
  "unit_tests/secure_channels/stubs/stub_sc_sopc_services_api.c")

# Secure channels (+ Sockets) layer tests: CLIENT executable
add_executable(test_secure_channels_client ${SOCKETS_SRC}
  ${SECURE_CHANNELS_SRC}
  ${SECURE_CHANNELS_CLIENT_TEST}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})

target_include_directories(test_secure_channels_client PRIVATE "unit_tests/secure_channels/stubs"
  ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(test_secure_channels_client PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(test_secure_channels_client PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(test_secure_channels_client PRIVATE s2opc_common)

# Secure channels (+ Sockets) layer tests: SERVER executable
add_executable(test_secure_channels_server ${SOCKETS_SRC}
  ${SECURE_CHANNELS_SRC}
  ${SECURE_CHANNELS_SERVER_TEST}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
target_link_libraries(test_secure_channels_server PRIVATE s2opc_common)
target_include_directories(test_secure_channels_server PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(test_secure_channels_server PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(test_secure_channels_server PRIVATE ${S2OPC_DEFINITIONS})
target_include_directories(test_secure_channels_server PRIVATE "unit_tests/secure_channels/stubs"
  ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})

  ## S2OPC validation tests ##

# Tests compilation definition #

# Client/Server test
add_executable(toolkit_test_server_client
  "validation_tests/client/toolkit_test_server_client.c"
  ${TEST_LIGHT_ADDRESS_SPACE_C}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
add_dependencies(toolkit_test_server_client make-light-address-space)
target_link_libraries(toolkit_test_server_client PRIVATE
  s2opc_clientserver
  s2opc_clientserver-loader-embedded
  Check::check)
target_include_directories(toolkit_test_server_client PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(toolkit_test_server_client PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_server_client PRIVATE ${S2OPC_DEFINITIONS})
if (expat_FOUND)
  target_compile_definitions(toolkit_test_server_client PRIVATE "WITH_EXPAT")
endif()
s2opc_unit_test(toolkit_test_server_client)

# Server running local services
add_executable(toolkit_test_server_local_service "validation_tests/server/toolkit_test_server_local_service.c"
  "validation_tests/client/wrap_read.c"
  "validation_tests/client/testlib_write.c"
  "validation_tests/client/test_results.c"
  "validation_tests/client/testlib_read_response.c"
  ${TEST_SERVER_ADDRESS_SPACE_C}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES}
  "unit_tests/helpers/custom_types.c")
add_dependencies(toolkit_test_server_local_service make-server-address-space)
target_include_directories(toolkit_test_server_local_service PRIVATE
                           "validation_tests/client"
                           ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES}
                           "unit_tests/helpers")
target_link_libraries(toolkit_test_server_local_service PRIVATE s2opc_clientserver s2opc_clientserver-loader-embedded)
target_compile_options(toolkit_test_server_local_service PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_server_local_service PRIVATE ${S2OPC_DEFINITIONS})
s2opc_unit_test(toolkit_test_server_local_service GENERATE_TAP)

if (expat_FOUND)
  target_compile_definitions(toolkit_test_server_local_service PRIVATE "WITH_EXPAT")
endif()



# Client scenario
set(TEST_CLIENT_CONFIG_XML "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Client_Test_Config.xml")

add_executable(toolkit_test_client "validation_tests/client/toolkit_test_client.c"
  "validation_tests/client/wrap_read.c"
  "validation_tests/client/testlib_write.c"
  "validation_tests/client/test_results.c"
  "validation_tests/client/testlib_read_response.c"
  ${TEST_LIGHT_ADDRESS_SPACE_C}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
add_dependencies(toolkit_test_client make-light-address-space)
target_link_libraries(toolkit_test_client PRIVATE s2opc_clientserver s2opc_clientserver-loader-embedded)
target_include_directories(toolkit_test_client PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(toolkit_test_client PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_client PRIVATE ${S2OPC_DEFINITIONS})
if (expat_FOUND)
  target_compile_definitions(toolkit_test_client PRIVATE "WITH_EXPAT")
endif()
if (WITH_STATIC_SECURITY_DATA)
  target_compile_definitions(toolkit_test_client PRIVATE "WITH_STATIC_SECURITY_DATA")
  # Reuse data of test server
  target_include_directories(toolkit_test_client PRIVATE "validation_tests/server")
endif()


add_custom_command(
  TARGET toolkit_test_client POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_CLIENT_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Client test AddNodes
add_executable(toolkit_test_client_add_nodes "validation_tests/client/toolkit_test_client_add_nodes.c"
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
target_link_libraries(toolkit_test_client_add_nodes PRIVATE s2opc_clientserver)
target_include_directories(toolkit_test_client_add_nodes PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(toolkit_test_client_add_nodes PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_client_add_nodes PRIVATE ${S2OPC_DEFINITIONS})
if (expat_FOUND)
  target_compile_definitions(toolkit_test_client_add_nodes PRIVATE "WITH_EXPAT")
endif()

# Client test DeleteNodes
add_executable(toolkit_test_client_delete_nodes "validation_tests/client/toolkit_test_client_delete_nodes.c"
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
target_link_libraries(toolkit_test_client_delete_nodes PRIVATE s2opc_clientserver)
target_include_directories(toolkit_test_client_delete_nodes PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(toolkit_test_client_delete_nodes PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_client_delete_nodes PRIVATE ${S2OPC_DEFINITIONS})
if (expat_FOUND)
  target_compile_definitions(toolkit_test_client_delete_nodes PRIVATE "WITH_EXPAT")
endif()

# Client test for events subscription
if (expat_FOUND)
  add_executable(toolkit_test_client_events "validation_tests/client/toolkit_test_client_events.c"
                 ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
  target_link_libraries(toolkit_test_client_events PRIVATE s2opc_clientserver)
  target_include_directories(toolkit_test_client_events PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
  target_compile_options(toolkit_test_client_events PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_client_events PRIVATE ${S2OPC_DEFINITIONS})

  add_custom_command(
  TARGET toolkit_test_client_events POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${TEST_CLIENT_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endif()

# Client scenario with service faults
add_executable(toolkit_test_client_service_faults "validation_tests/client/toolkit_test_client_service_faults.c")
target_compile_options(toolkit_test_client_service_faults PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_client_service_faults PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(toolkit_test_client_service_faults PRIVATE s2opc_clientserver)

# Client scenario with ReverseConnection faults
add_executable(toolkit_test_client_rhe_faults "validation_tests/client/toolkit_test_client_rhe_faults.c")
target_compile_options(toolkit_test_client_rhe_faults PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_client_rhe_faults PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(toolkit_test_client_rhe_faults PRIVATE s2opc_clientserver)

# Client scenario with file transfer
if (NOT WIN32 AND expat_FOUND)
  add_executable(check_file_transfer "validation_tests/client/file_transfer/check_file_transfer.c"
                                                  "validation_tests/client/file_transfer/check_file_transfer_method.c")
  target_include_directories(check_file_transfer PRIVATE "validation_tests/client/file_transfer")
  add_dependencies(check_file_transfer toolkit_test_server_file_transfer)
  target_link_libraries(check_file_transfer PRIVATE s2opc_clientserver Check::check)
  target_compile_options(check_file_transfer PRIVATE ${S2OPC_COMPILER_FLAGS} -D_GNU_SOURCE) # GNU_SOURCE for fileno function.
  target_compile_definitions(check_file_transfer PRIVATE ${S2OPC_DEFINITIONS})
  add_custom_command(
      TARGET check_file_transfer POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/ft_data/bin_file.a ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ft_data/bin_file.a)
endif()

# Client services scenarii

# Reuse code of demo client
set(S2OPC_CLIENTDEMO_INCLUDES ${S2OPC_ROOT_PATH}/samples/ClientServer/demo_client)

add_executable(toolkit_test_suite_client "validation_tests/client/test_suite_client.c"
  "validation_tests/client/test_discovery.c"
  "validation_tests/client/test_session.c"
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
target_link_libraries(toolkit_test_suite_client PRIVATE s2opc_clientserver Check::check)
target_include_directories(toolkit_test_suite_client PRIVATE ${S2OPC_CLIENTDEMO_INCLUDES} ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(toolkit_test_suite_client PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_suite_client PRIVATE ${S2OPC_DEFINITIONS})

# Server scenario for file transfer
if (NOT WIN32 AND expat_FOUND)
  add_executable(toolkit_test_server_file_transfer "validation_tests/server/toolkit_test_server_file_transfer.c"
                                                    "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_file_transfer/sopc_file_transfer.c"
                                                    ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
  target_include_directories(toolkit_test_server_file_transfer PRIVATE "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_file_transfer"
                                                                        ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
  target_link_libraries(toolkit_test_server_file_transfer PRIVATE s2opc_clientserver)
  target_compile_options(toolkit_test_server_file_transfer PRIVATE ${S2OPC_COMPILER_FLAGS} -D_GNU_SOURCE) # GNU_SOURCE for fileno and mkstemp functions.
  target_compile_definitions(toolkit_test_server_file_transfer PRIVATE ${S2OPC_DEFINITIONS} "WITH_EXPAT")
endif()

# Server scenario (configured for UACTT)

# Manage the Nano profiles extended mode
if (S2OPC_NANO_PROFILE)
  set(TOOLKIT_TEST_SERVER "toolkit_test_nano_server")
else()
  set(TOOLKIT_TEST_SERVER "toolkit_test_server")
endif()

add_executable(${TOOLKIT_TEST_SERVER}
  "${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/server/toolkit_test_server.c"
  ${TEST_SERVER_ADDRESS_SPACE_C}
  ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES}
  "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_server/toolkit_demo_server_methods.c")
add_dependencies(${TOOLKIT_TEST_SERVER} make-server-address-space)
target_link_libraries(${TOOLKIT_TEST_SERVER} PRIVATE s2opc_clientserver s2opc_clientserver-loader-embedded)
target_include_directories(${TOOLKIT_TEST_SERVER} PRIVATE
  ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES}
  "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_server")
target_compile_options(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_DEFINITIONS})
if (expat_FOUND)
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_EXPAT")
endif()

if (WITH_STATIC_SECURITY_DATA)
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_STATIC_SECURITY_DATA")
endif()


# Test server SKS

if (NOT S2OPC_NANO_PROFILE)
  set(TEST_SKS_ADDRESS_SPACE_C "test_sks_address_space.c")
  set(TEST_SKS_ADDRESS_SPACE_XML "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc_sks.xml")
  s2opc_embed_address_space(${TEST_SKS_ADDRESS_SPACE_C} ${TEST_SKS_ADDRESS_SPACE_XML})
  add_custom_target(make-sks-address-space DEPENDS ${TEST_SKS_ADDRESS_SPACE_C})
  add_executable(test_server_sks
    "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_server/sks_demo_server_methods.c"
    "${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/server/test_sks_server.c"
    ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES}
    ${TEST_SKS_ADDRESS_SPACE_C})
  target_link_libraries(test_server_sks PRIVATE s2opc_clientserver-loader-embedded)
  target_include_directories(test_server_sks PRIVATE
    ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES}
    "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_server")
  target_compile_options(test_server_sks PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(test_server_sks PRIVATE ${S2OPC_DEFINITIONS})
  if (expat_FOUND)
    target_link_libraries(test_server_sks PRIVATE s2opc_clientserver)
    target_compile_definitions(test_server_sks PRIVATE "WITH_EXPAT")
  endif()
  if (WITH_STATIC_SECURITY_DATA)
    target_compile_definitions(test_server_sks PRIVATE "WITH_STATIC_SECURITY_DATA")
  endif()
endif()

function(make_pki target_exe pki_name)
  add_custom_command(
    TARGET ${target_exe} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/trusted
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/trusted/certs
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/trusted/crl
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/issuers
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/issuers/certs
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/issuers/crl
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/trusted/certs
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/trusted/crl
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/issuers/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pki_name}/issuers/crl/
    )
endfunction()

add_custom_command(
  TARGET ${TOOLKIT_TEST_SERVER} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_public
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_private
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/certs
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/crl
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/certs
  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/crl
  # Add S2OPC application certs in S2OPC_Users_PKI
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/user_cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/certs
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/user_cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/crl
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/certs/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/crl/
  # Add UaCTT user certs in S2OPC_UACTT_Users_PKI
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/X509UserIdentity_PKI/trusted/certs/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_Users_PKI/trusted/certs/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/X509UserIdentity_PKI/trusted/crl/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_Users_PKI/trusted/crl/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/X509UserIdentity_PKI/issuers/certs/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_Users_PKI/issuers/certs/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/X509UserIdentity_PKI/issuers/crl/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_Users_PKI/issuers/crl/
  # Add S2OPC user certs in S2OPC_UACTT_Users_PKI
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/user_cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_Users_PKI/trusted/certs
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/user_cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_Users_PKI/trusted/crl
  # Add UaCTT application certs in S2OPC_UACTT_PKI
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_PKI/trusted/certs/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/crl/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_PKI/trusted/crl/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_PKI/issuers/certs/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/crl/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_PKI/issuers/crl/
  # Add S2OPC application certs in S2OPC_UACTT_PKI
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_PKI/trusted/certs
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_UACTT_PKI/trusted/crl
  # Add 2048 client, server and user keys
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_user_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/user_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_public/
  # Add 4096 client, server and user keys
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_client_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_user_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_private/
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/user_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_public/
  )

make_pki("toolkit_test_server" "S2OPC_Demo_PKI")

# Test discovery server

add_executable(toolkit_test_discovery_server
  "${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/server/toolkit_test_discovery_server.c"
  ${TEST_SERVER_ADDRESS_SPACE_C})
add_dependencies(toolkit_test_discovery_server make-server-address-space)
target_link_libraries(toolkit_test_discovery_server PRIVATE s2opc_clientserver s2opc_clientserver-loader-embedded)
target_compile_options(toolkit_test_discovery_server PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(toolkit_test_discovery_server PRIVATE ${S2OPC_DEFINITIONS})

# Tests run definition #

#Note: use absolute path for scripts to allow ctest to be run from another another path

# Function to add validation test
function(s2opc_validation_test target_name bin_name)
  add_test(NAME "validation::${target_name}"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
  "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
  "--server-cmd" $<TARGET_FILE:${TOOLKIT_TEST_SERVER}>
  $<TARGET_FILE:${bin_name}>)
  set_tests_properties("validation::${target_name}" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${target_name}.tap")
endfunction()

s2opc_validation_test(client_server_test toolkit_test_client)
s2opc_validation_test(client_server_test_add_nodes toolkit_test_client_add_nodes)
s2opc_validation_test(client_server_test_delete_nodes toolkit_test_client_delete_nodes)
s2opc_validation_test(client_service_faults_test toolkit_test_client_service_faults)
s2opc_validation_test(client_RHE_faults_test toolkit_test_client_rhe_faults)

if(S2OPC_EVENT_MANAGEMENT)
  s2opc_validation_test(client_server_events_test toolkit_test_client_events)
endif()


if (NOT WIN32 AND expat_FOUND)
  add_test(NAME "validation::check_file_transfer"
          WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
          COMMAND
          "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
          "--server-wd" $<TARGET_FILE_DIR:toolkit_test_server_file_transfer>
          "--server-cmd" $<TARGET_FILE:toolkit_test_server_file_transfer>
          "./check_file_transfer")
  set_tests_properties("validation::check_file_transfer" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=check_file_transfer.tap")
endif()

# Function to run a validation of the S2OPC server scenario with given client scenario test
function(s2opc_custom_server_validation_test server_target name)
  get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
  set(test_name "validation::${name}")
  add_test(NAME "${test_name}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${server_target}>
    "--server-cmd" "$<TARGET_FILE:${server_target}> ${no_ext_name}"
    "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/interop_tools/${name})
endfunction()

function(s2opc_server_validation_test name)
  s2opc_custom_server_validation_test(${TOOLKIT_TEST_SERVER} ${name})
endfunction()

# Function to run a validation of the given S2OPC client scenario with another server
function(s2opc_client_validation_test name)
  set(test_name "validation::${name}")
  add_test(NAME "${test_name}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" "${CMAKE_CURRENT_SOURCE_DIR}/interop_tools"
    "--server-cmd" "${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/interop_tools/server.py 25000"
    "./${name}")
  set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${name}.tap")
endfunction()

# Function to run a validation of the given S2OPC client scenario with S2OPC server
function(s2opc_client_self_validation_test name)
  set(test_name "validation::${name}")
  add_test(NAME "${test_name}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_TEST_SERVER}>
    "./${name}")
  set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${name}.tap")
endfunction()

s2opc_custom_server_validation_test("toolkit_test_discovery_server" "client.py")
s2opc_server_validation_test("client_sc_renew.py")
s2opc_server_validation_test("client_session_timeout.py")
s2opc_server_validation_test("client_sc_establish_timeout.py")
s2opc_client_validation_test("toolkit_test_suite_client")

# Function to run the secure channels (+ sockets) layer validation test with different key sizes / secu modes
function(s2opc_sc_level_test name server_key_size)
  set(test_name "validation::secure_channel_level::${name}")
  add_test(NAME "${test_name}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:test_secure_channels_server>
    "--server-cmd" "$<TARGET_FILE:test_secure_channels_server> ${server_key_size}"
    "--wait-server"
    $<TARGET_FILE:test_secure_channels_client> ${ARGN})
  set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=secure_channel_level_${name}.tap")
endfunction()

s2opc_sc_level_test("SignAndEncrypt_B256Sha256_2048bit" "2048")
s2opc_sc_level_test("Sign_B256Sha256_2048bit" "2048" "sign")
s2opc_sc_level_test("None" "2048" "none")
s2opc_sc_level_test("Sign_B256_2048bit" "2048" "encrypt" "B256")
s2opc_sc_level_test("SignAndEncrypt_B256Sha256_4096bit" "4096" "encrypt" "B256Sha256" "4096")
s2opc_sc_level_test("SignAndEncrypt_B256Sha256_4096bit_server_vs_2048bit_client" "4096" "encrypt" "B256Sha256" "2048" "4096")
s2opc_sc_level_test("SignAndEncrypt_B256Sha256_2048bit_server_vs_4096bit_client" "2048" "encrypt" "B256Sha256" "4096" "2048")

## S2OPC wrapper client library validation tests ##

# Helpers unit tests
add_executable(check_wrapper "validation_tests/client_wrapper/check_wrapper.c" ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES})
target_include_directories(check_wrapper PRIVATE ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES})
target_compile_options(check_wrapper PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(check_wrapper PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(check_wrapper PRIVATE s2opc_clientserver Check::check)
s2opc_client_validation_test(check_wrapper)

## PyS2OPC library validation tests ##

# The library is built by python. It may use the shared library but it must not be built with ASan.
# The problem with ASan is to embed a dynamic library compiled with ASan into Python, which is not compiled with ASan.
# The compilation is not done with clang-analyzer either, as this would analyze CFFI-generated code.
if(WITH_PYS2OPC)
  set(PYS2OPC_SRC_PATH "${S2OPC_ROOT_PATH}/src/ClientServer/frontend/pys2opc")
  set(PYS2OPC_TEST_PATH "${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/pys2opc")
  # TODO: This variable must be the one defined in ../src/ClientServer/frontend/pys2opc/CMakeLists.txt
  set(PYS2OPC_TEST_INSTALL_PATH "${CMAKE_BINARY_DIR}/pys2opc_install")
  # py_env (PYTHONPATH) get (in order) : pys2opc, common.py|tap_logger.py, wait_server.py, utils.py
  set(py_env "PYTHONPATH=${PYS2OPC_TEST_INSTALL_PATH}${ENV_PATH_SEP}${PYS2OPC_SRC_PATH}\
${ENV_PATH_SEP}${S2OPC_ROOT_PATH}/tests/ClientServer/interop_tools\
${ENV_PATH_SEP}${S2OPC_ROOT_PATH}/tests/ClientServer/scripts\
${ENV_PATH_SEP}${S2OPC_ROOT_PATH}/samples/ClientServer/pys2opc/client"
      "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

  # Validation test
  function(pys2opc_validation_test name with_server)
    get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
    set(test_name "validation::${name}")
    if(with_server)
      add_test(NAME "${test_name}"
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND
        "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
        "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
        "--server-cmd" "$<TARGET_FILE:${TOOLKIT_TEST_SERVER}> ${no_ext_name}"
        "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/validation_tests/pys2opc/${name}"
        )
    else()
      add_test(NAME "${test_name}"
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/validation_tests/pys2opc/${name}"
        )
    endif()
    # ASan requires to be the first dynamically linked library to be loaded. This is done with LD_PRELOAD.
    if(WITH_ASAN OR WITH_UBSAN OR WITH_TSAN)
      set(py_env ${py_env} "LD_PRELOAD=/usr/local/lib64/libasan.so")
    endif()
    # TODO: make the test depends on pys2opc_install
    set_tests_properties(${test_name} PROPERTIES
      ENVIRONMENT "${py_env}")
  endfunction()

  pys2opc_validation_test("pys2opc_client.py" YES)
  pys2opc_validation_test("pys2opc_client_server_disco.py" NO)
  pys2opc_validation_test("pys2opc_server.py" NO)

## Role Permissions validation tests ##
  set(TOOLKIT_DEMO_SERVER "toolkit_demo_server")
  set(TEST_ADDRESS_SPACE_XML_ROLE_PERMISSIONS "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc_role_test.xml")
  set(USERS_TESTS_ROLES_CONFIG_XML  "${S2OPC_ROOT_PATH}/tests/ClientServer/data/config/S2OPC_Users_Tests_Roles_Config.xml")
  set (env_role ${py_env} "TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=${TEST_ADDRESS_SPACE_XML_ROLE_PERMISSIONS};TEST_USERS_XML_CONFIG=${USERS_TESTS_ROLES_CONFIG_XML}")
  add_test(NAME "validation::pys2opc_test_role_permissions.py"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/validation_tests/pys2opc/pys2opc_test_role_permissions.py"
  )
  set_tests_properties("validation::pys2opc_test_role_permissions.py" PROPERTIES ENVIRONMENT "${env_role}")

endif()

## S2OPC fuzzing tests: to be run manually ##

if(ENABLE_FUZZING)
  set(FUZZING_PATH "unit_tests/fuzzing")

  s2opc_fuzzer(server_request_fuzzer "${FUZZING_PATH}/fuzz_server_request.c" s2opc_clientserver "${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}")
  s2opc_fuzzer(parse_tcp_uri_fuzzer "${FUZZING_PATH}/fuzz_parse_tcp_uri.c" s2opc_clientserver "${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}")
  s2opc_fuzzer(decode_fuzzer "${FUZZING_PATH}/fuzz_decoder.c" s2opc_clientserver "${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}")
endif(ENABLE_FUZZING)

### Push server tests ###

# These validations tests will temporaly modify the server PKI folder: S2OPC_Demo_PKI/.
# They use the toolkit_demo_push_server executable, made when compiling the samples.

## Make executables
# Push client, useful for scenarios
add_executable(push_client "validation_tests/client/push_client/push_client.c"
                                                "validation_tests/client/push_client/push_client_connection_helper.c"
                                                "validation_tests/client/push_client/push_server_methods.c"
                                                "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_push_server/push_server_lib/internal/sopc_trustlist_internal_helper.c"
                                                ${S2OPC_TEST_CLIENTSERVER_HELPER_SOURCES}
                                                "unit_tests/helpers/check_crypto_certificates.c")
target_include_directories(push_client PRIVATE "validation_tests/client/push_client"
                                                "unit_tests/helpers"
                                                ${S2OPC_TEST_CLIENTSERVER_HELPER_INCLUDES}
                                                "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_push_server/push_server_lib"
                                                "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_push_server/push_server_lib/internal")
target_link_libraries(push_client PRIVATE s2opc_clientserver Check::check)
target_compile_options(push_client PRIVATE ${S2OPC_COMPILER_FLAGS} -D_GNU_SOURCE) # GNU_SOURCE for fileno function.
target_compile_definitions(push_client PRIVATE ${S2OPC_DEFINITIONS})

## Copy data to build and make client PKI
add_custom_command(
  TARGET push_client POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/push_data
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/client/push_client/data/cacrl_with_revoked_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/push_data
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/selfsigned/ca_selfsigned_pathLen0key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/push_data
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/push_data
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/encrypted_server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/push_data
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cakey.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/push_data)

make_pki(push_client "S2OPC_Demo_PKI_Client")

## Tests.
# Test the different scenarios (with multiple clients)

# Tests are disabled when compiling with Cyclone because some crypto functions useful for the tests are not implemented (CSR functions for example)
if(NOT S2OPC_CRYPTO_CYCLONE AND UNIX AND expat_FOUND AND NOT S2OPC_NANO_PROFILE)
  function(add_push_test_scenario scenario_name python_script_scenario nb_password input_csr)
    set(test_name "validation::${scenario_name}")
    add_test(NAME "${test_name}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" ${nb_password}
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:toolkit_demo_push_server>
    "--server-cmd" $<TARGET_FILE:toolkit_demo_push_server>
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/client/push_client/scenarios/${python_script_scenario}" ${input_csr})
    set_tests_properties("${test_name}" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${scenario_name}.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_Push_AddSpace.xml;TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml")
  endfunction()

  add_push_test_scenario(push_server_self_signed test_tl_self_signed.py 7 "")
  add_push_test_scenario(push_server_revoke test_tl_revoke_client.py 7 "")
  add_push_test_scenario(push_server_untrusted test_tl_untrusted.py 12 "")
  add_push_test_scenario(push_server_trusted_int test_tl_trusted_int.py 8 "")

  ## CSR tests
  # Success UpdateCertificate and 3 invalid CSR
  add_push_test_scenario(push_server_csr_invalid test_csr_invalid.py 5 "")

  # Success scenarios: do only one test? But without losing discriminatory nature.
  # These two tests need to set up the env variable TEST_PASSWORD_CACERT which is the password of the private key of cacert
  # and TEST_PASSWORD_USER_SECUADMIN which is the password of the user "secuAdmin" (because the client user uses a connection by username/password)
  add_push_test_scenario(push_server_csr_valid_noNewKey test_csr_valid.py 6 noNewKey)
  add_push_test_scenario(push_server_csr_valid_newKey test_csr_valid.py 6 newKey)
endif()
