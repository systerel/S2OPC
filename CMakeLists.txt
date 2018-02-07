cmake_minimum_required (VERSION 2.8)
project (INGOPCS)

# Define runtime output directory
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

# Compiler options (C99 compliant)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit -Wreturn-type -Wsequence-point -Wcast-qual -Wuninitialized -Wcast-align -Wstrict-prototypes -Wchar-subscripts -Wformat-security -Wformat")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  # Specific flags for CERT rules
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wunicode -Wimplicit-int -Wreserved-id-macro -Wsometimes-uninitialized -Wunsequenced -Wincompatible-pointer-types-discards-qualifiers -Wunevaluated-expression -Wparentheses -Wint-conversion -Wint-to-pointer-cast -Wincompatible-pointer-types")
else()
  message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -O0")

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

# Define INGOPCS library (static or shared based on BUILD_SHARED_LIBS)
add_library(ingopcs ${PLATFORM_SRCS} ${INGOPCS_SRCS})

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
target_include_directories(ingopcs BEFORE
    PUBLIC  ${INGOPCS_PUBLIC_INCLUDES}
    PRIVATE ${INGOPCS_PRIVATE_INCLUDES}
)

# Define INGOPCS library dependencies
set(INGOPCS_DEP_LIBS mbedtls mbedcrypto mbedx509 ${MATH_LIB} ${THREAD_LIB} ${SOCK_LIB})
target_link_libraries(ingopcs PUBLIC ${INGOPCS_DEP_LIBS})
if(WIN32)
  # always link libc statically (even in case of shared library)
  target_link_libraries(ingopcs PRIVATE -static-libgcc)
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

# INGOPCS tests ################################################################################

# lib Check dependency: set specialized linking configuration dependent on platform
if(UNIX)
  # force static linking
  set(LIBCHECK_DEP_LIB libcheck.a)
elseif(WIN32)
  # force static linking + libcheck dependency to provide missing functions on windows
  set(LIBCHECK_DEP_LIB libcheck.a compat)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Check generated address space for tests
set(ADDSPACEGEN_DIR "address_space_generation")

add_executable(check_ingopcs_addspace "${ADDSPACEGEN_DIR}/genc/check_ingopcs_addspace.c"
                                      "${ADDSPACEGEN_DIR}/genc/ingopcs_addspace.c")
target_link_libraries(check_ingopcs_addspace PRIVATE)
target_include_directories(check_ingopcs_addspace PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                          "${ADDSPACEGEN_DIR}/include")

# Allow to test the generated addspace with "make test" since some tests are built using it
enable_testing()
add_test(check_addspace ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/check_ingopcs_addspace)
set_tests_properties(check_addspace PROPERTIES DEPENDS check_ingopcs_addspace)

# Helpers unit tests
file(GLOB_RECURSE INTERNAL_TESTS_SRCS "tests/helpers/*.c")
add_executable(check_helpers ${INTERNAL_TESTS_SRCS})
target_include_directories(check_helpers PRIVATE ${INGOPCS_PRIVATE_INCLUDES})
target_link_libraries(check_helpers PRIVATE ingopcs ${LIBCHECK_DEP_LIB})

# Client test
add_executable(toolkit_test_client "tests/services/toolkit_test_client.c"
                                   "tests/services/wrap_read.c"
                                   "tests/services/testlib_write.c"
                                   "tests/services/test_results.c"
                                   "tests/services/testlib_read_response.c"
                                   "address_space_generation/genc/ingopcs_addspace.c")
target_link_libraries(toolkit_test_client PRIVATE ingopcs)
target_include_directories(toolkit_test_client PRIVATE "${ADDSPACEGEN_DIR}/include"
                                                       ${INGOPCS_PUBLIC_INCLUDES} 
                                                       ${INGOPCS_PRIVATE_INCLUDES})

# Server test
add_executable(toolkit_test_server "tests/services/toolkit_test_server.c"
                                   "address_space_generation/genc/ingopcs_addspace.c")
target_link_libraries(toolkit_test_server PRIVATE ingopcs)
target_include_directories(toolkit_test_server PRIVATE "${ADDSPACEGEN_DIR}/include"
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
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/cacert.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/trusted/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/client_2k.pem
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client_2k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/server_2k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/server_2k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/
    # Add 4096 client / server keys
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client_4k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client_4k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/server_4k.key ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/server_4k.der ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/server_public/)

# Read service test
add_executable(toolkit_test_read "tests/services/toolkit_test_read.c"
                                 "tests/services/wrap_read.c"
                                 "tests/services/testlib_read_response.c"
                                 "address_space_generation/genc/ingopcs_addspace.c")
target_link_libraries(toolkit_test_read PRIVATE ingopcs)
target_include_directories(toolkit_test_read PRIVATE "${ADDSPACEGEN_DIR}/include"
                                                     ${INGOPCS_PUBLIC_INCLUDES} 
                                                     ${INGOPCS_PRIVATE_INCLUDES})

# Write service test
add_executable(toolkit_test_write "tests/services/toolkit_test_write.c"
                                  "tests/services/testlib_write.c"
                                  "address_space_generation/genc/ingopcs_addspace.c")
target_link_libraries(toolkit_test_write PRIVATE ingopcs)
target_include_directories(toolkit_test_write PRIVATE "${ADDSPACEGEN_DIR}/include"
                                                      ${INGOPCS_PUBLIC_INCLUDES} 
                                                      ${INGOPCS_PRIVATE_INCLUDES})

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
                                                 ${INGOPCS_PRIVATE_INCLUDES})
target_link_libraries(check_sockets PRIVATE ${LIBCHECK_DEP_LIB} ${INGOPCS_DEP_LIBS})

# Secure channels (stubbed on services side) tests
file(GLOB_RECURSE SECURE_CHANNELS_SRC
    "csrc/secure_channels/*.c"
    "csrc/opcua_types/*.c"
    "csrc/crypto/*.c"
)

file(GLOB_RECURSE SECURE_CHANNELS_CLIENT_TEST
     "tests/secure_channels/test_secure_channels_client.c"
     "tests/secure_channels/stubs/*.c")
     
file(GLOB_RECURSE SECURE_CHANNELS_SERVER_TEST
     "tests/secure_channels/test_secure_channels_server.c"
     "tests/secure_channels/stubs/*.c")

# Secure channels tests: CLIENT executable
add_executable(test_secure_channels_client ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_CLIENT_TEST})

target_include_directories(test_secure_channels_client PRIVATE "tests/secure_channels/stubs" 
                                                               ${INGOPCS_PUBLIC_INCLUDES} 
                                                               ${INGOPCS_PRIVATE_INCLUDES})
target_link_libraries(test_secure_channels_client PRIVATE ${INGOPCS_DEP_LIBS})

# Secure channels tests: SERVER executable
add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_SERVER_TEST})
target_link_libraries(test_secure_channels_server PRIVATE ${INGOPCS_DEP_LIBS})

target_include_directories(test_secure_channels_server PRIVATE "tests/secure_channels/stubs" 
                                                               ${INGOPCS_PUBLIC_INCLUDES} 
                                                               ${INGOPCS_PRIVATE_INCLUDES})
