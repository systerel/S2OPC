cmake_minimum_required (VERSION 3.5)

project (S2OPC_ClientServer_Tests LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC client/server tests ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and tests separately

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

# We always link check statically, so that our test binaries are standalone
if(ENABLE_TESTING)
  find_package(check REQUIRED)
endif()

#### Tests for S2OPC library and extension libraries ####

if(WIN32)
    set(ENV_PATH_SEP "\;")
else()
    set(ENV_PATH_SEP ":")
endif()

### Tests and validation of S2OPC library ###

# Get S2OPC necessary internal header for unit and validation tests
set(S2OPC_CLIENTSERVER_INTERNAL_INCLUDES
  "${S2OPC_ROOT_PATH}/src/ClientServer/api_toolkit"
  "${S2OPC_ROOT_PATH}/src/ClientServer/configuration"
  "${S2OPC_ROOT_PATH}/src/ClientServer/secure_channels"
  "${S2OPC_ROOT_PATH}/src/ClientServer/services"
  "${S2OPC_ROOT_PATH}/src/ClientServer/services/b2c"
  "${S2OPC_ROOT_PATH}/src/ClientServer/services/bgenc"
  "${S2OPC_ROOT_PATH}/src/ClientServer/sockets"
  )

if(ENABLE_TESTING)
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

  set(TEST_LIGHT_ADDRESS_SPACE_XML "${S2OPC_ROOT_PATH}/tests/ClientServer/data/address_space/s2opc_no_base_info.xml")

  if (WITH_NANO_EXTENDED)
    set(TEST_ADDRESS_SPACE_XML_ORIGIN "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc.xml")
  else()
    set(TEST_ADDRESS_SPACE_XML_ORIGIN "${S2OPC_ROOT_PATH}/samples/ClientServer/data/address_space/s2opc_nano.xml")
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

  file(GLOB INTERNAL_TESTS_SRCS "unit_tests/helpers/*.c")
  add_executable(check_helpers ${INTERNAL_TESTS_SRCS} ${TEST_SERVER_ADDRESS_SPACE_C})
  add_dependencies(check_helpers make-server-address-space)
  target_include_directories(check_helpers PRIVATE ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
  target_link_libraries(check_helpers PRIVATE Check::check s2opc_clientserver s2opc_clientserver-loader-embedded)
  target_compile_options(check_helpers PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_helpers PRIVATE ${S2OPC_DEFINITIONS})
  if(expat_FOUND)
    target_link_libraries(check_helpers PRIVATE s2opc_clientserver-xml-loaders-expat)
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
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_ADDRESS_SPACE_XML_ORIGIN} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_ADDRESS_SPACE_XML_FILE})

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
       "unit_tests/helpers/hexlify.c"
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
       "unit_tests/helpers/hexlify.c"
       "unit_tests/secure_channels/stubs/*.c")

  add_executable(check_sc_rcv_encrypted_buffer ${SECURE_CHANNELS_SRC}
                                               ${SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST})
  target_link_libraries(check_sc_rcv_encrypted_buffer PRIVATE Check::check s2opc_common)
  target_compile_options(check_sc_rcv_encrypted_buffer PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_sc_rcv_encrypted_buffer PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(check_sc_rcv_encrypted_buffer PRIVATE "unit_tests/secure_channels/stubs"
                                                                   "unit_tests/secure_channels"
                                                                   "unit_tests/helpers"
                                                                   ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
  add_custom_command(
    TARGET check_sc_rcv_encrypted_buffer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/check_sc_test/check_sc_client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/
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
                                             ${SECURE_CHANNELS_CLIENT_TEST})

  target_include_directories(test_secure_channels_client PRIVATE "unit_tests/secure_channels/stubs"
                                                                 ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
  target_compile_options(test_secure_channels_client PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(test_secure_channels_client PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(test_secure_channels_client PRIVATE s2opc_common)

  # Secure channels (+ Sockets) layer tests: SERVER executable
  add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                             ${SECURE_CHANNELS_SRC}
                                             ${SECURE_CHANNELS_SERVER_TEST})
  target_link_libraries(test_secure_channels_server PRIVATE s2opc_common)
  target_compile_options(test_secure_channels_server PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(test_secure_channels_server PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(test_secure_channels_server PRIVATE "unit_tests/secure_channels/stubs"
                                                                 ${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES})
## S2OPC validation tests ##

# Tests compilation definition #


  # Client/Server test
  add_executable(toolkit_test_server_client
      "validation_tests/client/toolkit_test_server_client.c"
      ${TEST_LIGHT_ADDRESS_SPACE_C})
  add_dependencies(toolkit_test_server_client make-light-address-space)
  target_link_libraries(toolkit_test_server_client PRIVATE
    s2opc_serverwrapper
    s2opc_clientwrapper
    s2opc_clientserver-loader-embedded
    Check::check)
  target_compile_options(toolkit_test_server_client PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_server_client PRIVATE ${S2OPC_DEFINITIONS})
  s2opc_unit_test(toolkit_test_server_client)

  # Server running local services
  add_executable(toolkit_test_server_local_service "validation_tests/server/toolkit_test_server_local_service.c"
                                                   "validation_tests/client/wrap_read.c"
                                                   "validation_tests/client/testlib_write.c"
                                                   "validation_tests/client/test_results.c"
                                                   "validation_tests/client/testlib_read_response.c"
                                                   ${TEST_LIGHT_ADDRESS_SPACE_C})
  add_dependencies(toolkit_test_server_local_service make-light-address-space)
  target_include_directories(toolkit_test_server_local_service PRIVATE "validation_tests/client")
  target_link_libraries(toolkit_test_server_local_service PRIVATE s2opc_serverwrapper s2opc_clientserver-loader-embedded)
  target_compile_options(toolkit_test_server_local_service PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_server_local_service PRIVATE ${S2OPC_DEFINITIONS})
  s2opc_unit_test(toolkit_test_server_local_service GENERATE_TAP)

  # Client scenario
  add_executable(toolkit_test_client "validation_tests/client/toolkit_test_client.c"
                                     "validation_tests/client/wrap_read.c"
                                     "validation_tests/client/testlib_write.c"
                                     "validation_tests/client/test_results.c"
                                     "validation_tests/client/testlib_read_response.c"
                                     ${TEST_LIGHT_ADDRESS_SPACE_C})
  add_dependencies(toolkit_test_server_local_service make-light-address-space)
  target_link_libraries(toolkit_test_client PRIVATE s2opc_serverwrapper s2opc_clientserver-loader-embedded)
  target_compile_options(toolkit_test_client PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_client PRIVATE ${S2OPC_DEFINITIONS})

  # Client scenario with service faults
  add_executable(toolkit_test_client_service_faults "validation_tests/client/toolkit_test_client_service_faults.c")
  target_compile_options(toolkit_test_client_service_faults PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_client_service_faults PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(toolkit_test_client_service_faults PRIVATE s2opc_clientserver)

  # Client services scenarii

  # Reuse code of demo client
  set(S2OPC_CLIENTDEMO_INCLUDES ${S2OPC_ROOT_PATH}/samples/ClientServer/demo_client)

  add_executable(toolkit_test_suite_client "validation_tests/client/test_suite_client.c"
                                           "validation_tests/client/test_discovery.c"
                                           "validation_tests/client/wait_machines.c"
                                           "validation_tests/client/test_session.c"
                                           # Use code common with samples
                                           "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_client/config.c"
                                           "${S2OPC_ROOT_PATH}/samples/ClientServer/demo_client/state_machine.c")
  target_link_libraries(toolkit_test_suite_client PRIVATE s2opc_clientserver Check::check)
  target_include_directories(toolkit_test_suite_client PRIVATE ${S2OPC_CLIENTDEMO_INCLUDES})
  target_compile_options(toolkit_test_suite_client PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_suite_client PRIVATE ${S2OPC_DEFINITIONS})

  # Server scenario (configured for UACTT)

  # Manage the Nano profiles extended mode
  if (WITH_NANO_EXTENDED)
    set(TOOLKIT_TEST_SERVER "toolkit_test_server")
  else()
    set(TOOLKIT_TEST_SERVER "toolkit_test_nano_server")
  endif()

  add_executable(${TOOLKIT_TEST_SERVER}
                 "${S2OPC_ROOT_PATH}/tests/ClientServer/validation_tests/server/toolkit_test_server.c"
                 ${TEST_SERVER_ADDRESS_SPACE_C})
  add_dependencies(toolkit_test_server_local_service make-server-address-space)
  target_link_libraries(${TOOLKIT_TEST_SERVER} PRIVATE s2opc_serverwrapper s2opc_clientserver-loader-embedded)
  target_compile_options(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_DEFINITIONS})
  if (expat_FOUND)
    target_link_libraries(${TOOLKIT_TEST_SERVER} PRIVATE s2opc_clientserver-xml-loaders-expat)
    target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_EXPAT")
  endif()

  if (WITH_STATIC_SECURITY_DATA)
    target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_STATIC_SECURITY_DATA")
  endif()

  add_custom_command(
    TARGET ${TOOLKIT_TEST_SERVER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add 4096 client / server keys
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/client_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add UaCTT CAs / CRLs
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_ca2T.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/crl/ctt_ca1T.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/crl/ctt_ca1I_ca2T.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ctt_ca1IC.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ctt_ca1I.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ctt_ca1TC_ca2I.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/crl/ctt_ca1I.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/issuers/crl/ctt_ca1TC_ca2I.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTE.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha1_1024.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha1_2048.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha256_2048.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha256_4096.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSincorrect.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSip.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSuri.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTV.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_ca2T_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_ca2T_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1IC_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1IC_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_ca2U_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_ca2U_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_ca2I_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_ca2I_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1U_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1U_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    )

# Tests run definition #

  #Note: use absolute path for scripts to allow ctest to be run from another another path

  add_test(NAME "validation::client_server_test"
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           COMMAND
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
           "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
           "--server-cmd" $<TARGET_FILE:${TOOLKIT_TEST_SERVER}>
           $<TARGET_FILE:toolkit_test_client>)
  set_tests_properties("validation::client_server_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=client_server_test.tap")

  add_test(NAME "validation::client_service_faults_test"
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           COMMAND
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
           "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
           "--server-cmd" $<TARGET_FILE:${TOOLKIT_TEST_SERVER}>
           $<TARGET_FILE:toolkit_test_client_service_faults>)
  set_tests_properties("validation::client_service_faults_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=client_service_faults_test.tap")

  # Function to run a validation of the S2OPC server scenario with given client scenario test
  function(s2opc_server_validation_test name)
    get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
    set(test_name "validation::${name}")
    add_test(NAME "${test_name}"
             WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
             COMMAND
             "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
             "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
             "--server-cmd" "$<TARGET_FILE:${TOOLKIT_TEST_SERVER}> ${no_ext_name}"
             "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/interop_tools/${name})
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

  s2opc_server_validation_test("client.py")
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

## S2OPC subscription client library validation tests ##

  # Helpers unit tests
  add_executable(check_libsub "validation_tests/client_wrapper/check_libsub.c")
  target_compile_options(check_libsub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_libsub PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(check_libsub PRIVATE s2opc_clientwrapper Check::check)
  s2opc_client_validation_test(check_libsub)

  add_executable(check_wrapper "validation_tests/client_wrapper/check_wrapper.c")
  target_compile_options(check_wrapper PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_wrapper PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(check_wrapper PRIVATE s2opc_clientwrapper Check::check)
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

    # Validation test
    function(pys2opc_validation_test name)
      get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
      set(test_name "validation::${name}")
      add_test(NAME "${test_name}"
        WORKING_DIRECTORY "${PYS2OPC_TEST_PATH}"
        COMMAND
        "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
        "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_TEST_SERVER}>
        "--server-cmd" "$<TARGET_FILE:${TOOLKIT_TEST_SERVER}> ${no_ext_name}"
        "${PYTHON_EXECUTABLE}" "${name}"
        )
      # ASan requires to be the first dynamically linked library to be loaded. This is done with LD_PRELOAD.
      set(new_env "PYTHONPATH=${PYS2OPC_TEST_INSTALL_PATH}${ENV_PATH_SEP}${PYS2OPC_SRC_PATH}${ENV_PATH_SEP}${S2OPC_ROOT_PATH}/tests/ClientServer/interop_tools"
        "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
      if(WITH_ASAN OR WITH_UBSAN OR WITH_TSAN)
        set(new_env ${new_env} "LD_PRELOAD=/usr/local/lib64/libasan.so")
      endif()
      # TODO: make the test depends on pys2opc_install
      set_tests_properties(${test_name} PROPERTIES
                           ENVIRONMENT "${new_env}")
    endfunction()

    pys2opc_validation_test("pys2opc_client.py")
  endif()

endif(ENABLE_TESTING)


## S2OPC fuzzing tests: to be run manually ##

if(ENABLE_FUZZING)
  set(FUZZING_PATH "unit_tests/fuzzing")

  s2opc_fuzzer(server_request_fuzzer "${FUZZING_PATH}/fuzz_server_request.c" s2opc_clientserver "${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}")
  s2opc_fuzzer(parse_tcp_uri_fuzzer "${FUZZING_PATH}/fuzz_parse_tcp_uri.c" s2opc_clientserver "${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}")
  s2opc_fuzzer(decode_fuzzer "${FUZZING_PATH}/fuzz_decoder.c" s2opc_clientserver "${S2OPC_CLIENTSERVER_INTERNAL_INCLUDES}")
endif(ENABLE_FUZZING)
