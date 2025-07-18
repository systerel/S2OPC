cmake_minimum_required (VERSION 3.5)

project (S2OPC_PubSub_Tests LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC PubSub tests ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and tests separately

# Python3 is mandatory for compilation
if(POLICY CMP0148)
  cmake_policy(SET CMP0148 OLD)
endif()
find_package(PythonInterp 3 REQUIRED)

# We always link check statically, so that our test binaries are standalone
find_package(check REQUIRED)

#### Tests for S2OPC library and extension libraries ####

# Options reserved to tests
option(PUBSUB_STATIC_CONFIG "Load PubSub configuration statically" OFF)

if(WIN32)
  set(ENV_PATH_SEP "\;")
else()
  set(ENV_PATH_SEP ":")
endif()

### Tests and validation of S2OPC library ###

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

# Function to add a validation test using the pubsub server code
# Note: no tap-wrap used in this case, as caller is directly calling TapLogger (python script).
function(s2opc_pubsub_server_validation_test name pubsub_server_name static_flag sks_name sks_port)
  set(test_name "validation::${name}")

  if(NOT S2OPC_NANO_PROFILE)
    # Note: use absolute path for scripts to allow ctest to be run from another path
    add_test(NAME "${test_name}"
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMAND
      "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
      --sks-cmd $<TARGET_FILE:${sks_name}>
      --sks-url "opc.tcp://localhost:${sks_port}"
      "--publisher-cmd" $<TARGET_FILE:${pubsub_server_name}>
      "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/validation_tools/${name}.py" "--tap" "${name}.tap" ${static_flag})
  endif()

endfunction()

## S2OPC PubSub unit tests ##

add_executable(xml_parser_test "unit_tests/xml_parser_test.c")
target_link_libraries(xml_parser_test PRIVATE s2opc_pubsub Check::check)
target_compile_options(xml_parser_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(xml_parser_test PRIVATE ${S2OPC_DEFINITIONS})

add_custom_command(TARGET xml_parser_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_sub.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub_acyclic.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
  COMMAND ${CMAKE_COMMAND} -E copy ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/sks_server_2k_cert.der
  )

s2opc_unit_test(xml_parser_test)

add_executable(pubsub_modules_test "unit_tests/pubsub_modules_test.c")
target_link_libraries(pubsub_modules_test PRIVATE s2opc_pubsub Check::check)
target_compile_options(pubsub_modules_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(pubsub_modules_test PRIVATE ${S2OPC_DEFINITIONS})

s2opc_unit_test(pubsub_modules_test)

add_executable(mqtt_pubsub_test "unit_tests/mqtt_pubsub_test.c")
target_link_libraries(mqtt_pubsub_test PRIVATE s2opc_pubsub Check::check)
target_compile_options(mqtt_pubsub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(mqtt_pubsub_test PRIVATE ${S2OPC_DEFINITIONS})

add_custom_command(
  TARGET mqtt_pubsub_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/mosquitto.conf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/mqttBroker_passwordFile.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )

s2opc_unit_test(mqtt_pubsub_test)


## S2OPC PubSub validation tests ##

add_executable(udp_pub_test "validation_tests/udp_pub_test.c")
target_compile_options(udp_pub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(udp_pub_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(udp_pub_test PRIVATE s2opc_pubsub)

add_executable(udp_pub_conf_test "validation_tests/udp_pub_conf_test.c")
target_compile_options(udp_pub_conf_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(udp_pub_conf_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(udp_pub_conf_test PRIVATE s2opc_pubsub)

add_executable(udp_pub_xml_test "validation_tests/udp_pub_xml_test.c")
target_compile_options(udp_pub_xml_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(udp_pub_xml_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(udp_pub_xml_test PRIVATE s2opc_pubsub)

add_executable(pub_scheduler_test "validation_tests/pub_scheduler_test.c")
target_compile_options(pub_scheduler_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(pub_scheduler_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(pub_scheduler_test PRIVATE s2opc_pubsub)

# Copy data
add_custom_command(
  TARGET pub_scheduler_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub_interop.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub_scheduler.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub_scheduler_security.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub_scheduler_eth.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_pub_scheduler_eth_err.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )

add_executable(udp_sub_test "validation_tests/udp_sub_test.c")
target_compile_options(udp_sub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(udp_sub_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(udp_sub_test PRIVATE s2opc_pubsub)

add_executable(sub_scheduler_conf_test "validation_tests/sub_scheduler_conf_test.c")
target_compile_options(sub_scheduler_conf_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(sub_scheduler_conf_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(sub_scheduler_conf_test PRIVATE s2opc_pubsub)

# Copy data
add_custom_command(
  TARGET sub_scheduler_conf_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_sub_security.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_sub_eth.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_sub_eth_err.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/config_sub_eth2.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
  )

add_executable(udp_multi_pub_test "validation_tests/udp_multi_pub_test.c")
target_compile_options(udp_multi_pub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(udp_multi_pub_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(udp_multi_pub_test PRIVATE s2opc_pubsub)

add_executable(udp_multi_sub_test "validation_tests/udp_multi_sub_test.c")
target_compile_options(udp_multi_sub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(udp_multi_sub_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(udp_multi_sub_test PRIVATE s2opc_pubsub)

add_test(NAME "validation::ll_pub_sub_test"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--publisher-cmd" $<TARGET_FILE:udp_pub_test>
  $<TARGET_FILE:udp_sub_test>)
set_tests_properties("validation::ll_pub_sub_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ll_pub_sub_test.tap")

add_test(NAME "validation::ll_pub_sub_conf_test"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--publisher-cmd" $<TARGET_FILE:udp_pub_conf_test>
  $<TARGET_FILE:udp_sub_test>)
set_tests_properties("validation::ll_pub_sub_conf_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ll_pub_sub_conf_test.tap")

add_test(NAME "validation::ll_pub_sub_xml_test"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--publisher-cmd" $<TARGET_FILE:udp_pub_xml_test>
  $<TARGET_FILE:sub_scheduler_conf_test>)
set_tests_properties("validation::ll_pub_sub_xml_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ll_pub_sub_xml_test.tap")

add_test(NAME "validation::pubsub_scheduler_udp"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--publisher-cmd" $<TARGET_FILE:pub_scheduler_test>
  $<TARGET_FILE:sub_scheduler_conf_test>)
set_tests_properties("validation::pubsub_scheduler_udp" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_scheduler_udp.tap")

add_test(NAME "validation::pubsub_sched_ethernet_ROOT_required"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--no-wait-pub-message"
  "--publisher-cmd" "$<TARGET_FILE:pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_scheduler_eth.xml"
  $<TARGET_FILE:sub_scheduler_conf_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_sub_eth.xml)
set_tests_properties("validation::pubsub_sched_ethernet_ROOT_required" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_sched_ethernet.tap")

add_test(NAME "validation::pubsub_sched_ethernet_with_recv_itf"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--no-wait-pub-message"
  "--publisher-cmd" "$<TARGET_FILE:pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_scheduler_eth.xml"
  $<TARGET_FILE:sub_scheduler_conf_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_sub_eth2.xml)
set_tests_properties("validation::pubsub_sched_ethernet_with_recv_itf" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_sched_ethernet_with_recv_itf.tap")

add_test(NAME "validation::pubsub_sched_ethernet_pub_config_err"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  $<TARGET_FILE:pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_scheduler_eth_err.xml)
set_tests_properties("validation::pubsub_sched_ethernet_pub_config_err" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_sched_ethernet_pub_config_err.tap;CK_EXP_RET_CODE=255")

add_test(NAME "validation::pubsub_sched_ethernet_sub_config_err"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--no-wait-pub-message"
  "--publisher-cmd" "$<TARGET_FILE:pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_scheduler_eth.xml"
  $<TARGET_FILE:sub_scheduler_conf_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_sub_eth_err.xml)
set_tests_properties("validation::pubsub_sched_ethernet_sub_config_err" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_sched_ethernet_sub_config_err.tap;CK_EXP_RET_CODE=255")

# Test signature failure callback is called when publisher has no keys available to sign the message
add_test(NAME "validation::pubsub_sign_failure_callback"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  $<TARGET_FILE:pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_scheduler_security.xml)
set_tests_properties("validation::pubsub_sign_failure_callback" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_sign_failure_callback.tap;CK_EXP_RET_CODE=125")

add_test(NAME "validation::ll_multi_pub_sub_test"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--publisher-cmd" $<TARGET_FILE:udp_multi_pub_test>
  $<TARGET_FILE:udp_multi_sub_test>)
set_tests_properties("validation::ll_multi_pub_sub_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ll_multi_pub_sub_test.tap")

if(NOT S2OPC_PUBSUB_ONLY)
  # This test requires both PubSub and ClientServer libraries
  if(PUBSUB_STATIC_CONFIG)
     set(test_server_static "--static")
  else()
     set(test_server_static "")
  endif()

  s2opc_pubsub_server_validation_test("pubsub_server_test" pubsub_server "${test_server_static}" test_server_sks 4841)
  s2opc_pubsub_server_validation_test("controller_device_pubsub_test" controller_sks_client "" device_sks_server_push 4842)

  # Test check signature failure callback is called when no keys are available
  add_test(NAME "validation::pubsub_check_sign_failure_callback_sub_no_keys"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
    "--sks-cmd" $<TARGET_FILE:test_server_sks>
    "--sks-url" "opc.tcp://localhost:4841"
    "--kill-publisher"
    "--publisher-cmd" $<TARGET_FILE:pubsub_server>
    $<TARGET_FILE:sub_scheduler_conf_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_sub_security.xml)
  set_tests_properties("validation::pubsub_check_sign_failure_callback_sub_no_keys" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_check_sign_failure_callback_sub_no_keys.tap;CK_EXP_RET_CODE=125")

  # Test check signature failure callback is called when keys are available but do not match the publisher ones
  add_test(NAME "validation::pubsub_check_sign_failure_callback_sub_unexpected_keys"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
    "--sks-cmd" $<TARGET_FILE:test_server_sks>
    "--sks-url" "opc.tcp://localhost:4841"
    "--kill-publisher"
    "--publisher-cmd" $<TARGET_FILE:pubsub_server>
    $<TARGET_FILE:sub_scheduler_conf_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_sub_security.xml "true")
  set_tests_properties("validation::pubsub_check_sign_failure_callback_sub_unexpected_keys" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=pubsub_check_sign_failure_callback_sub_unexpected_keys.tap;CK_EXP_RET_CODE=125")
endif()

## S2OPC PubSub interoperability tests ##

add_executable(interop_sub_test "interop_tests/interop_sub_test.c")
target_compile_options(interop_sub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(interop_sub_test PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(interop_sub_test PRIVATE s2opc_pubsub)

if(UNIX)

  # open62541_pub only for linux
  add_test(NAME "interop::open62541_pub"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
    "--publisher-cmd" "${S2OPC_ROOT_PATH}/tests/PubSub/bin_interop/open62541_pub"
    $<TARGET_FILE:interop_sub_test>)
  set_tests_properties("interop::open62541_pub" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=interop_pub_test.tap")

  add_test(NAME "interop::open62541_sub"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
    "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
    "--publisher-cmd" "$<TARGET_FILE:pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_interop.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin_interop/open62541_sub")
  set_tests_properties("interop::open62541_sub" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=interop_sub_test.tap")

  add_test(NAME "interop::open62541_pub_encrypted"
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
  "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
  "--publisher-cmd" "${S2OPC_ROOT_PATH}/tests/PubSub/bin_interop/open62541_pub_encrypted"
  $<TARGET_FILE:interop_sub_test> "encrypted")
  set_tests_properties("interop::open62541_pub_encrypted" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=interop_pub_encrypted_interop.tap")

  if(NOT S2OPC_PUBSUB_ONLY)
  # This test requires both PubSub and ClientServer libraries
    # PubSub only for linux
    add_test(NAME "validation::cli_self_test"
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMAND
      "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/validation_tools/cli_autotest.py")
    set_tests_properties("validation::cli_self_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=cli_self_test.tap")
  endif()

endif()

## S2OPC fuzzing tests: to be run manually ##

if(ENABLE_FUZZING)
  set(FUZZING_PATH "fuzzing")

  s2opc_fuzzer(sub_fuzzer "${FUZZING_PATH}/fuzz_sub.c" s2opc_pubsub "")
endif(ENABLE_FUZZING)
