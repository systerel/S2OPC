cmake_minimum_required (VERSION 3.5)

project (S2OPC_ClientServer_Samples LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC client/server samples ####

#### Samples for S2OPC library and extension libraries ####

### Dependencies ###
# EXPAT dependency managed in CommonDefs
# use find_package onto s2opc in case we built s2opc and sample separately
find_package(s2opc)
# Python3 is mandatory for compilation
if(POLICY CMP0148)
  cmake_policy(SET CMP0148 OLD)
endif()
find_package(PythonInterp 3 REQUIRED)

enable_testing()

# Demo server

# Define a demo XML server configuration
set(SERVER_DEMO_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Server_Demo_Config.xml")
set(SERVER_CSPN_ST_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Server_CSPN_ST_Config.xml")
set(USERS_DEMO_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Users_Demo_Config.xml")

# Define a demo XML address space configuration
if (NOT S2OPC_NANO_PROFILE)
  if(NOT S2OPC_NODE_MANAGEMENT)
    set(DEMO_ADDRESS_SPACE_XML_ORIGIN "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc.xml")
  else()
    set(DEMO_ADDRESS_SPACE_XML_ORIGIN "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_node_mgt.xml")
  endif()
  set(DEMO_ADDRESS_SPACE_XML_FILE "S2OPC_Demo_NodeSet.xml")
else()
  set(DEMO_ADDRESS_SPACE_XML_ORIGIN "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_nano.xml")
  set(DEMO_ADDRESS_SPACE_XML_FILE "S2OPC_Demo_Nano_NodeSet.xml")
endif()

# Manage the Nano profile mode
if (NOT S2OPC_NANO_PROFILE)
  set(TOOLKIT_DEMO_SERVER "toolkit_demo_server")
else()
  set(TOOLKIT_DEMO_SERVER "toolkit_demo_nano_server")
endif()

if (expat_FOUND)
  add_executable(${TOOLKIT_DEMO_SERVER}
    "demo_server/toolkit_demo_server_methods.c"
    "demo_server/toolkit_demo_server.c")
  target_include_directories(${TOOLKIT_DEMO_SERVER} PRIVATE demo_server)
  target_link_libraries(${TOOLKIT_DEMO_SERVER} PRIVATE s2opc_clientserver)
  target_compile_options(${TOOLKIT_DEMO_SERVER} PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(${TOOLKIT_DEMO_SERVER} PRIVATE ${S2OPC_DEFINITIONS})
  target_compile_definitions(${TOOLKIT_DEMO_SERVER} PRIVATE "WITH_EXPAT")

  add_custom_command(
    TARGET ${TOOLKIT_DEMO_SERVER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SERVER_DEMO_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${SERVER_CSPN_ST_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${USERS_DEMO_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${DEMO_ADDRESS_SPACE_XML_ORIGIN} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DEMO_ADDRESS_SPACE_XML_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_public
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/crl
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/certs
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/crl
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/certs
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/crl
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/certs
    COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/crl

    # Add S2OPC application certs in S2OPC_Demo_PKI
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/crl/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/selfsigned/ca_selfsigned_pathLen0.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/untrusted_cacert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/untrusted_cacrl.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/issuers/crl/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/trusted_server_cert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/trusted_client_cert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/int_cli_cacert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/int_cli_cacrl.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/crl/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/int_srv_cacert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/int_srv_cacrl.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_PKI/trusted/crl/
    # Add S2OPC application certs in S2OPC_Users_PKI
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/user_cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/user_cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/trusted/crl/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/certs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/.placeholder ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_PKI/issuers/crl/
    # Add 2048 client, server and user keys
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/encrypted_client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/encrypted_server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/encrypted_user_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/user_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_public/
    # Add 4096 client, server and user keys
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/encrypted_client_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/client_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/encrypted_trusted_client_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/trusted_client_cert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/encrypted_int_client_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/int_client_cert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/encrypted_server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/encrypted_trusted_server_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/untrusted/trusted_server_cert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/encrypted_int_server_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/intermediate/int_server_cert.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/encrypted_user_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/user_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/user_public/
    )
else()
  message(WARNING "Demo server ${TOOLKIT_DEMO_SERVER} executable will not be compiled since Expat library not available.")
endif()

# Demo server with SKS (Security Key Service) for Pub/Sub
if (expat_FOUND AND NOT S2OPC_NANO_PROFILE)
  set(SKS_DEMO_ADDRESS_SPACE_XML "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_sks.xml")
  add_executable(sks_demo_server
    "demo_server/sks_demo_server_methods.c"
    "demo_server/sks_demo_server.c")
  target_link_libraries(sks_demo_server PRIVATE s2opc_clientserver)
  target_compile_options(sks_demo_server PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(sks_demo_server PRIVATE ${S2OPC_DEFINITIONS})
  add_custom_command(
    TARGET sks_demo_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SKS_DEMO_ADDRESS_SPACE_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  message(WARNING "Demo SKS server sks_demo_server executable will not be compiled since Expat library not available or Nano profile activated.")
endif()

if (expat_FOUND)
  # toolkit_demo_file_transfer only support by linux system
  if(NOT WIN32)
    add_executable(toolkit_demo_file_transfer
      "demo_file_transfer/toolkit_demo_file_transfer.c"
      "demo_file_transfer/sopc_file_transfer.c")
    target_link_libraries(toolkit_demo_file_transfer PRIVATE s2opc_clientserver)
    # toolkit_demo_file_transfer needs GNU_SOURCE (fileno and mkstemp functions)
    target_compile_options(toolkit_demo_file_transfer PRIVATE ${S2OPC_COMPILER_FLAGS} -D_GNU_SOURCE)
    target_compile_definitions(toolkit_demo_file_transfer PRIVATE ${S2OPC_DEFINITIONS})
    target_compile_definitions(toolkit_demo_file_transfer PRIVATE "WITH_EXPAT")
    add_custom_command(
      TARGET toolkit_demo_file_transfer POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_file_transfer.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ft_data/s2opc_file_transfer.xml)
  endif()
else()
  message(WARNING "Demo file transfer toolkit_demo_file_transfer executable will not be compiled since Expat library not available.")
endif()

if (expat_FOUND)
  add_executable(toolkit_demo_file_transfer_new
    "demo_file_transfer/toolkit_demo_file_transfer_new.c")
  target_link_libraries(toolkit_demo_file_transfer_new PRIVATE s2opc_clientserver)
  target_compile_options(toolkit_demo_file_transfer_new PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_demo_file_transfer_new PRIVATE ${S2OPC_DEFINITIONS})
  target_compile_definitions(toolkit_demo_file_transfer_new PRIVATE "WITH_EXPAT")
  add_custom_command(
    TARGET toolkit_demo_file_transfer_new POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_file_transfer.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/s2opc_file_transfer.xml)
else()
  message(WARNING "Demo file transfer toolkit_demo_file_transfer executable will not be compiled since Expat library not available.")
endif()

if (expat_FOUND AND NOT S2OPC_NANO_PROFILE)
  set(PUSH_SERVER_ADDRESS_SPACE_XML "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_push.xml")
  set(PUSH_SERVER_TOFU_USERS_XML "${CMAKE_CURRENT_SOURCE_DIR}/demo_push_server/data/S2OPC_Users_TOFU_Config.xml")
  set(PUSH_SERVER_LIB_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/demo_push_server/push_server_lib")
  file(GLOB_RECURSE PUSH_SERVER_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/demo_push_server/*.c )
  add_executable(toolkit_demo_push_server ${PUSH_SERVER_LIB_SOURCES})
  target_link_libraries(toolkit_demo_push_server PRIVATE s2opc_clientserver)
  target_compile_options(toolkit_demo_push_server PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(toolkit_demo_push_server PRIVATE ${S2OPC_DEFINITIONS} "WITH_EXPAT")
  target_include_directories(toolkit_demo_push_server PRIVATE ${PUSH_SERVER_LIB_INCLUDES})
  add_custom_command(
    TARGET ${TOOLKIT_DEMO_SERVER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PUSH_SERVER_ADDRESS_SPACE_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Demo_Push_AddSpace.xml
    COMMAND ${CMAKE_COMMAND} -E copy ${PUSH_SERVER_TOFU_USERS_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/S2OPC_Users_TOFU_Config.xml
  )
else()
  message(WARNING "Demo PUSH server toolkit_demo_push_server executable will not be compiled since Expat library not available or Nano profile activated.")
endif()

# Demo clients (S2OPC only without extension)
set(S2OPC_DEMOCLIENT_COMMON
  "demo_client/argparse.c"
  "demo_client/state_machine.c"
  "demo_client/config.c"
  )

add_executable(s2opc_browse "demo_client/s2opc_browse.c"
  ${S2OPC_DEMOCLIENT_COMMON})

target_link_libraries(s2opc_browse PRIVATE s2opc_clientserver)
target_compile_options(s2opc_browse PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_browse PRIVATE ${S2OPC_DEFINITIONS})

add_executable(s2opc_read "demo_client/s2opc_read.c"
  ${S2OPC_DEMOCLIENT_COMMON})
target_link_libraries(s2opc_read PRIVATE s2opc_clientserver)
target_compile_options(s2opc_read PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_read PRIVATE ${S2OPC_DEFINITIONS})

add_executable(s2opc_write "demo_client/s2opc_write.c"
  ${S2OPC_DEMOCLIENT_COMMON})
target_link_libraries(s2opc_write PRIVATE s2opc_clientserver)
target_compile_options(s2opc_write PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_write PRIVATE ${S2OPC_DEFINITIONS})

add_executable(s2opc_discovery "demo_client/s2opc_discovery.c"
  ${S2OPC_DEMOCLIENT_COMMON})
target_link_libraries(s2opc_discovery PRIVATE s2opc_clientserver)
target_compile_options(s2opc_discovery PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_discovery PRIVATE ${S2OPC_DEFINITIONS})

add_executable(s2opc_findserver "demo_client/s2opc_findserver.c"
  ${S2OPC_DEMOCLIENT_COMMON})
target_link_libraries(s2opc_findserver PRIVATE s2opc_clientserver)
target_compile_options(s2opc_findserver PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_findserver PRIVATE ${S2OPC_DEFINITIONS})

add_executable(s2opc_register "demo_client/s2opc_register.c"
  ${S2OPC_DEMOCLIENT_COMMON})

target_link_libraries(s2opc_register PRIVATE s2opc_clientserver)
target_compile_options(s2opc_register PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_register PRIVATE ${S2OPC_DEFINITIONS})

# Samples with S2OPC client wrapper:
if (expat_FOUND)

  set(CLIENT_WRAPPER_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Client_Wrapper_Config.xml")

  add_executable(s2opc_wrapper_write "client_wrapper/examples/write.c")
  target_include_directories(s2opc_wrapper_write PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_write PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_write PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_write PRIVATE ${S2OPC_DEFINITIONS})

  add_custom_command(
    TARGET s2opc_wrapper_write POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CLIENT_WRAPPER_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )

  add_executable(s2opc_wrapper_add_nodes "client_wrapper/examples/add_nodes.c")
  target_include_directories(s2opc_wrapper_add_nodes PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_add_nodes PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_add_nodes PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_add_nodes PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(s2opc_wrapper_delete_nodes "client_wrapper/examples/delete_nodes.c")
  target_include_directories(s2opc_wrapper_delete_nodes PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_delete_nodes PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_delete_nodes PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_delete_nodes PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(s2opc_wrapper_history_read "client_wrapper/examples/history_read.c")
  target_include_directories(s2opc_wrapper_history_read PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_history_read PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_history_read PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_history_read PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(s2opc_wrapper_read "client_wrapper/examples/read.c")
  target_include_directories(s2opc_wrapper_read PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_read PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_read PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_read PRIVATE ${S2OPC_DEFINITIONS})

  if(S2OPC_EVENT_MANAGEMENT)
    add_executable(s2opc_wrapper_event "client_wrapper/examples/event.c")
    target_include_directories(s2opc_wrapper_event PRIVATE s2opc_clientserver)
    target_link_libraries(s2opc_wrapper_event PRIVATE s2opc_clientserver)
    target_compile_options(s2opc_wrapper_event PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(s2opc_wrapper_event PRIVATE ${S2OPC_DEFINITIONS})
  endif()

  add_executable(s2opc_wrapper_connect "client_wrapper/examples/connect.c")
  target_include_directories(s2opc_wrapper_connect PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_connect PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_connect PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_connect PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(s2opc_wrapper_browse "client_wrapper/examples/browse.c")
  target_include_directories(s2opc_wrapper_browse PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_browse PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_browse PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_browse PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(s2opc_wrapper_get_endpoints "client_wrapper/examples/get_endpoints.c")
  target_include_directories(s2opc_wrapper_get_endpoints PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_get_endpoints PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_get_endpoints PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_get_endpoints PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(s2opc_wrapper_subscribe "client_wrapper/examples/subscribe.c")
  target_include_directories(s2opc_wrapper_subscribe PRIVATE s2opc_clientserver)
  target_link_libraries(s2opc_wrapper_subscribe PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_wrapper_subscribe PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_wrapper_subscribe PRIVATE ${S2OPC_DEFINITIONS})
endif()

# Benchmark tool demo
add_executable(bench_tool "benchmarks/bench_tool.c")
target_link_libraries(bench_tool PRIVATE s2opc_clientserver)
target_compile_options(bench_tool PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(bench_tool PRIVATE ${S2OPC_DEFINITIONS})

# TODO: XML parsing demo: make a unit test / validation test with it instead of demo
if (expat_FOUND)
  add_executable(s2opc_parse_uanodeset "loaders/s2opc_parse_uanodeset.c")
  target_link_libraries(s2opc_parse_uanodeset PRIVATE s2opc_clientserver)
  target_compile_options(s2opc_parse_uanodeset PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(s2opc_parse_uanodeset PRIVATE ${S2OPC_DEFINITIONS})
endif()

#Following binaries test : s2opc_write, s2opc_read, s2opc_browse, s2opc_discovery.
if (expat_FOUND)
  #write
  add_test(NAME "validation::s2opc_write"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 3
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_write>" -u user1 -n "ns=1;i=1008" -t 3 42)
  set_tests_properties("validation::s2opc_write" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_write.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

  add_test(NAME "validation::s2opc_write_X509"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 3
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
  "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
  "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
  "$<TARGET_FILE:s2opc_write>" --user_policy_id=X509_Basic256Sha256 --user_cert=user_public/user_2k_cert.der --user_key=user_private/encrypted_user_2k_key.pem -n "ns=1;i=1008" -t 3 42)
  set_tests_properties("validation::s2opc_write_X509" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_write_X509.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
  )

  add_test(NAME "validation::s2opc_wrapper_write"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 3
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
  "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
  "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
  "$<TARGET_FILE:s2opc_wrapper_write>" "ns=1;i=1008" 42)
set_tests_properties("validation::s2opc_wrapper_write" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_wrapper_write.tap;TEST_USER=user1;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
  )

  #read
  add_test(NAME "validation::s2opc_read"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 2
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_read>" -a 1 -n "ns=1;s=Array_Boolean_097")
  set_tests_properties("validation::s2opc_read" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_read.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

  add_test(NAME "validation::s2opc_read_X509"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 3
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
  "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
  "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
  "$<TARGET_FILE:s2opc_read>" --user_policy_id=X509_Basic256Sha256 --user_cert=user_public/user_4k_cert.der --user_key=user_private/encrypted_user_4k_key.pem -a 1 -n "ns=1;s=Array_Boolean_097")
  set_tests_properties("validation::s2opc_read_X509" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_read_X509.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
  )

  add_test(NAME "validation::s2opc_wrapper_read"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 2
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
  "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
  "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
  "$<TARGET_FILE:s2opc_wrapper_read>" "ns=1;i=1008" "ns=1;s=Array_Boolean_097")
set_tests_properties("validation::s2opc_wrapper_read" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_wrapper_read.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
  )

  #browse
  add_test(NAME "validation::s2opc_browse"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 2
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_browse>" -n "ns=1;i=20002")
  set_tests_properties("validation::s2opc_browse" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_browse.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

  add_test(NAME "validation::s2opc_wrapper_browse"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 2
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_wrapper_browse>" "ns=1;i=20002")
  set_tests_properties("validation::s2opc_wrapper_browse" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_wrapper_browse.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

  #discovery
  add_test(NAME "validation::s2opc_discovery"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_discovery>" --none)
  set_tests_properties("validation::s2opc_discovery" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_discovery.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

  add_test(NAME "validation::s2opc_wrapper_get_endpoints"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 2
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_wrapper_get_endpoints>")
  set_tests_properties("validation::s2opc_wrapper_get_endpoints" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_wrapper_get_endpoints.tap;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

  # subscription
  add_test(NAME "validation::s2opc_wrapper_subscribe"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/samples/ClientServer/scripts/run_with_pwd.py" "--nb-passwords" 2
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
    "--server-wd" $<TARGET_FILE_DIR:${TOOLKIT_DEMO_SERVER}>
    "--server-cmd" $<TARGET_FILE:${TOOLKIT_DEMO_SERVER}>
    "$<TARGET_FILE:s2opc_wrapper_subscribe>" "ns=1;i=1012" "i=2258")
  set_tests_properties("validation::s2opc_wrapper_subscribe" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=s2opc_wrapper_subscribe.tap;SET_SUBSCRIBE_TIMEOUT=1;TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml;TEST_SERVER_XML_ADDRESS_SPACE=./${DEMO_ADDRESS_SPACE_XML_FILE};TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml"
    )

endif()

## PyS2OPC library samples tests ##

if(WIN32)
  set(ENV_PATH_SEP "\;")
else()
  set(ENV_PATH_SEP ":")
endif()

if(WITH_PYS2OPC)
  set(PYS2OPC_SRC_PATH "${S2OPC_ROOT_PATH}/src/ClientServer/frontend/pys2opc")
  set(PYS2OPC_TEST_PATH "${S2OPC_ROOT_PATH}/samples/ClientServer/pys2opc")
  # TODO: This variable must be the one defined in ../src/ClientServer/frontend/pys2opc/CMakeLists.txt
  set(PYS2OPC_TEST_INSTALL_PATH "${CMAKE_BINARY_DIR}/pys2opc_install")

  # Validation test
  function(pys2opc_sample_test name)
    get_filename_component(no_ext_name ${name} NAME_WE) # remove extension to use name for log directory
    set(test_name "validation::pys2opc_server-${name}")
    add_test(NAME "${test_name}"
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMAND
      "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
      "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/ClientServer/scripts/with-opc-server.py"
      "--server-cmd" "${PYTHON_EXECUTABLE} ${PYS2OPC_TEST_PATH}/server/server.py --config-path ./S2OPC_Server_Demo_Config.xml --users-path ./S2OPC_UACTT_Users.xml"
      "${PYTHON_EXECUTABLE}" "${PYS2OPC_TEST_PATH}/client/${name}"
      )
    # ASan requires to be the first dynamically linked library to be loaded. This is done with LD_PRELOAD.
    set(new_env "PYTHONPATH=${PYS2OPC_TEST_INSTALL_PATH}${ENV_PATH_SEP}${PYS2OPC_SRC_PATH}"
      "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
      "CK_TAP_LOG_FILE_NAME=pys2opc_server-${name}.tap")
    if(WITH_ASAN OR WITH_UBSAN OR WITH_TSAN)
      set(new_env ${new_env} "LD_PRELOAD=/usr/local/lib64/libasan.so")
    endif()
    # TODO: make the test depends on pys2opc_install
    set_tests_properties(${test_name} PROPERTIES
      ENVIRONMENT "${new_env}")
  endfunction()

  pys2opc_sample_test("0-read-write.py")
  pys2opc_sample_test("1-browse.py")
  pys2opc_sample_test("2-subscribe.py")
  pys2opc_sample_test("3-multi-connection-multi-request.py")
endif()
