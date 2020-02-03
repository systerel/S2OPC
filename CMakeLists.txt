cmake_minimum_required (VERSION 3.5)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

project (S2OPC LANGUAGES C)

include(CheckCCompilerFlag)

#### S2OPC library and extensions ####

### Dependencies ###

# options with impact on dependencies
option(ENABLE_TESTING "Build the S2OPC tests" ON)
option(FORCE_CHECK_STATIC_LIB "Force the use of static check lib" ON)

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

find_package(Doxygen)

# Keep a static version of the mbedtls libraries to link the tests
set(MBEDTLS_CUSTOM_PATH 0)

if(DEFINED MBEDTLS_LIBRARY)
  set(MBEDTLS_CUSTOM_PATH 1)
endif()

set(MBEDTLS_USE_STATIC_LIBS 1)
find_package(MbedTLS QUIET)
set(MBEDTLS_STATIC_LIBRARIES ${MBEDTLS_LIBRARIES})

# If the path to MbedTLS was not given manually by the user, we need to clear
# those variables so that the second call to find_package below does a new
# search.
if (NOT ${MBEDTLS_CUSTOM_PATH})
  unset(MBEDTLS_LIBRARY CACHE)
  unset(MBEDX509_LIBRARY CACHE)
  unset(MBEDCRYPTO_LIBRARY CACHE)
endif()

# Mandatory dependencies
if (${BUILD_SHARED_LIBS})
  set(MBEDTLS_USE_STATIC_LIBS 0)
else()
  set(MBEDTLS_USE_STATIC_LIBS 1)
endif()

find_package(MbedTLS REQUIRED)

# We always link check statically, so that our test binaries are standalone
if(ENABLE_TESTING)
  if(FORCE_CHECK_STATIC_LIB)
    set(CHECK_USE_STATIC_LIBS 1)
  else()
    set(CHECK_USE_STATIC_LIBS 0)
  endif()
  find_package(Check REQUIRED)
endif()

### Compilation and configuration of S2OPC extension libraries ###


#### Tests / Samples for S2OPC library and extension libraries ####

# Options reserved to tests / samples
option(ENABLE_SAMPLES "Build the S2OPC library and extention libraries samples")
option(WITH_STATIC_SECURITY_DATA "Use static security data for test" OFF)

if(WIN32)
    set(FILE_EXT .exe)
    set(ENV_PATH_SEP "\;")
else()
    set(ENV_PATH_SEP ":")
endif()

### Tests and validation of S2OPC library ###

if(ENABLE_TESTING)
  enable_testing()

  # Function to add a unit test to the set of tests: define a test generating a TAP file from a binary name
  function(s2opc_unit_test binary_name)
    set(test_name "unit::${binary_name}")
    set(test_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${binary_name}${FILE_EXT})

    foreach(arg ${ARGN})
      if("${arg}" STREQUAL "GENERATE_TAP")
        set(test_cmd "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py" "${test_cmd}")
      else()
        message(FATAL_ERROR "Unknown parameter for s2opc_unit_test: ${arg}")
      endif()
    endforeach()

    add_test(NAME "${test_name}" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${test_cmd})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${binary_name}.tap")
  endfunction()

  set(TEST_CONFIG_XML              "${CMAKE_SOURCE_DIR}/tests/data/config/S2OPC_Test_XML_Config.xml")
  set(TEST_SERVER_DEMO_CONFIG_XML  "${CMAKE_SOURCE_DIR}/tests/data/config/S2OPC_Server_Demo_Config.xml")
  set(TEST_SERVER_UACTT_CONFIG_XML "${CMAKE_SOURCE_DIR}/tests/data/config/S2OPC_Server_UACTT_Config.xml")

  if (WITH_NANO_EXTENDED)
    set(TEST_ADDRESS_SPACE_XML "${CMAKE_SOURCE_DIR}/tests/data/address_space/s2opc.xml")
  else()
    set(TEST_ADDRESS_SPACE_XML "${CMAKE_SOURCE_DIR}/tests/data/address_space/s2opc_nano.xml")
  endif()
  set(TEST_CHECK_HELPERS_ADDRESS_SPACE_C "check_helpers_address_space.c")
  set(TEST_DEMO_SERVER_ADDRESS_SPACE_C "demo_server_address_space.c")
  set(TEST_LOCAL_DEMO_SERVER_ADDRESS_SPACE_C "local_demo_server_test_address_space.c")
  set(TEST_DEMO_CLIENT_ADDRESS_SPACE_C "demo_client_address_space.c")
  s2opc_embed_address_space(${TEST_CHECK_HELPERS_ADDRESS_SPACE_C} ${TEST_ADDRESS_SPACE_XML})
  s2opc_embed_address_space(${TEST_DEMO_SERVER_ADDRESS_SPACE_C} ${TEST_ADDRESS_SPACE_XML})
  s2opc_embed_address_space(${TEST_LOCAL_DEMO_SERVER_ADDRESS_SPACE_C} ${TEST_ADDRESS_SPACE_XML})
  s2opc_embed_address_space(${TEST_DEMO_CLIENT_ADDRESS_SPACE_C} ${TEST_ADDRESS_SPACE_XML})

## S2OPC unit tests ##

  # Helpers tests
  file(GLOB_RECURSE INTERNAL_TESTS_SRCS "tests/helpers/*.c")
  add_executable(check_helpers ${INTERNAL_TESTS_SRCS} ${TEST_CHECK_HELPERS_ADDRESS_SPACE_C})
  target_include_directories(check_helpers PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
  target_link_libraries(check_helpers PRIVATE s2opc ${CHECK_LIBRARIES} s2opc_clientserver-loader-embedded)
  target_compile_options(check_helpers PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_helpers PRIVATE ${S2OPC_DEFINITIONS})
  if (EXPAT_FOUND)
    target_link_libraries(check_helpers PRIVATE s2opc_clientserver-xml-loaders-expat)
    target_compile_definitions(check_helpers PRIVATE "WITH_EXPAT")
  endif()
  s2opc_unit_test(check_helpers)

  add_custom_command(
    TARGET check_helpers POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logAnother.ref ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logCircular.ref1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logCircular.ref2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logTest.ref1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logTest.ref2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logTest.ref3 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_SERVER_DEMO_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_SERVER_UACTT_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_ADDRESS_SPACE_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  # Sockets layer tests
  add_executable(check_sockets "tests/sockets/check_sockets.c")
  target_include_directories(check_sockets PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                   ${S2OPC_PRIVATE_INCLUDES}
                                                   ${CHECK_INCLUDE_DIRS})
  target_compile_options(check_sockets PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_sockets PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(check_sockets PRIVATE ${CHECK_LIBRARIES} s2opc)
  s2opc_unit_test(check_sockets)


  # Secure channels (stubbed on services side) tests
  file(GLOB_RECURSE SECURE_CHANNELS_SRC
    "csrc/configuration/sopc_ieee_check.c"
    "csrc/secure_channels/*.c"
    "csrc/opcua_types/*.c"
    "csrc/crypto/*.c"
    "csrc/helpers/*.c"
    ${PLATFORM_SRCS})

  # Secure channels tests: tests on received buffer
  file(GLOB_RECURSE SECURE_CHANNELS_BUFFER_TEST
       ${PLATFORM_SRCS}
       "csrc/helpers/*.c"
       "tests/secure_channels/event_recorder.c"
       "tests/secure_channels/check_sc_rcv_buffer.c"
       "tests/secure_channels/check_sc_rcv_helpers.c"
       "tests/secure_channels/event_helpers.c"
       "tests/helpers/hexlify.c"
       "tests/secure_channels/stubs/*.c")

  add_executable(check_sc_rcv_buffer ${SECURE_CHANNELS_SRC}
                                     ${SECURE_CHANNELS_BUFFER_TEST})
  target_link_libraries(check_sc_rcv_buffer PRIVATE ${CHECK_LIBRARIES} ${S2OPC_LINKER_FLAGS} ${S2OPC_LINK_LIBRARIES})
  target_compile_options(check_sc_rcv_buffer PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_sc_rcv_buffer PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(check_sc_rcv_buffer PRIVATE "tests/secure_channels/stubs"
                                                         "tests/secure_channels"
                                                         "tests/helpers"
                                                         ${S2OPC_PUBLIC_INCLUDES}
                                                         ${S2OPC_PRIVATE_INCLUDES}
                                                         ${MBEDTLS_INCLUDE_DIRS}
                                                         ${CHECK_INCLUDE_DIRS})
  s2opc_unit_test(check_sc_rcv_buffer)

  # Secure channels tests: tests on received encrypted buffer
  file(GLOB_RECURSE SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST
       ${PLATFORM_SRCS}
       "csrc/helpers/*.c"
       "tests/secure_channels/event_recorder.c"
       "tests/secure_channels/check_sc_rcv_helpers.c"
       "tests/secure_channels/check_sc_rcv_encrypted_buffer.c"
       "tests/secure_channels/event_helpers.c"
       "tests/helpers/hexlify.c"
       "tests/secure_channels/stubs/*.c")

  add_executable(check_sc_rcv_encrypted_buffer ${SECURE_CHANNELS_SRC}
                                               ${SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST})
  target_link_libraries(check_sc_rcv_encrypted_buffer PRIVATE ${CHECK_LIBRARIES} ${S2OPC_LINKER_FLAGS} ${S2OPC_LINK_LIBRARIES})
  target_compile_options(check_sc_rcv_encrypted_buffer PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_sc_rcv_encrypted_buffer PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(check_sc_rcv_encrypted_buffer PRIVATE "tests/secure_channels/stubs"
                                                                   "tests/secure_channels"
                                                                   "tests/helpers"
                                                                   ${S2OPC_PUBLIC_INCLUDES}
                                                                   ${S2OPC_PRIVATE_INCLUDES}
                                                                   ${MBEDTLS_INCLUDE_DIRS}
                                                                   ${CHECK_INCLUDE_DIRS})
  add_custom_command(
    TARGET check_sc_rcv_encrypted_buffer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/check_sc_test/check_sc_client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/check_sc_test/check_sc_client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/check_sc_test/check_sc_server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_sc_rcv_encrypted_buffer_data/)
  s2opc_unit_test(check_sc_rcv_encrypted_buffer)

  # Server running local services
  add_executable(toolkit_test_server_local_service "tests/services/toolkit_test_server_local_service.c"
                                                      "tests/services/wrap_read.c"
                                                      "tests/services/testlib_write.c"
                                                      "tests/services/test_results.c"
                                                      "tests/services/testlib_read_response.c"
                                                      ${TEST_LOCAL_DEMO_SERVER_ADDRESS_SPACE_C})
  target_link_libraries(toolkit_test_server_local_service PRIVATE s2opc s2opc_clientserver-loader-embedded)
  target_compile_options(toolkit_test_server_local_service PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_server_local_service PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(toolkit_test_server_local_service PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                                        ${S2OPC_PRIVATE_INCLUDES})
  s2opc_unit_test(toolkit_test_server_local_service GENERATE_TAP)

## S2OPC validation tests ##

# Tests compilation definition #

  file(GLOB_RECURSE SECURE_CHANNELS_CLIENT_TEST
    "csrc/sockets/*.c"
    "tests/secure_channels/event_helpers.c"
    "tests/secure_channels/event_recorder.c"
    "tests/secure_channels/test_secure_channels_client.c"
    "tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c"
    "tests/secure_channels/stubs/stub_sc_sopc_services_api.c")

  file(GLOB_RECURSE SECURE_CHANNELS_SERVER_TEST
    "csrc/sockets/*.c"
    "tests/secure_channels/event_helpers.c"
    "tests/secure_channels/event_recorder.c"
    "tests/secure_channels/test_secure_channels_server.c"
    "tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c"
    "tests/secure_channels/stubs/stub_sc_sopc_services_api.c")

  # Secure channels (+ Sockets) layer tests: CLIENT executable
  add_executable(test_secure_channels_client ${SOCKETS_SRC}
                                             ${SECURE_CHANNELS_SRC}
                                             ${SECURE_CHANNELS_CLIENT_TEST})

  target_include_directories(test_secure_channels_client PRIVATE "tests/secure_channels/stubs"
                                                                 ${S2OPC_PUBLIC_INCLUDES}
                                                                 ${S2OPC_PRIVATE_INCLUDES}
                                                                 ${MBEDTLS_INCLUDE_DIRS})
  target_compile_options(test_secure_channels_client PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(test_secure_channels_client PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(test_secure_channels_client PRIVATE ${S2OPC_LINKER_FLAGS} ${S2OPC_LINK_LIBRARIES})

  # Secure channels (+ Sockets) layer tests: SERVER executable
  add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                             ${SECURE_CHANNELS_SRC}
                                             ${SECURE_CHANNELS_SERVER_TEST})
  target_link_libraries(test_secure_channels_server PRIVATE ${S2OPC_LINKER_FLAGS} ${S2OPC_LINK_LIBRARIES})
  target_compile_options(test_secure_channels_server PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(test_secure_channels_server PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(test_secure_channels_server PRIVATE "tests/secure_channels/stubs"
                                                                 ${S2OPC_PUBLIC_INCLUDES}
                                                                 ${S2OPC_PRIVATE_INCLUDES}
                                                                 ${MBEDTLS_INCLUDE_DIRS})

  # Client scenario
  add_executable(toolkit_test_client "tests/services/toolkit_test_client.c"
                                     "tests/services/wrap_read.c"
                                     "tests/services/testlib_write.c"
                                     "tests/services/test_results.c"
                                     "tests/services/testlib_read_response.c"
                                     ${TEST_DEMO_CLIENT_ADDRESS_SPACE_C})
  target_link_libraries(toolkit_test_client PRIVATE s2opc s2opc_clientserver-loader-embedded)
  target_compile_options(toolkit_test_client PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_client PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(toolkit_test_client PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                         ${S2OPC_PRIVATE_INCLUDES})

  # Client scenario with service faults
  add_executable(toolkit_test_client_service_faults "tests/services/toolkit_test_client_service_faults.c")
  target_compile_options(toolkit_test_client_service_faults PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_test_client_service_faults PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(toolkit_test_client_service_faults PRIVATE s2opc)
  target_include_directories(toolkit_test_client_service_faults PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                                        ${S2OPC_PRIVATE_INCLUDES})

  add_executable(toolkit_test_suite_client "tests/demo/test_suite_client.c"
                                           "tests/demo/test_discovery.c"
                                           "tests/demo/config.c"
                                           "tests/demo/state_machine.c"
                                           "tests/demo/wait_machines.c"
                                           "tests/demo/test_session.c")
  target_link_libraries(toolkit_test_suite_client PRIVATE s2opc ${CHECK_LIBRARIES})
  target_include_directories(toolkit_test_suite_client PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                               ${S2OPC_PRIVATE_INCLUDES}
                                                               ${CHECK_INCLUDE_DIRS})
  # Server scenario (configured for UACTT)

  # Manage the Nano profiles extended mode
  if (WITH_NANO_EXTENDED)
    set(TOOLKIT_TEST_SERVER "toolkit_test_server")
  else()
    set(TOOLKIT_TEST_SERVER "toolkit_test_nano_server")
  endif()

  add_executable(${TOOLKIT_TEST_SERVER}
                 "tests/services/runtime_variables.c"
                 "tests/services/toolkit_test_server.c"
                 ${TEST_DEMO_SERVER_ADDRESS_SPACE_C})
  target_link_libraries(${TOOLKIT_TEST_SERVER} PRIVATE s2opc s2opc_clientserver-loader-embedded)
  target_compile_options(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                            ${S2OPC_PRIVATE_INCLUDES})

  if (EXPAT_FOUND)
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
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add 4096 client / server keys
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add UaCTT CAs / CRLs
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_ca2T.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/crl/revocation_list_ctt_ca1T.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/crl/revocation_list_ctt_ca1I_ca2T.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ctt_ca1IC.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ctt_ca1I.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/issuers/certs/ctt_ca1TC_ca2I.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/issuers/crl/revocation_list_ctt_ca1I.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/issuers/crl/revocation_list_ctt_ca1TC_ca2I.crl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTE.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha1_1024.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha1_2048.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha256_2048.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSha256_4096.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSincorrect.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSip.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTSuri.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_appTV.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_ca2T_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1I_ca2T_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1IC_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1IC_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_ca2U_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1T_ca2U_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_ca2I_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1TC_ca2I_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1U_appT.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/uactt/ApplicationInstance_PKI/trusted/certs/ctt_ca1U_appTR.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
    )
  
# Tests run definition #
  
  add_test(NAME "validation::client_server_test"
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           COMMAND
           "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py"
           "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
           "--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           "--server-cmd" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TOOLKIT_TEST_SERVER}${FILE_EXT}"
           "./toolkit_test_client${FILE_EXT}")
  set_tests_properties("validation::client_server_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=client_server_test.tap")
  
  add_test(NAME "validation::client_service_faults_test"
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           COMMAND
           "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py"
           "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
           "--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           "--server-cmd" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TOOLKIT_TEST_SERVER}${FILE_EXT}"
           "./toolkit_test_client_service_faults${FILE_EXT}")
  set_tests_properties("validation::client_service_faults_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=client_service_faults_test.tap")

  # Function to run a validation of the S2OPC server scenario with given client scenario test
  function(s2opc_server_validation_test name)
    get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
    set(test_name "validation::${name}")
    add_test(NAME "${test_name}"
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/validation
             COMMAND
             "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
             "--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
             "--server-cmd" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TOOLKIT_TEST_SERVER}${FILE_EXT} ${no_ext_name}"
             "${PYTHON_EXECUTABLE}" ${name})
  endfunction()

  # Function to run a validation of the given S2OPC client scenario with another server
  function(s2opc_client_validation_test name)
    set(test_name "validation::${name}")
    add_test(NAME "${test_name}"
             WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
             COMMAND
             "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
             "--server-wd" "${CMAKE_SOURCE_DIR}/validation"
             "--server-cmd" "${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/validation/server.py 25000"
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
             "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py"
             "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
             "--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
             "--server-cmd" "./test_secure_channels_server ${server_key_size}"
             "--wait-server"
             "./test_secure_channels_client${FILE_EXT}" ${ARGN})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=secure_channel_level_${name}.tap")
  endfunction()

  s2opc_sc_level_test("SignAndEncrypt_B256Sha256_2048bit" "2048")
  s2opc_sc_level_test("Sign_B256Sha256_2048bit" "2048" "sign")
  s2opc_sc_level_test("None" "2048" "none")
  s2opc_sc_level_test("Sign_B256_2048bit" "2048" "encrypt" "B256")
  s2opc_sc_level_test("SignAndEncrypt_B256Sha256_4096bit" "4096" "encrypt" "B256Sha256" "4096")
  s2opc_sc_level_test("SignAndEncrypt_B256Sha256_4096bit_server_vs_2048bit_client" "4096" "encrypt" "B256Sha256" "2048" "4096")
  s2opc_sc_level_test("SignAndEncrypt_B256Sha256_2048bit_server_vs_4096bit_client" "2048" "encrypt" "B256Sha256" "4096" "2048")

## S2OPC fuzzing tests: to be run manually ##
  
  set(FUZZING_PATH "tests/fuzzing")

  # Local Fuzzing
  function(s2opc_fuzzer name src_file)
    set(standalone_target_name "${name}.standalone")
    set(fuzz_target_name "${name}.libfuzzer")

    add_executable(${standalone_target_name} "${FUZZING_PATH}/${src_file}" "tests/fuzzing/standalone_fuzzer.c")
    target_include_directories(${standalone_target_name} PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES})
    target_compile_options(${standalone_target_name} PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(${standalone_target_name} PRIVATE ${S2OPC_DEFINITIONS})
    target_link_libraries(${standalone_target_name} s2opc)
    add_dependencies(fuzzers ${standalone_target_name})

    add_executable(${fuzz_target_name} "${FUZZING_PATH}/${src_file}")
    set_target_properties(${fuzz_target_name} PROPERTIES COMPILE_FLAGS "-fsanitize=fuzzer")
    set_target_properties(${fuzz_target_name} PROPERTIES LINK_FLAGS "-fsanitize=fuzzer")
    target_include_directories(${fuzz_target_name} PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES})
    target_link_libraries(${fuzz_target_name} s2opc)
    add_dependencies(fuzzers ${fuzz_target_name})
  endfunction()

  if (NOT WITH_OSS_FUZZ AND "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS "6.0.0")
      # We need at least CLang 6 to get LibFuzzer
      message(STATUS "Your version of Clang (${CMAKE_C_COMPILER_ID}) is not recent enough to have LibFuzzer")
    else()
      add_custom_target(fuzzers)
      s2opc_fuzzer(server_request_fuzzer "fuzz_server_request.c")
      s2opc_fuzzer(parse_tcp_uri_fuzzer "fuzz_parse_tcp_uri.c")
      s2opc_fuzzer(decode_fuzzer "fuzz_decoder.c")
    endif()
  endif()

  # OSS Fuzzing
  function(s2opc_oss_fuzzer target_name src_file)
    add_executable(${target_name} "${FUZZING_PATH}/${src_file}")
    target_include_directories(${target_name} PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES})
    target_link_libraries(${target_name} s2opc $ENV{LIB_FUZZING_ENGINE})
    add_dependencies(fuzzers ${target_name})
  endfunction()

  if(WITH_OSS_FUZZ)
    add_custom_target(fuzzers)
    s2opc_oss_fuzzer(server_request_fuzzer "fuzz_server_request.c")
    s2opc_oss_fuzzer(parse_tcp_uri_fuzzer "fuzz_parse_tcp_uri.c")
    s2opc_oss_fuzzer(decode_fuzzer "fuzz_decoder.c")
  endif()

## S2OPC subscription client library validation tests ##

  # Helpers unit tests
  add_executable(check_libsub "${LIBSUBSCRIBE_PATH}/check_libsub.c")
  target_include_directories(check_libsub PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
  target_compile_options(check_libsub PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_libsub PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(check_libsub PRIVATE client_subscription ${CHECK_LIBRARIES})
  s2opc_client_validation_test(check_libsub)

  add_executable(check_wrapper "${LIBSUBSCRIBE_PATH}/check_wrapper.c")
  target_include_directories(check_wrapper PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
  target_compile_options(check_wrapper PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(check_wrapper PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(check_wrapper PRIVATE client_subscription ${CHECK_LIBRARIES})
  s2opc_client_validation_test(check_wrapper)

## PyS2OPC library validation tests ##

  # The library is built by python. It may use the shared library but it must not be built with ASan.
  # The problem with ASan is to embed a dynamic library compiled with ASan into Python, which is not compiled with ASan.
  # The compilation is not done with clang-analyzer either, as this would analyze CFFI-generated code.
  if(WITH_PYS2OPC)
    # Validation test
    function(pys2opc_validation_test name)
      get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
      set(test_name "validation::${name}")
      add_test(NAME "${test_name}"
        WORKING_DIRECTORY "${PYS2OPC_PATH}/tests"
        COMMAND
        "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
        "--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        "--server-cmd" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TOOLKIT_TEST_SERVER}${FILE_EXT} ${no_ext_name}"
        "${PYTHON_EXECUTABLE}" "${name}"
        )
      # ASan requires to be the first dynamically linked library to be loaded. This is done with LD_PRELOAD.
      set(new_env "PYTHONPATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${ENV_PATH_SEP}${PYS2OPC_PATH}"
        "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
      if(WITH_ASAN OR WITH_UBSAN OR WITH_TSAN)
        set(new_env ${new_env} "LD_PRELOAD=/usr/local/lib64/libasan.so")
      endif()
      set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT "${new_env}")
    endfunction()

    pys2opc_validation_test("pys2opc_client.py")
  endif()
  
endif(ENABLE_TESTING)

### Samples of S2OPC library and extension libraries use ###

if(ENABLE_SAMPLES)

  # Demo clients (S2OPC only without extension)
  add_executable(s2opc_browse "tests/demo/s2opc_browse.c"
                              "tests/demo/state_machine.c"
                              "tests/demo/config.c")
  target_link_libraries(s2opc_browse PRIVATE s2opc)
  target_compile_options(s2opc_browse PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_browse PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(s2opc_browse PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                  ${S2OPC_PRIVATE_INCLUDES})

  add_executable(s2opc_read "tests/demo/s2opc_read.c"
                            "tests/demo/state_machine.c"
                            "tests/demo/config.c")
  target_link_libraries(s2opc_read PRIVATE s2opc)
  target_compile_options(s2opc_read PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_read PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(s2opc_read PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                ${S2OPC_PRIVATE_INCLUDES})

  add_executable(s2opc_write "tests/demo/s2opc_write.c"
                             "tests/demo/state_machine.c"
                             "tests/demo/config.c")
  target_link_libraries(s2opc_write PRIVATE s2opc)
  target_compile_options(s2opc_write PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_write PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(s2opc_write PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                 ${S2OPC_PRIVATE_INCLUDES})

  add_executable(s2opc_discovery "tests/demo/s2opc_discovery.c"
                                 "tests/demo/state_machine.c"
                                 "tests/demo/config.c")
  target_link_libraries(s2opc_discovery PRIVATE s2opc)
  target_compile_options(s2opc_discovery PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_discovery PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(s2opc_discovery PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                     ${S2OPC_PRIVATE_INCLUDES})

  add_executable(s2opc_findserver "tests/demo/s2opc_findserver.c"
                                  "tests/demo/state_machine.c"
                                  "tests/demo/config.c")
  target_link_libraries(s2opc_findserver PRIVATE s2opc)
  target_compile_options(s2opc_findserver PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_findserver PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(s2opc_findserver PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                      ${S2OPC_PRIVATE_INCLUDES})

  add_executable(s2opc_register "tests/demo/s2opc_register.c"
                                "tests/demo/state_machine.c"
                                "tests/demo/config.c")
  target_link_libraries(s2opc_register PRIVATE s2opc)
  target_compile_options(s2opc_register PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_register PRIVATE ${S2OPC_DEFINITIONS}) 
  target_include_directories(s2opc_register PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                    ${S2OPC_PRIVATE_INCLUDES})

  # Samples with S2OPC subscribing client library

  # s2opc_subscription_client is not built on Windows (use getopt GNU library to parse command line)
  if(NOT WIN32)
    # Demo client
    add_executable(s2opc_subscription_client "${LIBSUBSCRIBE_PATH}/client.c")
    target_compile_options(s2opc_subscription_client PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(s2opc_subscription_client PRIVATE ${S2OPC_DEFINITIONS})
    target_link_libraries(s2opc_subscription_client PRIVATE client_subscription)
  endif(NOT WIN32)

  # Samples with S2OPC client wrapper: TODO: remove dependency on client_subscription
  add_executable(example_wrapper_write "${LIBSUBSCRIBE_PATH}/examples/write.c")
  target_include_directories(example_wrapper_write PRIVATE ${LIBSUBSCRIBE_PATH})
  target_link_libraries(example_wrapper_write PRIVATE client_subscription s2opc)
  target_compile_options(example_wrapper_write PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_write PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_read "${LIBSUBSCRIBE_PATH}/examples/read.c")
  target_include_directories(example_wrapper_read PRIVATE ${LIBSUBSCRIBE_PATH})
  target_link_libraries(example_wrapper_read PRIVATE client_subscription s2opc)
  target_compile_options(example_wrapper_read PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_read PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_browse "${LIBSUBSCRIBE_PATH}/examples/browse.c")
  target_include_directories(example_wrapper_browse PRIVATE ${LIBSUBSCRIBE_PATH})
  target_link_libraries(example_wrapper_browse PRIVATE client_subscription s2opc)
  target_compile_options(example_wrapper_browse PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_browse PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_get_endpoints "${LIBSUBSCRIBE_PATH}/examples/get_endpoints.c")
  target_include_directories(example_wrapper_get_endpoints PRIVATE ${LIBSUBSCRIBE_PATH})
  target_link_libraries(example_wrapper_get_endpoints PRIVATE client_subscription s2opc)
  target_compile_options(example_wrapper_get_endpoints PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_get_endpoints PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_subscribe "${LIBSUBSCRIBE_PATH}/examples/subscribe.c")
  target_include_directories(example_wrapper_subscribe PRIVATE ${LIBSUBSCRIBE_PATH})
  target_link_libraries(example_wrapper_subscribe PRIVATE client_subscription s2opc)
  target_compile_options(example_wrapper_subscribe PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_subscribe PRIVATE ${S2OPC_DEFINITIONS})

  # Benchmark tool demo
  add_executable(bench_tool "tests/benchmarks/bench_tool.c")
  target_link_libraries(bench_tool PRIVATE s2opc)
  target_compile_options(bench_tool PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(bench_tool PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(bench_tool PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES})

  # TODO: XML parsing demo: make a unit test / validation test with it instead of demo
  if (EXPAT_FOUND)
    add_executable(s2opc_parse_uanodeset "tests/demo/s2opc_parse_uanodeset.c")
    target_link_libraries(s2opc_parse_uanodeset PRIVATE s2opc s2opc_clientserver-xml-loaders-expat)
    target_compile_options(s2opc_parse_uanodeset PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(s2opc_parse_uanodeset PRIVATE ${S2OPC_DEFINITIONS})
    target_include_directories(s2opc_parse_uanodeset PRIVATE ${S2OPC_PUBLIC_INCLUDES})
  endif()

endif(ENABLE_SAMPLES)

#### S2OPC code documentation ####

if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E env DOXYGEN_SOURCE_PREFIX=${CMAKE_SOURCE_DIR}/ ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxygen/s2opc-toolkit.doxyfile
        COMMENT "Generating API documentation"
        VERBATIM
    )
endif()
