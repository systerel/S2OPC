cmake_minimum_required (VERSION 2.8.8)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

project (S2OPC LANGUAGES C)
include(CheckCCompilerFlag)
include (CheckTypeSize)

# compatibility with CMake 3.1
if(POLICY CMP0054)
  # http://www.cmake.org/cmake/help/v3.1/policy/CMP0054.html
  # See the discussion https://github.com/CGAL/cgal/issues/189
  cmake_policy(SET CMP0054 NEW)
endif()

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

find_package(Doxygen)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit -Wreturn-type -Wsequence-point -Wcast-qual -Wuninitialized -Wcast-align -Wstrict-prototypes -Wchar-subscripts -Wformat=2 -Wconversion -Wshadow -Wmissing-prototypes")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wunreachable-code")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunicode -Wimplicit-int -Wreserved-id-macro -Wsometimes-uninitialized -Wunsequenced -Wincompatible-pointer-types-discards-qualifiers -Wunevaluated-expression -Wparentheses -Wint-conversion -Wint-to-pointer-cast -Wincompatible-pointer-types -Wvla -Wconversion")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /Zi /sdl")
else()
  message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

if(MINGW)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pedantic-ms-format")
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
	# -fPIC option is added to produce position independent code, which is reusable in dynamic libraries.
	# It is not set in Release mode.
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fPIC")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fPIC")
endif()

add_definitions(-DSOPC_PTR_SIZE=${CMAKE_SIZEOF_VOID_P})
check_type_size("time_t" CMAKE_TIME_T_SIZE)
add_definitions(-DSOPC_TIME_T_SIZE=${CMAKE_TIME_T_SIZE})

# check if compiler support new sanitization options
set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address")
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-compare" COMPILER_SUPPORTS_SAN_PC)
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-subtract" COMPILER_SUPPORTS_SAN_PS)
unset(CMAKE_REQUIRED_LIBRARIES)

option(WITH_ASAN "build with ASAN" OFF)
if(WITH_ASAN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  if(COMPILER_SUPPORTS_SAN_PC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=pointer-compare -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-compare -fno-omit-frame-pointer")
  endif()
  if(COMPILER_SUPPORTS_SAN_PS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=pointer-subtract -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=pointer-subtract -fno-omit-frame-pointer")
  endif()
endif()

option(WITH_TSAN "build with TSAN" OFF)
if(WITH_TSAN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -pie -fno-omit-frame-pointer")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -fno-omit-frame-pointer -fPIC")
endif()

option(WITH_UBSAN "build with UBSAN" OFF)
if(WITH_UBSAN)
  add_definitions(-DROCKSDB_UBSAN_RUN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
endif()

if(WITH_ASAN AND WITH_TSAN)
  message(FATAL_ERROR "ASan and TSan cannot be used at the same time")
endif()

option(WITH_COVERAGE "build with COVERAGE" OFF)
if(WITH_COVERAGE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

option(WITH_COVERITY "set this flag when built with coverity" OFF)

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

option(ENABLE_TESTING "Build the S2OPC tests" ON)
option(FORCE_CHECK_STATIC_LIB "Force the use of static check lib" ON)

option(WARNINGS_AS_ERRORS "Treat warnings as errors when building" ON)

if(WARNINGS_AS_ERRORS)
	if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
	else()
	  message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
	endif()
endif()

option(WITH_OSS_FUZZ "Add the fuzzers target when building for OSS-Fuzz" OFF)
if(WITH_OSS_FUZZ)
  if(WITH_ASAN OR WITH_UBSAN OR WITH_TSAN OR WITH_COVERAGE OR WITH_COVERITY OR WITH_CLANG_SOURCE_COVERAGE)
    message(FATAL_ERROR "Advanced WITH_ options not available when building WITH_OSS_FUZZ")
  endif()
endif()

option(WITH_PYS2OPC "Also builds PyS²OPC" OFF)

if(WITH_PYS2OPC AND (WITH_ASAN OR WITH_UBSAN OR WITH_TSAN))
  message(FATAL_ERROR "Sanitizers are known to not work (yet) with WITH_PYS2OPC.")
endif()
if(WITH_PYS2OPC AND (WITH_COVERAGE OR WITH_COVERITY OR WITH_CLANG_SOURCE_COVERAGE))
  message(WARNING "PyS²OPC relies on CFFI code generation, which may bend code analysis results.")
endif()

option(WITH_STATIC_SECURITY_DATA "Use static security data for test" OFF)

option(WITH_NANO_EXTENDED "Use Nano profile with additional services out of Nano scope" OFF)

# Re-enable asserts for Release and RelWithDebInfo builds
string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})

# Source files of S2OPC library

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
  set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_GNU_SOURCE")
endif()

if(CMAKE_HOST_UNIX)
  add_custom_command(OUTPUT csrc/configuration/sopc_toolkit_build_info.h
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/csrc/configuration/sopc_toolkit_build_info.h_ ${CMAKE_SOURCE_DIR}/csrc/configuration/sopc_toolkit_build_info.h
                     COMMAND ${CMAKE_SOURCE_DIR}/scripts/gen_build_info_file.sh ${CMAKE_SOURCE_DIR}
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
if(ENABLE_TESTING)
	if(FORCE_CHECK_STATIC_LIB)
		set(CHECK_USE_STATIC_LIBS 1)
	else()
		set(CHECK_USE_STATIC_LIBS 0)
	endif()
	find_package(Check REQUIRED)
endif()

# Define non-platform dependent source files
file(GLOB_RECURSE S2OPC_SRCS
    "csrc/api_toolkit/*.c"
    "csrc/configuration/*.c"
    "csrc/helpers/*.c"
    "csrc/crypto/*.c"
    "csrc/opcua_types/*.c"
    "csrc/sockets/*.c"
    "csrc/secure_channels/*.c"
    "csrc/services/*.c")

# Define S2OPC library includes
file(GLOB S2OPC_INCLUDE_FILES
    "csrc/api_toolkit/*.h"
    "csrc/configuration/*.h"
    "csrc/helpers/*.h"
    "csrc/opcua_types/*.h"
    "csrc/crypto/*.h"
    "csrc/helpers_platform_dep/*.h"
    "csrc/services/b2c/*.h"
)

set(S2OPC_PUBLIC_INCLUDES
    "csrc/api_toolkit"
    "csrc/configuration"
    "csrc/helpers"
    "csrc/opcua_types"
    "csrc/crypto"
    "csrc/helpers_platform_dep"
    "csrc/helpers_platform_dep/${PLATFORM_DIR}"
    "csrc/services/b2c"
)

set(S2OPC_PRIVATE_INCLUDES
    "csrc/sockets"
    "csrc/secure_channels"
    "csrc/services"
    "csrc/crypto/mbedtls"
    "csrc/services/bgenc"
)

# Define S2OPC library dependencies
set(S2OPC_DEP_LIBS ${MBEDTLS_LIBRARIES} ${MATH_LIB} ${THREAD_LIB} ${SOCK_LIB})
set(S2OPC_STATIC_DEP_LIBS ${MBEDTLS_STATIC_LIBRARIES} ${MATH_LIB} ${THREAD_LIB} ${SOCK_LIB})

add_library(s2opc_obj OBJECT ${PLATFORM_SRCS} ${S2OPC_SRCS})
target_include_directories(s2opc_obj BEFORE
	PUBLIC  ${S2OPC_PUBLIC_INCLUDES}
	PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS}
)
add_dependencies(s2opc_obj build_info)

# Needed if we want to use the objects in a shared library
set_property(TARGET s2opc_obj PROPERTY POSITION_INDEPENDENT_CODE 1)

# Manage the Nano profiles extended mode
if (WITH_NANO_EXTENDED)
  target_compile_definitions(s2opc_obj PRIVATE "WITH_NANO_EXTENDED")
  set(TOOLKIT_TEST_SERVER "toolkit_test_server")
else()
  set(TOOLKIT_TEST_SERVER "toolkit_test_nano_server")
endif()

# minimum Vista for IPV6 support
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/DWIN32_LEAN_AND_MEAN)
  add_definitions(/D_WIN32_WINNT=0x0600)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
elseif(MINGW)
  add_definitions(-D_WIN32_WINNT=0x0600)
endif()

add_library(s2opc $<TARGET_OBJECTS:s2opc_obj>)
target_include_directories(s2opc PUBLIC ${S2OPC_PUBLIC_INCLUDES})
target_link_libraries(s2opc PUBLIC ${S2OPC_DEP_LIBS})

if(MINGW)
    # always link libc statically (even in case of shared library)
    target_link_libraries(s2opc PRIVATE -static-libgcc)
endif()

# Define installation targets
## retrieve public include files
foreach(dir ${S2OPC_PUBLIC_INCLUDES})
    file(GLOB temp_files "${dir}/*.h")
    list(APPEND S2OPC_INCLUDE_FILES ${temp_files})
endforeach()
## set public header property
set_target_properties(s2opc PROPERTIES PUBLIC_HEADER "${S2OPC_INCLUDE_FILES}")
## install files
install(TARGETS s2opc
        LIBRARY DESTINATION lib # Supposedly dynamic lib
        ARCHIVE DESTINATION lib # Supposedly static lib
        PUBLIC_HEADER DESTINATION include
)

install(FILES ${CMAKE_SOURCE_DIR}/scripts/generate-s2opc-address-space.py DESTINATION bin)

# Address space loader backends

if(${WITH_CONST_ADDSPACE})
  set(const_addspace "--const_addspace")
endif()

add_library(s2opc-loader-embedded STATIC csrc/loaders/address_space_loaders/embedded/sopc_addspace_loader.c)
target_link_libraries(s2opc-loader-embedded PRIVATE s2opc)
target_include_directories(s2opc-loader-embedded PRIVATE ${S2OPC_PUBLIC_INCLUDES} PUBLIC "csrc/loaders/address_space_loaders")
set_target_properties(s2opc-loader-embedded PROPERTIES PUBLIC_HEADER "csrc/loaders/address_space_loaders/embedded/sopc_addspace_loader.h")
install(TARGETS s2opc-loader-embedded
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION "include/embedded")

function(s2opc_embed_address_space c_file_name xml_uanodeset_path)

    add_custom_command(
        OUTPUT ${c_file_name}
        DEPENDS ${xml_uanodeset_path}
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/generate-s2opc-address-space.py ${xml_uanodeset_path} ${c_file_name} ${const_addspace}
        COMMENT "Generating address space ${c_file_name}"
        VERBATIM
    )

    set_source_files_properties(${c_file_name} PROPERTIES GENERATED TRUE)
    if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
        set_source_files_properties(${c_file_name} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
    endif()

endfunction()

#find_package(EXPAT)

if (EXPAT_FOUND)
  add_library(s2opc-xml-loaders-expat STATIC csrc/loaders/helpers/sopc_helper_expat.c
                                             csrc/loaders/address_space_loaders/xml_expat/sopc_uanodeset_loader.c
                                             csrc/loaders/config_loaders/xml_expat/sopc_config_loader.c)
  target_link_libraries(s2opc-xml-loaders-expat PUBLIC ${EXPAT_LIBRARIES} PRIVATE s2opc)
  target_include_directories(s2opc-xml-loaders-expat PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${EXPAT_INCLUDE_DIRS} PUBLIC "csrc/loaders/address_space_loaders"
                                                                                                                   "csrc/loaders/config_loaders"
                                                                                                                   "csrc/loaders/helpers")
  target_compile_definitions(s2opc-xml-loaders-expat PRIVATE "UANODESET_LOADER_LOG")
  set_property(TARGET s2opc-xml-loaders-expat PROPERTY PUBLIC_HEADER "csrc/loaders/address_space_loaders/xml_expat/sopc_uanodeset_loader.h"
                                                                     "csrc/loaders/config_loaders/xml_expat/sopc_config_loader.h")
  install(TARGETS s2opc-xml-loaders-expat
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION "include/xml_expat")
endif()

# S2OPC tests ################################################################################

if(WIN32)
    set(FILE_EXT .exe)
    set(ENV_PATH_SEP "\;")
else()
    set(ENV_PATH_SEP ":")
endif()

if(ENABLE_TESTING)

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

enable_testing()

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

# Helpers unit tests
file(GLOB_RECURSE INTERNAL_TESTS_SRCS "tests/helpers/*.c")
add_executable(check_helpers ${INTERNAL_TESTS_SRCS} ${TEST_CHECK_HELPERS_ADDRESS_SPACE_C})
target_include_directories(check_helpers PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
target_link_libraries(check_helpers PRIVATE s2opc ${CHECK_LIBRARIES} s2opc-loader-embedded)
if (EXPAT_FOUND)
	target_link_libraries(check_helpers PRIVATE s2opc-xml-loaders-expat)
	target_compile_definitions(check_helpers PRIVATE "WITH_EXPAT")
endif()
# Manage the Nano profiles extended mode
if (WITH_NANO_EXTENDED)
  target_compile_definitions(check_helpers PRIVATE "WITH_NANO_EXTENDED")
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

# Server test: local services
add_executable(toolkit_test_server_local_service "tests/services/toolkit_test_server_local_service.c"
                                                    "tests/services/wrap_read.c"
                                                    "tests/services/testlib_write.c"
                                                    "tests/services/test_results.c"
                                                    "tests/services/testlib_read_response.c"
                                                    ${TEST_LOCAL_DEMO_SERVER_ADDRESS_SPACE_C})
target_link_libraries(toolkit_test_server_local_service PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(toolkit_test_server_local_service PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                                        ${S2OPC_PRIVATE_INCLUDES})
s2opc_unit_test(toolkit_test_server_local_service GENERATE_TAP)

# Client tests
add_executable(toolkit_test_client "tests/services/toolkit_test_client.c"
                                   "tests/services/wrap_read.c"
                                   "tests/services/testlib_write.c"
                                   "tests/services/test_results.c"
                                   "tests/services/testlib_read_response.c"
                                   ${TEST_DEMO_CLIENT_ADDRESS_SPACE_C})
# Manage the Nano profiles extended mode
if (WITH_NANO_EXTENDED)
  target_compile_definitions(toolkit_test_client PRIVATE "WITH_NANO_EXTENDED")
endif()

target_link_libraries(toolkit_test_client PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(toolkit_test_client PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                       ${S2OPC_PRIVATE_INCLUDES})


add_executable(toolkit_test_client_service_faults "tests/services/toolkit_test_client_service_faults.c")
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

# Server test
add_executable(${TOOLKIT_TEST_SERVER}
    "tests/services/runtime_variables.c"
    "tests/services/toolkit_test_server.c"
    ${TEST_DEMO_SERVER_ADDRESS_SPACE_C})
target_link_libraries(${TOOLKIT_TEST_SERVER} PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(${TOOLKIT_TEST_SERVER} PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                          ${S2OPC_PRIVATE_INCLUDES})

if (WITH_NANO_EXTENDED)
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_NANO_EXTENDED")
endif()
if (EXPAT_FOUND)
  target_link_libraries(${TOOLKIT_TEST_SERVER} PRIVATE s2opc-xml-loaders-expat)
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_EXPAT")
endif()

if (WITH_STATIC_SECURITY_DATA)
  target_compile_definitions(${TOOLKIT_TEST_SERVER} PRIVATE "WITH_STATIC_SECURITY_DATA")
endif()

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

# Demo clients
add_executable(s2opc_browse "tests/demo/s2opc_browse.c"
                              "tests/demo/state_machine.c"
                              "tests/demo/config.c"
                              )
target_link_libraries(s2opc_browse PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(s2opc_browse PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                  ${S2OPC_PRIVATE_INCLUDES})

add_executable(s2opc_read "tests/demo/s2opc_read.c"
                            "tests/demo/state_machine.c"
                            "tests/demo/config.c"
                            )
target_link_libraries(s2opc_read PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(s2opc_read PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                ${S2OPC_PRIVATE_INCLUDES})

add_executable(s2opc_write "tests/demo/s2opc_write.c"
                             "tests/demo/state_machine.c"
                             "tests/demo/config.c"
                             )
target_link_libraries(s2opc_write PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(s2opc_write PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                 ${S2OPC_PRIVATE_INCLUDES})

add_executable(s2opc_discovery "tests/demo/s2opc_discovery.c"
                                 "tests/demo/state_machine.c"
                                 "tests/demo/config.c"
                                 )
target_link_libraries(s2opc_discovery PRIVATE s2opc s2opc-loader-embedded)
target_include_directories(s2opc_discovery PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                     ${S2OPC_PRIVATE_INCLUDES})

add_executable(s2opc_findserver "tests/demo/s2opc_findserver.c"
                                  "tests/demo/state_machine.c"
                                  "tests/demo/config.c"
                                   )
target_link_libraries(s2opc_findserver PRIVATE s2opc)
target_include_directories(s2opc_findserver PRIVATE "${ADDSPACEGEN_INCLUDE_DIR}"
                                                       ${S2OPC_PUBLIC_INCLUDES}
                                                       ${S2OPC_PRIVATE_INCLUDES})

add_executable(s2opc_register "tests/demo/s2opc_register.c"
                                  "tests/demo/state_machine.c"
                                  "tests/demo/config.c"
                                   )
target_link_libraries(s2opc_register PRIVATE s2opc)
target_include_directories(s2opc_register PRIVATE "${ADDSPACEGEN_INCLUDE_DIR}"
                                                       ${S2OPC_PUBLIC_INCLUDES}
                                                       ${S2OPC_PRIVATE_INCLUDES})
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

# Sockets layer tests
add_executable(check_sockets "tests/sockets/check_sockets.c")
target_include_directories(check_sockets PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                 ${S2OPC_PRIVATE_INCLUDES}
                                                 ${CHECK_INCLUDE_DIRS})
target_link_libraries(check_sockets PRIVATE ${CHECK_LIBRARIES} s2opc)
s2opc_unit_test(check_sockets)

# Secure channels (stubbed on services side) tests
file(GLOB_RECURSE SECURE_CHANNELS_SRC
    "csrc/configuration/sopc_ieee_check.c"
    "csrc/secure_channels/*.c"
    "csrc/opcua_types/*.c"
    "csrc/crypto/*.c"
    "csrc/helpers/*.c"
    ${PLATFORM_SRCS}
)

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

# Secure channels tests: CLIENT executable
add_executable(test_secure_channels_client ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_CLIENT_TEST})

target_include_directories(test_secure_channels_client PRIVATE "tests/secure_channels/stubs"
                                                               ${S2OPC_PUBLIC_INCLUDES}
                                                               ${S2OPC_PRIVATE_INCLUDES}
                                                               ${MBEDTLS_INCLUDE_DIRS})
target_link_libraries(test_secure_channels_client PRIVATE ${S2OPC_STATIC_DEP_LIBS})

# Secure channels tests: SERVER executable
add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_SERVER_TEST})
target_link_libraries(test_secure_channels_server PRIVATE ${S2OPC_STATIC_DEP_LIBS})

target_include_directories(test_secure_channels_server PRIVATE "tests/secure_channels/stubs"
                                                               ${S2OPC_PUBLIC_INCLUDES}
                                                               ${S2OPC_PRIVATE_INCLUDES}
                                                               ${MBEDTLS_INCLUDE_DIRS})

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
target_link_libraries(check_sc_rcv_buffer PRIVATE ${CHECK_LIBRARIES} ${S2OPC_STATIC_DEP_LIBS})

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
target_link_libraries(check_sc_rcv_encrypted_buffer PRIVATE ${CHECK_LIBRARIES} ${S2OPC_STATIC_DEP_LIBS})

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

# Validation tests

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

# Benchmarks
add_executable(bench_tool "tests/benchmarks/bench_tool.c")
target_link_libraries(bench_tool PRIVATE s2opc)
target_include_directories(bench_tool PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES})

if (EXPAT_FOUND)
	add_executable(s2opc_parse_uanodeset "tests/demo/s2opc_parse_uanodeset.c")
	target_link_libraries(s2opc_parse_uanodeset PRIVATE s2opc "s2opc-xml-loaders-expat")
	target_include_directories(s2opc_parse_uanodeset PRIVATE ${S2OPC_PUBLIC_INCLUDES})
endif()

# Fuzzing
set(FUZZING_PATH "tests/fuzzing")
set(FUZZING_RECEIVE_MSG_BUFFER "${FUZZING_PATH}/fuzz_mgr__receive_msg_buffer")

# Local Fuzzing
function(s2opc_fuzzer name src_file)
	set(s2opc_fuzzer_loop_counter "0")	#For each counter
	set(standalone_target_name "${name}.standalone")
	set(fuzz_target_name "${name}.libfuzzer")

	add_executable(${standalone_target_name} "${src_file}" "${FUZZING_PATH}/standalone_fuzzer.c")
	target_include_directories(${standalone_target_name} PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES} "tests/services")
	target_link_libraries(${standalone_target_name} s2opc s2opc-loader-embedded)
	add_dependencies(fuzzers ${standalone_target_name})

	add_executable(${fuzz_target_name} "${src_file}" ${TEST_DEMO_SERVER_ADDRESS_SPACE_C})
	set_target_properties(${fuzz_target_name} PROPERTIES COMPILE_FLAGS "-fsanitize=fuzzer")
	set_target_properties(${fuzz_target_name} PROPERTIES LINK_FLAGS "-fsanitize=fuzzer")
	target_include_directories(${fuzz_target_name} PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES})
	target_link_libraries(${fuzz_target_name} s2opc s2opc-loader-embedded)
	add_dependencies(fuzzers ${fuzz_target_name})

	#link dynamically extra parameters
	foreach(param ${ARGV})
		if (NOT (${s2opc_fuzzer_loop_counter} MATCHES "0" OR ${s2opc_fuzzer_loop_counter} MATCHES "1"))
			target_sources(${standalone_target_name} PUBLIC "${param}")
			target_sources(${fuzz_target_name} PUBLIC "${param}")
		endif()
		MATH(EXPR s2opc_fuzzer_loop_counter "${s2opc_fuzzer_loop_counter}+1") #Counter incrementation
	endforeach()
endfunction()

if (NOT WITH_OSS_FUZZ AND "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS "6.0.0")
		# We need at least CLang 6 to get LibFuzzer
		message(STATUS "Your version of Clang (${CMAKE_C_COMPILER_ID}) is not recent enough to have LibFuzzer")
	else()
		add_custom_target(fuzzers)
		s2opc_fuzzer(receive_msg_buffer_fuzzer  "${FUZZING_RECEIVE_MSG_BUFFER}/fuzz_main.c"
												"${FUZZING_RECEIVE_MSG_BUFFER}/fuzz_server.c"
												"${FUZZING_RECEIVE_MSG_BUFFER}/fuzz_client.c"
												"${TEST_DEMO_SERVER_ADDRESS_SPACE_C}"
                                                "tests/services/test_results.c"
                                                )
		s2opc_fuzzer(server_request_fuzzer "${FUZZING_PATH}/fuzz_server_request.c")
		s2opc_fuzzer(parse_tcp_uri_fuzzer "${FUZZING_PATH}/fuzz_parse_tcp_uri.c")
	endif()
endif()

endif(ENABLE_TESTING)

# OSS Fuzzing
function(s2opc_oss_fuzzer target_name src_file)
  set(s2opc_fuzzer_loop_counter "0")	#For each counter
  add_executable(${target_name} "${src_file}")

  add_executable(${target_name} "${src_file}" ${TEST_DEMO_SERVER_ADDRESS_SPACE_C})
  target_include_directories(${target_name} PRIVATE ${S2OPC_PUBLIC_INCLUDES} ${S2OPC_PRIVATE_INCLUDES} "tests/services")  
  target_link_libraries(${target_name} s2opc s2opc-loader-embedded)
  target_link_libraries(${target_name} s2opc $ENV{LIB_FUZZING_ENGINE})
  add_dependencies(fuzzers ${target_name})

  foreach(param ${ARGV})
		if (NOT (${s2opc_fuzzer_loop_counter} MATCHES "0" OR ${s2opc_fuzzer_loop_counter} MATCHES "1"))
			target_sources(${target_name} PRIVATE "${param}")
		endif()
		MATH(EXPR s2opc_fuzzer_loop_counter "${s2opc_fuzzer_loop_counter}+1") #Counter incrementation
	endforeach()
endfunction()

if(WITH_OSS_FUZZ)
  add_custom_target(fuzzers)
  s2opc_oss_fuzzer(receive_msg_buffer_fuzzer "${FUZZING_RECEIVE_MSG_BUFFER}/fuzz_main.c"
  											 "${FUZZING_RECEIVE_MSG_BUFFER}/fuzz_server.c"
                                             "${FUZZING_RECEIVE_MSG_BUFFER}/fuzz_client.c" )
  s2opc_oss_fuzzer(server_request_fuzzer "${FUZZING_PATH}/fuzz_server_request.c")
  s2opc_oss_fuzzer(parse_tcp_uri_fuzzer "${FUZZING_PATH}/fuzz_parse_tcp_uri.c")
endif()

###### Subscribing client library and binary ######

set(LIBSUBSCRIBE_PATH "tests/client_subscription")
add_library(client_subscription "${LIBSUBSCRIBE_PATH}/libs2opc_client.c"
                                "${LIBSUBSCRIBE_PATH}/libs2opc_client_cmds.c"
                                "${LIBSUBSCRIBE_PATH}/libs2opc_client_common.c"
                                "${LIBSUBSCRIBE_PATH}/toolkit_helpers.c"
                                "${LIBSUBSCRIBE_PATH}/state_machine.c"
                                "${LIBSUBSCRIBE_PATH}/pki_permissive.c"
                                )
target_link_libraries(client_subscription PRIVATE s2opc)
target_include_directories(client_subscription PRIVATE ${S2OPC_PUBLIC_INCLUDES}
                                                       ${S2OPC_PRIVATE_INCLUDES}
                                                       ${LIBSUBSCRIBE_PATH})
set_target_properties(client_subscription PROPERTIES PUBLIC_HEADER "${LIBSUBSCRIBE_PATH}/libs2opc_client.h")
install(TARGETS client_subscription
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

if(ENABLE_TESTING)

# s2opc_subscription_client is not built on Windows
if(NOT WIN32)
# Demo client
add_executable(s2opc_subscription_client "${LIBSUBSCRIBE_PATH}/client.c")

target_link_libraries(s2opc_subscription_client PRIVATE client_subscription s2opc)

add_executable(example_wrapper_write "${LIBSUBSCRIBE_PATH}/examples/write.c")
target_include_directories(example_wrapper_write PRIVATE ${LIBSUBSCRIBE_PATH})
target_link_libraries(example_wrapper_write PRIVATE client_subscription s2opc)

add_executable(example_wrapper_read "${LIBSUBSCRIBE_PATH}/examples/read.c")
target_include_directories(example_wrapper_read PRIVATE ${LIBSUBSCRIBE_PATH})
target_link_libraries(example_wrapper_read PRIVATE client_subscription s2opc)

add_executable(example_wrapper_browse "${LIBSUBSCRIBE_PATH}/examples/browse.c")
target_include_directories(example_wrapper_browse PRIVATE ${LIBSUBSCRIBE_PATH})
target_link_libraries(example_wrapper_browse PRIVATE client_subscription s2opc)

add_executable(example_wrapper_get_endpoints "${LIBSUBSCRIBE_PATH}/examples/get_endpoints.c")
target_include_directories(example_wrapper_get_endpoints PRIVATE ${LIBSUBSCRIBE_PATH})
target_link_libraries(example_wrapper_get_endpoints PRIVATE client_subscription s2opc)

add_executable(example_wrapper_subscribe "${LIBSUBSCRIBE_PATH}/examples/subscribe.c")
target_include_directories(example_wrapper_subscribe PRIVATE ${LIBSUBSCRIBE_PATH})
target_link_libraries(example_wrapper_subscribe PRIVATE client_subscription s2opc)

endif(NOT WIN32)

# Helpers unit tests
add_executable(check_libsub "${LIBSUBSCRIBE_PATH}/check_libsub.c")
target_include_directories(check_libsub PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
target_link_libraries(check_libsub PRIVATE client_subscription s2opc ${CHECK_LIBRARIES})
s2opc_client_validation_test(check_libsub)

add_executable(check_wrapper "${LIBSUBSCRIBE_PATH}/check_wrapper.c")
target_include_directories(check_wrapper PRIVATE ${S2OPC_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS} ${CHECK_INCLUDE_DIRS})
target_link_libraries(check_wrapper PRIVATE client_subscription s2opc ${CHECK_LIBRARIES})
s2opc_client_validation_test(check_wrapper)

endif(ENABLE_TESTING)

###### PyS2OPC ######

# The library is built by python. It may use the shared library but it must not be built with ASan.
# The problem with ASan is to embed a dynamic library compiled with ASan into Python, which is not compiled with ASan.
# The compilation is not done with clang-analyzer either, as this would analyze CFFI-generated code.
if(WITH_PYS2OPC)
  set(PYS2OPC_PATH "${CMAKE_SOURCE_DIR}/tests/pys2opc")
  set(PYS2OPC_FILES "${PYS2OPC_PATH}/pys2opc/__init__.py"
                    "${PYS2OPC_PATH}/pys2opc/connection.py"
                    "${PYS2OPC_PATH}/pys2opc/pys2opc_build.py"
                    "${PYS2OPC_PATH}/pys2opc/responses.py"
                    "${PYS2OPC_PATH}/pys2opc/s2opc.py"
                    "${PYS2OPC_PATH}/pys2opc/types.py"
                    )
  set(PYS2OPC_PLACEHOLDER ${CMAKE_BINARY_DIR}/pys2opc_touch)
  add_custom_command(OUTPUT ${PYS2OPC_PLACEHOLDER}
                     COMMAND ${PYTHON_EXECUTABLE} setup.py build_ext -b ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -t ${CMAKE_BINARY_DIR}
                     COMMAND ${CMAKE_COMMAND} -E touch ${PYS2OPC_PLACEHOLDER}
                     WORKING_DIRECTORY ${PYS2OPC_PATH})
  add_custom_target(pys2opc ALL
                    DEPENDS ${PYS2OPC_PLACEHOLDER}
                    DEPENDS ${PYS2OPC_FILES})
  add_dependencies(pys2opc s2opc_subscription_client)

  # Validation test
  if(ENABLE_TESTING)

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
  endif(ENABLE_TESTING)
endif()

###### Doc ######

if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E env DOXYGEN_SOURCE_PREFIX=${CMAKE_SOURCE_DIR}/ ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxygen/s2opc-toolkit.doxyfile
        COMMENT "Generating API documentation"
        VERBATIM
    )
endif()
