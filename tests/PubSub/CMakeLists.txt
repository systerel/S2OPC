cmake_minimum_required (VERSION 3.5)

project (S2OPC_PubSub_Tests LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

#### S2OPC PubSub tests ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and tests separately

# options with impact on dependencies
option(ENABLE_TESTING "Build the S2OPC tests" ON)

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

# We always link check statically, so that our test binaries are standalone
if(ENABLE_TESTING)
  find_package(check REQUIRED)
endif()


#### Tests for S2OPC library and extension libraries ####

# Options reserved to tests
option(PUBSUB_STATIC_CONFIG "Load PubSub configuration statically" OFF)

if(WIN32)
    set(ENV_PATH_SEP "\;")
else()
    set(ENV_PATH_SEP ":")
endif()

### Tests and validation of S2OPC library ###

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

  # Function to add a validation test using the pubsub server code
  function(s2opc_pubsub_server_validation_test name)
    set(test_name "validation::${name}")
    if(PUBSUB_STATIC_CONFIG)
      set(test_server_static "--static")
    else()
      set(test_server_static "")
    endif()

    #Note: use absolute path for scripts to allow ctest to be run from another another path
    add_test(NAME "${test_name}"
      WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMAND
      "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
      "--sks-cmd" $<TARGET_FILE:toolkit_demo_sks>
      "--publisher-cmd" $<TARGET_FILE:pubsub_server>
      "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/validation_tools/${name}.py" ${test_server_static})
  endfunction()

  # Function to add a validation test using the pubsub server code
  function(s2opc_sks_server_validation_test name)
    set(test_name "validation::${name}")
    if(PUBSUB_STATIC_CONFIG)
      # Test script is using dynamic configuration, this test cannot be done with static configuration
      # Still generate a TAP file since file is expected by test-all.sh script
      set(test_cmd "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py" "${PYTHON_EXECUTABLE}" "--version")
      add_test(NAME "${test_name}"
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${test_cmd})
      set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${name}.tap")

    else()
      #Note: use absolute path for scripts to allow ctest to be run from another another path
      add_test(NAME "${test_name}"
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND
        "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/validation_tools/${name}.py"
        "--binary_dir" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif()

  endfunction()

## S2OPC PubSub unit tests ##

  add_executable(xml_parser_test "unit_tests/xml_parser_test.c")
  target_link_libraries(xml_parser_test PRIVATE s2opc_pubsub Check::check)
  target_compile_options(xml_parser_test PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(xml_parser_test PRIVATE ${S2OPC_DEFINITIONS})

  add_custom_command(TARGET xml_parser_test POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/xml_test/config_sub.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/xml_test/config_pub.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  s2opc_unit_test(xml_parser_test)

  add_executable(pubsub_modules_test "unit_tests/pubsub_modules_test.c")
  target_link_libraries(pubsub_modules_test PRIVATE s2opc_pubsub Check::check)
  target_compile_options(pubsub_modules_test PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(pubsub_modules_test PRIVATE ${S2OPC_DEFINITIONS})

  s2opc_unit_test(pubsub_modules_test)

  add_executable(mqtt_pub_test "unit_tests/mqtt_pub_test.c")
  target_link_libraries(mqtt_pub_test PRIVATE s2opc_pubsub ${CHECK_LIBRARIES})
  target_compile_options(mqtt_pub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(mqtt_pub_test PRIVATE ${S2OPC_DEFINITIONS})

  s2opc_unit_test(mqtt_pub_test)


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

  add_executable(udp_pub_scheduler_test "validation_tests/udp_pub_scheduler_test.c")
  target_compile_options(udp_pub_scheduler_test PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(udp_pub_scheduler_test PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(udp_pub_scheduler_test PRIVATE s2opc_pubsub)

  # Copy data
  add_custom_command(
    TARGET udp_pub_scheduler_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/xml_test/config_pub_interop.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/xml_test/config_pub_scheduler.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    )

  add_executable(udp_sub_test "validation_tests/udp_sub_test.c")
  target_compile_options(udp_sub_test PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(udp_sub_test PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(udp_sub_test PRIVATE s2opc_pubsub)

  add_executable(udp_sub_conf_test "validation_tests/udp_sub_conf_test.c")
  target_compile_options(udp_sub_conf_test PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(udp_sub_conf_test PRIVATE ${S2OPC_DEFINITIONS})
  target_link_libraries(udp_sub_conf_test PRIVATE s2opc_pubsub)

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
           $<TARGET_FILE:udp_sub_conf_test>)
  set_tests_properties("validation::ll_pub_sub_xml_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ll_pub_sub_xml_test.tap")

  add_test(NAME "validation::publisher_scheduler_test"
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           COMMAND
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
           "--publisher-cmd" $<TARGET_FILE:udp_pub_scheduler_test>
           $<TARGET_FILE:udp_sub_conf_test>)
  set_tests_properties("validation::publisher_scheduler_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=publisher_scheduler_test.tap")

  add_test(NAME "validation::ll_multi_pub_sub_test"
           WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
           COMMAND
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/scripts/tap-wrap.py"
           "${PYTHON_EXECUTABLE}" "${S2OPC_ROOT_PATH}/tests/PubSub/scripts/with-publisher.py"
           "--publisher-cmd" $<TARGET_FILE:udp_multi_pub_test>
           $<TARGET_FILE:udp_multi_sub_test>)
  set_tests_properties("validation::ll_multi_pub_sub_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=ll_multi_pub_sub_test.tap")

  if(S2OPC_PUBSUB_ONLY)
    # it is not possible to use both PubSub and ClientServer libraries
  elseif(WITH_NANO_EXTENDED)
    s2opc_pubsub_server_validation_test("pubsub_server_test")
    s2opc_sks_server_validation_test("pubsub_sks_test")
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
      "--publisher-cmd" "$<TARGET_FILE:udp_pub_scheduler_test> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config_pub_interop.xml"
      "${CMAKE_CURRENT_SOURCE_DIR}/bin_interop/open62541_sub")
    set_tests_properties("interop::open62541_sub" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=interop_sub_test.tap")

  endif()

endif(ENABLE_TESTING)
