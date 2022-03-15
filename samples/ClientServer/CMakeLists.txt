cmake_minimum_required (VERSION 3.5)

project (S2OPC_ClientServer_Samples LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC client/server samples ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and sample separately
find_package(s2opc)

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

#### Samples for S2OPC library and extension libraries ####

# Options reserved to tests / samples
option(ENABLE_SAMPLES "Build the S2OPC library and extention libraries samples" ON)

### Samples of S2OPC library and extension libraries use ###

if(ENABLE_SAMPLES)

  # Demo server

  # Define a demo XML server configuration
  set(SERVER_DEMO_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Server_Demo_Config.xml")
  set(USERS_DEMO_CONFIG_XML  "${CMAKE_CURRENT_SOURCE_DIR}/data/config/S2OPC_Users_Demo_Config.xml")

  # Define a demo XML address space configuration
  if (WITH_NANO_EXTENDED)
    set(DEMO_ADDRESS_SPACE_XML_ORIGIN "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc.xml")
    set(DEMO_ADDRESS_SPACE_XML_FILE "S2OPC_Demo_NodeSet.xml")
  else()
    set(DEMO_ADDRESS_SPACE_XML_ORIGIN "${CMAKE_CURRENT_SOURCE_DIR}/data/address_space/s2opc_nano.xml")
    set(DEMO_ADDRESS_SPACE_XML_FILE "S2OPC_Demo_Nano_NodeSet.xml")
  endif()

  # Manage the Nano profiles extended mode
  if (WITH_NANO_EXTENDED)
    set(TOOLKIT_DEMO_SERVER "toolkit_demo_server")
  else()
    set(TOOLKIT_DEMO_SERVER "toolkit_demo_nano_server")
  endif()

  if (expat_FOUND)
    add_executable(${TOOLKIT_DEMO_SERVER}
      "demo_server/toolkit_demo_server.c")
    target_link_libraries(${TOOLKIT_DEMO_SERVER} PRIVATE s2opc_serverwrapper)
    target_compile_options(${TOOLKIT_DEMO_SERVER} PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(${TOOLKIT_DEMO_SERVER} PRIVATE ${S2OPC_DEFINITIONS})

    target_link_libraries(${TOOLKIT_DEMO_SERVER} PRIVATE s2opc_clientserver-xml-loaders-expat)
    target_compile_definitions(${TOOLKIT_DEMO_SERVER} PRIVATE "WITH_EXPAT")

    add_custom_command(
      TARGET ${TOOLKIT_DEMO_SERVER} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${SERVER_DEMO_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMAND ${CMAKE_COMMAND} -E copy ${USERS_DEMO_CONFIG_XML} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMAND ${CMAKE_COMMAND} -E copy ${DEMO_ADDRESS_SPACE_XML_ORIGIN} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DEMO_ADDRESS_SPACE_XML_FILE}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/issued
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/cacrl.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
      # Add 4096 client / server keys
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/client_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/client_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
      )
  else()
    message(WARNING "Demo server ${TOOLKIT_DEMO_SERVER} executable will not be compiled since Expat library not available.")
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

  # Samples with S2OPC subscribing client library

  # s2opc_subscription_client is not built on Windows (use getopt GNU library to parse command line)
  if(NOT WIN32)
    # Demo client
    add_executable(s2opc_subscription_client "client_wrapper/client_subscription.c")
    target_compile_options(s2opc_subscription_client PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(s2opc_subscription_client PRIVATE ${S2OPC_DEFINITIONS})
    target_link_libraries(s2opc_subscription_client PRIVATE s2opc_clientwrapper)
  endif(NOT WIN32)

  # Samples with S2OPC client wrapper:
  add_executable(example_wrapper_write "client_wrapper/examples/write.c")
  target_include_directories(example_wrapper_write PRIVATE s2opc_clientwrapper)
  target_link_libraries(example_wrapper_write PRIVATE s2opc_clientwrapper)
  target_compile_options(example_wrapper_write PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_write PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_read "client_wrapper/examples/read.c")
  target_include_directories(example_wrapper_read PRIVATE s2opc_clientwrapper)
  target_link_libraries(example_wrapper_read PRIVATE s2opc_clientwrapper)
  target_compile_options(example_wrapper_read PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_read PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_browse "client_wrapper/examples/browse.c")
  target_include_directories(example_wrapper_browse PRIVATE s2opc_clientwrapper)
  target_link_libraries(example_wrapper_browse PRIVATE s2opc_clientwrapper)
  target_compile_options(example_wrapper_browse PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_browse PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_get_endpoints "client_wrapper/examples/get_endpoints.c")
  target_include_directories(example_wrapper_get_endpoints PRIVATE s2opc_clientwrapper)
  target_link_libraries(example_wrapper_get_endpoints PRIVATE s2opc_clientwrapper)
  target_compile_options(example_wrapper_get_endpoints PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_get_endpoints PRIVATE ${S2OPC_DEFINITIONS})

  add_executable(example_wrapper_subscribe "client_wrapper/examples/subscribe.c")
  target_include_directories(example_wrapper_subscribe PRIVATE s2opc_clientwrapper)
  target_link_libraries(example_wrapper_subscribe PRIVATE s2opc_clientwrapper)
  target_compile_options(example_wrapper_subscribe PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(example_wrapper_subscribe PRIVATE ${S2OPC_DEFINITIONS})

  # Benchmark tool demo
  add_executable(bench_tool "benchmarks/bench_tool.c")
  target_link_libraries(bench_tool PRIVATE s2opc_clientserver)
  target_compile_options(bench_tool PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(bench_tool PRIVATE ${S2OPC_DEFINITIONS})

  # TODO: XML parsing demo: make a unit test / validation test with it instead of demo
  if (expat_FOUND)
    add_executable(s2opc_parse_uanodeset "loaders/s2opc_parse_uanodeset.c")
    target_link_libraries(s2opc_parse_uanodeset PRIVATE s2opc_clientserver-xml-loaders-expat)
    target_compile_options(s2opc_parse_uanodeset PRIVATE ${S2OPC_COMPILER_FLAGS})
    target_compile_definitions(s2opc_parse_uanodeset PRIVATE ${S2OPC_DEFINITIONS})
  endif()

endif(ENABLE_SAMPLES)
