cmake_minimum_required (VERSION 2.8)
project (INGOPCS)

# Define runtime output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Compiler options (C99 compliant)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -O0")

# Source files of INGOPCS library

# Define platform dependent source files
if(UNIX)
  set(PLATFORM_DIR linux)
  set(STATIC_LIB_SUFFIX ".a")
elseif(WIN32)
  set(PLATFORM_DIR windows)
  set(STATIC_LIB_SUFFIX ".lib")
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

file(GLOB PLATFORM_SRCS "src/helpers_platform_dep/${PLATFORM_DIR}/*.c")

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
    "src/configuration/*.c"
    "src/helpers/*.c"
    "src/helpers_crypto/*.c"
    "src/helpers_opcua_types/*.c"
    "src/sockets/*.c"
    "src/secure_channels/*.c"
    "src/services/*.c"
)

# Define INGOPCS static library
add_library(ingopcs STATIC ${PLATFORM_SRCS} ${INGOPCS_SRCS})

# Define INGOPCS library includes
set(INGOPCS_PUBLIC_INCLUDES
    "src/configuration"
    "src/helpers"
    "src/helpers_opcua_types"
    "src/helpers_crypto"
    "src/helpers_platform_dep"
    "src/sockets"
    "src/secure_channels"
    "src/services"
)
set(INGOPCS_PRIVATE_INCLUDES
    "src/helpers_crypto/mbedtls"
    "src/helpers_platform_dep/${PLATFORM_DIR}"
    "src/services/BtoC_itf"
    "src/services/B_genC"
) 
target_include_directories(ingopcs BEFORE
    PUBLIC  ${INGOPCS_PUBLIC_INCLUDES}
    PRIVATE ${INGOPCS_PRIVATE_INCLUDES}
)

# Define INGOPCS library dependencies
target_link_libraries(ingopcs PUBLIC mbedtls mbedcrypto mbedx509)
target_link_libraries(ingopcs PUBLIC pthread)
target_link_libraries(ingopcs PUBLIC rt)
target_link_libraries(ingopcs PUBLIC m)


# INGOPCS tests ################################################################################

# Check generated address space for tests
set(ADDSPACEGEN_DIR "address_space_generation")

add_executable(check_ingopcs_addspace "${ADDSPACEGEN_DIR}/genc/check_ingopcs_addspace.c"
                                      "${ADDSPACEGEN_DIR}/genc/ingopcs_addspace.c")
target_link_libraries(check_ingopcs_addspace PRIVATE)
target_include_directories(check_ingopcs_addspace PRIVATE ${INGOPCS_PUBLIC_INCLUDES}
                                                          "${ADDSPACEGEN_DIR}/include")

# Allow to test the generated addspace with "make test" since some tests are built using it
enable_testing()
add_test(check_addspace ${CMAKE_CURRENT_SOURCE_DIR}/bin/check_ingopcs_addspace)
set_tests_properties(check_addspace PROPERTIES DEPENDS check_ingopcs_addspace)

# Helpers unit tests
if (NOT CMAKE_CROSSCOMPILING)
    file(GLOB_RECURSE INTERNAL_TESTS_SRCS "tests/helpers/*.c")
    add_executable(check_helpers ${INTERNAL_TESTS_SRCS})
    target_include_directories(check_helpers PRIVATE ${INGOPCS_PRIVATE_INCLUDES})
    target_link_libraries(check_helpers PRIVATE ingopcs libcheck${STATIC_LIB_SUFFIX})
endif()

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
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/revoked
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/untrusted
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/trusted
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/client_private
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/server_private
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/client_public
    COMMAND ${CMAKE_COMMAND} -E make_directory $(CMAKE_SOURCE_DIR)/bin/server_public
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/cacert.der ${CMAKE_SOURCE_DIR}/bin/trusted/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client.key ${CMAKE_SOURCE_DIR}/bin/client_private/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client.key ${CMAKE_SOURCE_DIR}/bin/client_private/client.pem
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/client.der ${CMAKE_SOURCE_DIR}/bin/client_public/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/server.key ${CMAKE_SOURCE_DIR}/bin/server_private/
    COMMAND ${CMAKE_COMMAND} -E copy $(CMAKE_SOURCE_DIR)/tests/data/cert/server.der ${CMAKE_SOURCE_DIR}/bin/server_public/)

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
    "src/helpers/*.c"
    "src/sockets/*.c"
)

file(GLOB_RECURSE SOCKETS_TEST
     "tests/sockets/*.c")

if (NOT CMAKE_CROSSCOMPILING)
    add_executable(check_sockets ${SOCKETS_SRC}
                                 ${SOCKETS_TEST})
    target_include_directories(check_sockets PRIVATE "tests/sockets/stubs" 
                                                     ${INGOPCS_PUBLIC_INCLUDES} 
                                                     ${INGOPCS_PRIVATE_INCLUDES})
    target_link_libraries(check_sockets PRIVATE ingopcs libcheck${STATIC_LIB_SUFFIX})
endif()

# Secure channels (stubbed on services side) tests
file(GLOB_RECURSE SECURE_CHANNELS_SRC
    "src/secure_channels/*.c"
    "src/helpers_opcua_types/*.c"
    "src/helpers_crypto/*.c"
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
target_link_libraries(test_secure_channels_client PRIVATE ingopcs)

# Secure channels tests: SERVER executable
add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_SERVER_TEST})

target_include_directories(test_secure_channels_server PRIVATE "tests/secure_channels/stubs" 
                                                               ${INGOPCS_PUBLIC_INCLUDES} 
                                                               ${INGOPCS_PRIVATE_INCLUDES})
target_link_libraries(test_secure_channels_server PRIVATE ingopcs)
