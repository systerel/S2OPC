cmake_minimum_required (VERSION 2.8.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

project (INGOPCS LANGUAGES C)
include(CheckCCompilerFlag)

# compatibility with CMake 3.1
if(POLICY CMP0054)
  # http://www.cmake.org/cmake/help/v3.1/policy/CMP0054.html
  # See the discussion https://github.com/CGAL/cgal/issues/189
  cmake_policy(SET CMP0054 NEW)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit -Wreturn-type -Wsequence-point -Wcast-qual -Wuninitialized -Wcast-align -Wstrict-prototypes -Wchar-subscripts -Wformat-security -Wformat -Wconversion")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wunicode -Wimplicit-int -Wreserved-id-macro -Wsometimes-uninitialized -Wunsequenced -Wincompatible-pointer-types-discards-qualifiers -Wunevaluated-expression -Wparentheses -Wint-conversion -Wint-to-pointer-cast -Wincompatible-pointer-types -Wvla -Wconversion")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /Zi /sdl")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
else()
  message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	# TODO for MSVC
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
else()
	# Compiler options (C99 compliant)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")

	# Define compiler options for each mode.
	# Default optimization level is -O0 for debug and -O2 for others modes.
	# -f-no-omit-frame-pointer is set to use Perf profiling tool,
	# it is not set in Release mode.
	# -g option is already set in Debug and Debugwithdebinfo modes.
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")
endif()

add_definitions(-DSOPC_PTR_SIZE=${CMAKE_SIZEOF_VOID_P})

# check if compiler support new sanitization options
set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address")
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-compare" COMPILER_SUPPORTS_SAN_PC)
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-subtract" COMPILER_SUPPORTS_SAN_PS)
unset(CMAKE_REQUIRED_LIBRARIES)

option(WITH_ASAN "build with ASAN" OFF)
if(WITH_ASAN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  if(COMPILER_SUPPORTS_SAN_PC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=pointer-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-compare")
  endif()
  if(COMPILER_SUPPORTS_SAN_PS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=pointer-subtract")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-subtract")
  endif()
endif()

option(WITH_TSAN "build with TSAN" OFF)
if(WITH_TSAN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -pie")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -fPIC")
endif()

option(WITH_UBSAN "build with UBSAN" OFF)
if(WITH_UBSAN)
  add_definitions(-DROCKSDB_UBSAN_RUN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
endif()

if(WITH_ASAN AND WITH_TSAN)
  message(FATAL_ERROR "ASan and TSan cannot be used at the same time")
endif()

option(WITH_COVERAGE "build with COVERAGE" OFF)
if(WITH_COVERAGE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

option(WITH_GPERF_PROFILER "link against the gperftool profiler")
if(WITH_GPERF_PROFILER)
	find_library(GPERF_PROFILER profiler)

	if (NOT GPERF_PROFILER)
		message(FATAL_ERROR "Could not find libprofiler")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
endif()

option(WITH_CLANG_SOURCE_COVERAGE "build with Clang source coverage" OFF)
if (WITH_CLANG_SOURCE_COVERAGE)
	if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
		message(FATAL_ERROR "Clang is required to enable Clang source coverage")
	endif()

	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
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

# minimum Vista for IPV6 support
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/DWIN32_LEAN_AND_MEAN)
  add_definitions(/DWIN32_WINNT=0x0600)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
# it shall be MinGw
elseif(MINGW)
  add_definitions(-D_WIN32_WINNT=0x0600)
endif()

add_library(ingopcs $<TARGET_OBJECTS:ingopcs_obj>)
target_include_directories(ingopcs PUBLIC ${INGOPCS_PUBLIC_INCLUDES})
target_link_libraries(ingopcs PUBLIC ${INGOPCS_DEP_LIBS})

if(MINGW)
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
    if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
        set_source_files_properties(${c_file_path} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
    endif()

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
			set(test_cmd "${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py" "${test_cmd}")
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

add_test(NAME "validation::client_server_test"
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND
	"${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py"
	"${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
	"--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	"--server-cmd" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/toolkit_test_server${FILE_EXT}"
	"./toolkit_test_client${FILE_EXT}")
set_tests_properties("validation::client_server_test" PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=client_server_test.tap")

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
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_2k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add 4096 client / server keys
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/client_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_4k_key.pem ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/tests/data/cert/server_4k_cert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/)

# Sockets layer (stubbed) tests
file(GLOB_RECURSE SOCKETS_SRC
    ${PLATFORM_SRCS}
    "csrc/helpers/*.c"
    "csrc/opcua_types/sopc_builtintypes.c"
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
	add_test(NAME "${test_name}"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/validation
		COMMAND
		"${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
		"--server-wd" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		"--server-cmd" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/toolkit_test_server${FILE_EXT}"
		"${PYTHON_BIN}" ${name})
endfunction()

function(s2opc_client_validation_test name)
	set(test_name "validation::${name}")
	add_test(NAME "${test_name}"
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		COMMAND
		"${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
		"--server-wd" "${CMAKE_SOURCE_DIR}/validation"
		"--server-cmd" "${PYTHON_BIN} ${CMAKE_SOURCE_DIR}/validation/server.py 25000"
		"./${name}")
	set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CK_TAP_LOG_FILE_NAME=${name}.tap")
endfunction()

s2opc_server_validation_test("client.py")
s2opc_server_validation_test("client_sc_renew.py")
s2opc_server_validation_test("client_session_timeout.py")
s2opc_server_validation_test("client_sc_establish_timeout.py")
s2opc_client_validation_test("toolkit_test_suite_client")

function(s2opc_sc_level_test name server_key_size)
	set(test_name "validation::secure_channel_level::${name}")
	add_test(NAME "${test_name}"
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		COMMAND
		"${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/tap-wrap.py"
		"${PYTHON_BIN}" "${CMAKE_SOURCE_DIR}/tests/scripts/with-opc-server.py"
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

# Benchmarks
add_executable(bench_tool "tests/benchmarks/bench_tool.c")
target_link_libraries(bench_tool PRIVATE ${INGOPCS_STATIC_LIB})
target_include_directories(bench_tool PRIVATE ${INGOPCS_PUBLIC_INCLUDES} ${INGOPCS_PRIVATE_INCLUDES})

if (EXPAT_FOUND)
	add_executable(ingopcs_parse_uanodeset "tests/demo/ingopcs_parse_uanodeset.c")
	target_link_libraries(ingopcs_parse_uanodeset PRIVATE ingopcs "ingopcs-loader-uanodeset-expat")
	target_include_directories(ingopcs_parse_uanodeset PRIVATE ${INGOPCS_PUBLIC_INCLUDES})
endif()

# Fuzzing
function(s2opc_fuzzer name src_file)
	set(standalone_target_name "${name}.standalone")
	set(fuzz_target_name "${name}.libfuzzer")

	add_executable(${standalone_target_name} "tests/fuzzing/${src_file}" "tests/fuzzing/standalone_fuzzer.c")
	target_include_directories(${standalone_target_name} PRIVATE ${INGOPCS_PUBLIC_INCLUDES} ${INGOPCS_PRIVATE_INCLUDES})
	target_link_libraries(${standalone_target_name} ${INGOPCS_STATIC_LIB})

	add_executable(${fuzz_target_name} "tests/fuzzing/${src_file}")
	set_target_properties(${fuzz_target_name} PROPERTIES COMPILE_FLAGS "-fsanitize=fuzzer")
	set_target_properties(${fuzz_target_name} PROPERTIES LINK_FLAGS "-fsanitize=fuzzer")
	target_include_directories(${fuzz_target_name} PRIVATE ${INGOPCS_PUBLIC_INCLUDES} ${INGOPCS_PRIVATE_INCLUDES})
	target_link_libraries(${fuzz_target_name} ${INGOPCS_STATIC_LIB})
endfunction()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS "6.0.0")
		# We need at least CLang 6 to get LibFuzzer
		message(STATUS "Your version of Clang (${CMAKE_C_COMPILER_ID}) is not recent enough to have LibFuzzer")
	else()
		s2opc_fuzzer(server_request_fuzzer "fuzz_server_request.c")
		s2opc_fuzzer(parse_tcp_uri_fuzzer "fuzz_parse_tcp_uri.c")
	endif()
endif()

###### Subscribing client library and binary ######

# Library, always static
set(LIBSUBSCRIBE_PATH "tests/client_subscription")
add_library(libsubscribe STATIC "${LIBSUBSCRIBE_PATH}/libs2opc_client.c")
add_dependencies(libsubscribe ingopcs)
target_link_libraries(libsubscribe PRIVATE ingopcs) # ${INGOPCS_DEP_LIBS})
target_include_directories(libsubscribe PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                ${INGOPCS_PRIVATE_INCLUDES})
set_target_properties(libsubscribe PROPERTIES PUBLIC_HEADER "${LIBSUBSCRIBE_PATH}/libs2opc_client.h")
install(TARGETS libsubscribe
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

# Demo client
add_executable(libsubscribe_client "${LIBSUBSCRIBE_PATH}/client.c"
                                   "${LIBSUBSCRIBE_PATH}/toolkit_helpers.c"
                                   "${LIBSUBSCRIBE_PATH}/state_machine.c"
                                   )
target_link_libraries(libsubscribe_client PRIVATE libsubscribe)
#target_include_directories(libsubscribe_client PRIVATE ${LIBSUBSCRIBE_PATH})
