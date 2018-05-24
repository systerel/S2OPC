cmake_minimum_required (VERSION 2.8.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

project (INGOPCS)
include(CheckCXXCompilerFlag)

# Define runtime output directory
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

# Compiler options (C99 compliant)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")

# Define compiler options for each mode.
# Default optimization level is -O0 for debug and -O2 for others modes.
# -f-no-omit-frame-pointer is set to use Perf profiling tool,
# it is not set in Release mode.
# -g option is already set in Debug and Debugwithdebinfo modes.
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit -Wreturn-type -Wsequence-point -Wcast-qual -Wuninitialized -Wcast-align -Wstrict-prototypes -Wchar-subscripts -Wformat-security -Wformat -Wconversion")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wunicode -Wimplicit-int -Wreserved-id-macro -Wsometimes-uninitialized -Wunsequenced -Wincompatible-pointer-types-discards-qualifiers -Wunevaluated-expression -Wparentheses -Wint-conversion -Wint-to-pointer-cast -Wincompatible-pointer-types -Wvla -Wconversion")
else()
  message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# check if compiler support new sanitization options
CHECK_CXX_COMPILER_FLAG(-fsanitize=pointer-compare COMPILER_SUPPORTS_SAN_PC)
CHECK_CXX_COMPILER_FLAG(-fsanitize=pointer-subtract COMPILER_SUPPORTS_SAN_PS)

option(WITH_ASAN "build with ASAN" OFF)
if(WITH_ASAN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  if(COMPILER_SUPPORTS_SAN_PC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=pointer-compare")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=pointer-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-compare")
  endif()
  if(COMPILER_SUPPORTS_SAN_PS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=pointer-subtract")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=pointer-subtract")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-subtract")
  endif()
endif()

option(WITH_TSAN "build with TSAN" OFF)
if(WITH_TSAN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -pie")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -fPIC")
endif()

option(WITH_UBSAN "build with UBSAN" OFF)
if(WITH_UBSAN)
  add_definitions(-DROCKSDB_UBSAN_RUN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
endif()

option(WITH_COVERAGE "build with COVERAGE" OFF)
if(WITH_COVERAGE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

# Re-enable asserts for Release and RelWithDebInfo builds
string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})

# Source files of INGOPCS library

# Define platform dependent source files
if(UNIX)
  set(PLATFORM_DIR linux)
  set(THREAD_LIB pthread rt)
  set(MATH_LIB m)
elseif(WIN32)
  set(PLATFORM_DIR windows)
  set(SOCK_LIB ws2_32)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

file(GLOB PLATFORM_SRCS "csrc/helpers_platform_dep/*.c" "csrc/helpers_platform_dep/${PLATFORM_DIR}/*.c")

# Set specialized compilation flags for platform dependent source files
if(UNIX)
  # add POSIX support
  set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_XOPEN_SOURCE=600")
elseif(WIN32)
  # minimum Vista for IPV6 support
  set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_WIN32_WINNT=0x0600")
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_HOST_UNIX)
  set(PYTHON_BIN python3)
elseif(CMAKE_HOST_WIN32)
  set(PYTHON_BIN python)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_HOST_UNIX)
  add_custom_command(OUTPUT csrc/configuration/sopc_toolkit_build_info.h
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/csrc/configuration/sopc_toolkit_build_info.h_ ${CMAKE_SOURCE_DIR}/csrc/configuration/sopc_toolkit_build_info.h
                     COMMAND ./gen_build_info_file.sh ${CMAKE_SOURCE_DIR}
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else()
  add_custom_command(OUTPUT csrc/configuration/sopc_toolkit_build_info.h
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/csrc/configuration/sopc_toolkit_build_info.h_ ${CMAKE_SOURCE_DIR}/csrc/configuration/sopc_toolkit_build_info.h
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

add_custom_target(build_info ALL
                   DEPENDS csrc/configuration/sopc_toolkit_build_info.h
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

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
set(CHECK_USE_STATIC_LIBS 1)
find_package(Check REQUIRED)

# Define non-platform dependent source files
file(GLOB_RECURSE INGOPCS_SRCS
    "csrc/api_toolkit/*.c"
    "csrc/configuration/*.c"
    "csrc/helpers/*.c"
    "csrc/crypto/*.c"
    "csrc/opcua_types/*.c"
    "csrc/sockets/*.c"
    "csrc/secure_channels/*.c"
    "csrc/services/*.c"
)

# Define INGOPCS library includes
file(GLOB INGOPCS_INCLUDE_FILES
    "csrc/api_toolkit/*.h"
    "csrc/configuration/*.h"
    "csrc/helpers/*.h"
    "csrc/opcua_types/*.h"
    "csrc/crypto/*.h"
    "csrc/helpers_platform_dep/*.h"
    "csrc/services/b2c/*.h")

set(INGOPCS_PUBLIC_INCLUDES
    "csrc/api_toolkit"
    "csrc/configuration"
    "csrc/helpers"
    "csrc/opcua_types"
    "csrc/crypto"
    "csrc/helpers_platform_dep"
    "csrc/services/b2c"
)

set(INGOPCS_PRIVATE_INCLUDES
    "csrc/sockets"
    "csrc/secure_channels"
    "csrc/services"
    "csrc/crypto/mbedtls"
    "csrc/helpers_platform_dep/${PLATFORM_DIR}"
    "csrc/services/bgenc"
)

# Define INGOPCS library dependencies
set(INGOPCS_DEP_LIBS ${MBEDTLS_LIBRARIES} ${MATH_LIB} ${THREAD_LIB} ${SOCK_LIB})
set(INGOPCS_STATIC_DEP_LIBS ${MBEDTLS_STATIC_LIBRARIES} ${MATH_LIB} ${THREAD_LIB} ${SOCK_LIB})

add_library(ingopcs_obj OBJECT ${PLATFORM_SRCS} ${INGOPCS_SRCS})
target_include_directories(ingopcs_obj BEFORE
	PUBLIC  ${INGOPCS_PUBLIC_INCLUDES}
	PRIVATE ${INGOPCS_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS}
)
add_dependencies(ingopcs_obj build_info)

# Needed if we want to use the objects in a shared library
set_property(TARGET ingopcs_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(ingopcs $<TARGET_OBJECTS:ingopcs_obj>)
target_include_directories(ingopcs PUBLIC ${INGOPCS_PUBLIC_INCLUDES})
target_link_libraries(ingopcs PUBLIC ${INGOPCS_DEP_LIBS})

if(WIN32)
	# always link libc statically (even in case of shared library)
	target_link_libraries(ingopcs PRIVATE -static-libgcc)
endif()

# Ensure we always build a static version of ingopcs for the tests
if (${BUILD_SHARED_LIBS})
	add_library(ingopcs_static STATIC $<TARGET_OBJECTS:ingopcs_obj>)
	target_include_directories(ingopcs_static PUBLIC ${INGOPCS_PUBLIC_INCLUDES})
	target_link_libraries(ingopcs_static PUBLIC ${INGOPCS_STATIC_DEP_LIBS})
endif()

if (${BUILD_SHARED_LIBS})
	set(INGOPCS_STATIC_LIB ingopcs_static)
else()
	set(INGOPCS_STATIC_LIB ingopcs)
endif()

# Define installation targets
## retrieve public include files
foreach(dir ${INGOPCS_PUBLIC_INCLUDES})
    file(GLOB temp_files "${dir}/*.h")
    list(APPEND INGOPCS_INCLUDE_FILES ${temp_files})
endforeach()
## set public header property
set_target_properties(ingopcs PROPERTIES PUBLIC_HEADER "${INGOPCS_INCLUDE_FILES}")
## install files
install(TARGETS ingopcs
        LIBRARY DESTINATION lib # Supposedly dynamic lib
        ARCHIVE DESTINATION lib # Supposedly static lib
        PUBLIC_HEADER DESTINATION include
)

# Address space loader backends

add_library(ingopcs-loader-embedded STATIC csrc/address_space_loaders/embedded/loader.c)
target_link_libraries(ingopcs-loader-embedded PRIVATE ingopcs)
target_include_directories(ingopcs-loader-embedded PRIVATE ${INGOPCS_PUBLIC_INCLUDES} PUBLIC "csrc/address_space_loaders")

function(s2opc_embed_address_space var_name uanodeset_path)
    set(c_file_path "address_space_${var_name}.c")

    add_custom_command(
        OUTPUT ${c_file_path}
        DEPENDS ${uanodeset_path}
		COMMAND ${PYTHON_BIN} ${CMAKE_SOURCE_DIR}/generate-ingopcs-address-space ${uanodeset_path} ${c_file_path}
        COMMENT "Generating address space ${var_name}"
        VERBATIM
    )

    set_source_files_properties(${c_file_path} PROPERTIES GENERATED TRUE)
    set_source_files_properties(${c_file_path} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)

    set(${var_name} ${c_file_path} PARENT_SCOPE)
endfunction()

find_package(EXPAT)

if (EXPAT_FOUND)
	add_library(ingopcs-loader-uanodeset-expat STATIC csrc/address_space_loaders/uanodeset_expat/loader.c)
	target_link_libraries(ingopcs-loader-uanodeset-expat PUBLIC ${EXPAT_LIBRARIES} PRIVATE ingopcs)
	target_include_directories(ingopcs-loader-uanodeset-expat PRIVATE ${INGOPCS_PUBLIC_INCLUDES} ${EXPAT_INCLUDE_DIRS} PUBLIC "csrc/address_space_loaders")
	set_target_properties(ingopcs-loader-uanodeset-expat PROPERTIES COMPILE_DEFINITIONS "UANODESET_LOADER_LOG")
endif()

# INGOPCS tests ################################################################################

if(WIN32)
    set(FILE_EXT .exe)
endif()

function(s2opc_unit_test binary_name)
	set(test_name "unit::${binary_name}")
	set(test_cmd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${binary_name}${FILE_EXT})

	foreach(arg ${ARGN})
		if("${arg}" STREQUAL "GENERATE_TAP")
			set(test_cmd "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap" "${test_cmd}")
		else()
			message(FATAL_ERROR "Unknown parameter for s2opc_unit_test: ${arg}")
		endif()
	endforeach()

	add_test(NAME "${test_name}" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${test_cmd})
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${binary_name}.tap")
endfunction()

enable_testing()

s2opc_embed_address_space(TEST_ADDRESS_SPACE "${CMAKE_SOURCE_DIR}/tests/data/address_space/ingopcs.xml")

# Helpers unit tests
file(GLOB_RECURSE INTERNAL_TESTS_SRCS "tests/helpers/*.c")
add_executable(check_helpers ${INTERNAL_TESTS_SRCS})
target_include_directories(check_helpers PRIVATE ${INGOPCS_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
target_link_libraries(check_helpers PRIVATE ${INGOPCS_STATIC_LIB} ${CHECK_LIBRARIES})

s2opc_unit_test(check_helpers)

## Copy reference logs files to bin/ directory
add_custom_command(
    TARGET check_helpers POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logAnother.ref ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logCircular.ref1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logCircular.ref2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logTest.ref1 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logTest.ref2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/logger/logTest.ref3 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Server test: local services
add_executable(toolkit_test_server_local_service "tests/services/toolkit_test_server_local_service.c"
                                                 "tests/services/wrap_read.c"
                                                 "tests/services/testlib_write.c"
                                                 "tests/services/test_results.c"
                                                 "tests/services/testlib_read_response.c"
                                                 ${TEST_ADDRESS_SPACE})
target_link_libraries(toolkit_test_server_local_service PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(toolkit_test_server_local_service PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                                     ${INGOPCS_PRIVATE_INCLUDES})
s2opc_unit_test(toolkit_test_server_local_service GENERATE_TAP)

# Client tests
add_executable(toolkit_test_client "tests/services/toolkit_test_client.c"
                                   "tests/services/wrap_read.c"
                                   "tests/services/testlib_write.c"
                                   "tests/services/test_results.c"
                                   "tests/services/testlib_read_response.c"
                                   ${TEST_ADDRESS_SPACE})
target_link_libraries(toolkit_test_client PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(toolkit_test_client PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                       ${INGOPCS_PRIVATE_INCLUDES})

add_executable(toolkit_test_suite_client "tests/demo/test_suite_client.c"
                                         "tests/demo/test_discovery.c"
                                         "tests/demo/config.c"
                                         "tests/demo/state_machine.c")
target_link_libraries(toolkit_test_suite_client PRIVATE ${INGOPCS_STATIC_LIB} ${CHECK_LIBRARIES})
target_include_directories(toolkit_test_suite_client PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                             ${INGOPCS_PRIVATE_INCLUDES}
                                                             ${CHECK_INCLUDE_DIRS})

# Server test
add_executable(toolkit_test_server "tests/services/toolkit_test_server.c" ${TEST_ADDRESS_SPACE})
target_link_libraries(toolkit_test_server PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(toolkit_test_server PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                       ${INGOPCS_PRIVATE_INCLUDES})

if (EXPAT_FOUND)
	target_link_libraries(toolkit_test_server PRIVATE ingopcs-loader-uanodeset-expat)
	set_target_properties(toolkit_test_server PROPERTIES COMPILE_DEFINITIONS "WITH_EXPAT")
endif()

add_test(NAME "validation::client_server_test" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap" "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-toolkit-test-server" "./toolkit_test_client${FILE_EXT}")
set_tests_properties("validation::client_server_test" PROPERTIES ENVIRONMENT "TEST_SERVER_CMD=./toolkit_test_server${FILE_EXT};CK_TAP_LOG_FILE_NAME=client_server_test.tap")

# Demo clients
add_executable(ingopcs_browse "tests/demo/ingopcs_browse.c"
                              "tests/demo/state_machine.c"
                              "tests/demo/config.c"
                              )
target_link_libraries(ingopcs_browse PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(ingopcs_browse PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                  ${INGOPCS_PRIVATE_INCLUDES})

add_executable(ingopcs_read "tests/demo/ingopcs_read.c"
                            "tests/demo/state_machine.c"
                            "tests/demo/config.c"
                            )
target_link_libraries(ingopcs_read PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(ingopcs_read PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                ${INGOPCS_PRIVATE_INCLUDES})

add_executable(ingopcs_write "tests/demo/ingopcs_write.c"
                             "tests/demo/state_machine.c"
                             "tests/demo/config.c"
                             )
target_link_libraries(ingopcs_write PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(ingopcs_write PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                 ${INGOPCS_PRIVATE_INCLUDES})

add_executable(ingopcs_discovery "tests/demo/ingopcs_discovery.c"
                                 "tests/demo/state_machine.c"
                                 "tests/demo/config.c"
                                 )
target_link_libraries(ingopcs_discovery PRIVATE ${INGOPCS_STATIC_LIB} ingopcs-loader-embedded)
target_include_directories(ingopcs_discovery PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                     ${INGOPCS_PRIVATE_INCLUDES})

add_executable(ingopcs_findserver "tests/demo/ingopcs_findserver.c"
                                  "tests/demo/state_machine.c"
                                  "tests/demo/config.c"
                                   )
target_link_libraries(ingopcs_findserver PRIVATE ${INGOPCS_STATIC_LIB})
target_include_directories(ingopcs_findserver PRIVATE "${ADDSPACEGEN_INCLUDE_DIR}"
                                                       ${INGOPCS_PUBLIC_INCLUDES}
                                                       ${INGOPCS_PRIVATE_INCLUDES})

## COPY ALL CERTIFICATES TO BIN/ DIRECTORY
add_custom_command(
    TARGET toolkit_test_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/revoked
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/untrusted
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/client_2k.pem
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/server_2k.pem
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add 4096 client / server keys
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_4k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_4k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_4k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_4k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/)

# Sockets layer (stubbed) tests
file(GLOB_RECURSE SOCKETS_SRC
    ${PLATFORM_SRCS}
    "csrc/helpers/*.c"
    "csrc/sockets/*.c"
)

file(GLOB_RECURSE SOCKETS_TEST
     "tests/sockets/*.c")

add_executable(check_sockets ${SOCKETS_SRC}
                             ${SOCKETS_TEST})
target_include_directories(check_sockets PRIVATE "tests/sockets/stubs"
                                                 ${INGOPCS_PUBLIC_INCLUDES}
                                                 ${INGOPCS_PRIVATE_INCLUDES}
                                                 ${CHECK_INCLUDE_DIRS})
target_link_libraries(check_sockets PRIVATE ${CHECK_LIBRARIES} ${INGOPCS_STATIC_DEP_LIBS})
s2opc_unit_test(check_sockets)

# Secure channels (stubbed on services side) tests
file(GLOB_RECURSE SECURE_CHANNELS_SRC
    "csrc/configuration/sopc_ieee_check.c"
    "csrc/secure_channels/*.c"
    "csrc/opcua_types/*.c"
    "csrc/crypto/*.c"
)

file(GLOB_RECURSE SECURE_CHANNELS_CLIENT_TEST
     "tests/secure_channels/test_secure_channels_client.c"
     "tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c"
     "tests/secure_channels/stubs/stub_sc_sopc_services_api.c")

file(GLOB_RECURSE SECURE_CHANNELS_SERVER_TEST
     "tests/secure_channels/test_secure_channels_server.c"
     "tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c"
     "tests/secure_channels/stubs/stub_sc_sopc_services_api.c")

# Secure channels tests: CLIENT executable
add_executable(test_secure_channels_client ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_CLIENT_TEST})

target_include_directories(test_secure_channels_client PRIVATE "tests/secure_channels/stubs"
                                                               ${INGOPCS_PUBLIC_INCLUDES}
                                                               ${INGOPCS_PRIVATE_INCLUDES}
                                                               ${MBEDTLS_INCLUDE_DIRS})
target_link_libraries(test_secure_channels_client PRIVATE ${INGOPCS_STATIC_DEP_LIBS})

# Secure channels tests: SERVER executable
add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_SERVER_TEST})
target_link_libraries(test_secure_channels_server PRIVATE ${INGOPCS_STATIC_DEP_LIBS})

target_include_directories(test_secure_channels_server PRIVATE "tests/secure_channels/stubs"
                                                               ${INGOPCS_PUBLIC_INCLUDES}
                                                               ${INGOPCS_PRIVATE_INCLUDES}
                                                               ${MBEDTLS_INCLUDE_DIRS})

# Secure channels tests: tests on received buffer
file(GLOB_RECURSE SECURE_CHANNELS_BUFFER_TEST
     ${PLATFORM_SRCS}
     "csrc/helpers/*.c"
     "tests/secure_channels/check_sc_rcv_buffer.c"
     "tests/helpers/hexlify.c"
     "tests/secure_channels/stubs/*.c")

add_executable(check_sc_rcv_buffer ${SECURE_CHANNELS_SRC}
                                               ${SECURE_CHANNELS_BUFFER_TEST})
target_link_libraries(check_sc_rcv_buffer PRIVATE ${CHECK_LIBRARIES} ${INGOPCS_STATIC_DEP_LIBS})

target_include_directories(check_sc_rcv_buffer PRIVATE "tests/secure_channels/stubs"
                                                       "tests/helpers"
                                                       ${INGOPCS_PUBLIC_INCLUDES}
                                                       ${INGOPCS_PRIVATE_INCLUDES}
                                                       ${MBEDTLS_INCLUDE_DIRS}
                                                       ${CHECK_INCLUDE_DIRS})
s2opc_unit_test(check_sc_rcv_buffer)

# Secure channels tests: tests on received encrypted buffer
file(GLOB_RECURSE SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST
     ${PLATFORM_SRCS}
     "csrc/helpers/*.c"
     "tests/secure_channels/check_sc_rcv_encrypted_buffer.c"
     "tests/helpers/hexlify.c"
     "tests/secure_channels/stubs/*.c")

add_executable(check_sc_rcv_encrypted_buffer ${SECURE_CHANNELS_SRC}
                                             ${SECURE_CHANNELS_ENCRYPTED_BUFFER_TEST})
target_link_libraries(check_sc_rcv_encrypted_buffer PRIVATE ${CHECK_LIBRARIES} ${INGOPCS_STATIC_DEP_LIBS})

target_include_directories(check_sc_rcv_encrypted_buffer PRIVATE "tests/secure_channels/stubs"
                                                                 "tests/helpers"
                                                                 ${INGOPCS_PUBLIC_INCLUDES}
                                                                 ${INGOPCS_PRIVATE_INCLUDES}
                                                                 ${MBEDTLS_INCLUDE_DIRS}
                                                                 ${CHECK_INCLUDE_DIRS})
s2opc_unit_test(check_sc_rcv_encrypted_buffer)

# Validation tests

function(s2opc_server_validation_test name)
	set(test_name "validation::${name}")
	add_test(NAME "${test_name}" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-toolkit-test-server" "${PYTHON_BIN}" "../validation/${name}")
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "TEST_SERVER_CMD=./toolkit_test_server${FILE_EXT}")
endfunction()

function(s2opc_client_validation_test name)
	set(test_name "validation::${name}")
	add_test(NAME "${test_name}" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-freeopcua-server" "./${name}")
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${name}.tap")
endfunction()

s2opc_server_validation_test("client.py")
s2opc_server_validation_test("client_sc_renew.py")
s2opc_server_validation_test("client_session_timeout.py")
s2opc_server_validation_test("client_sc_establish_timeout.py")
s2opc_client_validation_test("toolkit_test_suite_client")

function(s2opc_sc_level_test name server_key_size)
	set(test_name "validation::secure_channel_level::${name}")
	add_test(NAME "${test_name}" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap" "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-toolkit-test-server" "./test_secure_channels_client${FILE_EXT}" ${ARGN})
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "TEST_SERVER_CMD=./test_secure_channels_server ${server_key_size};CK_TAP_LOG_FILE_NAME=secure_channel_level_${name}.tap;TEST_SERVER_EXITS_ALONE=1")
endfunction()

s2opc_sc_level_test("SignAndEncrypt_B256Sha256_2048bit" "2048")
s2opc_sc_level_test("Sign_B256Sha256_2048bit" "2048" "sign")
s2opc_sc_level_test("None" "2048" "none")
s2opc_sc_level_test("Sign_B256_2048bit" "2048" "encrypt" "B256")
s2opc_sc_level_test("SignAndEncrypt_B256Sha256_4096bit" "4096" "encrypt" "B256Sha256" "4096")
s2opc_sc_level_test("SignAndEncrypt_B256Sha256_4096bit_server_vs_2048bit_client" "4096" "encrypt" "B256Sha256" "2048" "4096")
s2opc_sc_level_test("SignAndEncrypt_B256Sha256_2048bit_server_vs_4096bit_client" "2048" "encrypt" "B256Sha256" "4096" "2048")

# Benchmarks
add_executable(bench_reads "tests/benchmarks/bench_reads.c")
set_source_files_properties("tests/benchmarks/bench_reads.c" PROPERTIES COMPILE_FLAGS "-D_XOPEN_SOURCE=600")
target_link_libraries(bench_reads PRIVATE ${INGOPCS_STATIC_LIB})
target_include_directories(bench_reads PRIVATE ${INGOPCS_PUBLIC_INCLUDES} ${INGOPCS_PRIVATE_INCLUDES})

if (EXPAT_FOUND)
	add_executable(ingopcs_parse_uanodeset "tests/demo/ingopcs_parse_uanodeset.c")
	target_link_libraries(ingopcs_parse_uanodeset PRIVATE ingopcs "ingopcs-loader-uanodeset-expat")
	target_include_directories(ingopcs_parse_uanodeset PRIVATE ${INGOPCS_PUBLIC_INCLUDES})
endif()
