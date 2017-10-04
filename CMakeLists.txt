cmake_minimum_required (VERSION 2.8)
project (INGOPCS)

set(stack_version STACK_1_01 CACHE STRING "Version of the string")
set_property(CACHE stack_version PROPERTY STRINGS STACK_1_01 STACK_1_02 STACK_1_03)

option(wrapper_reception_thread "Activate a global reception thread" OFF)

# Extern target: mbedtls
add_subdirectory(libraries/mbedtls-2.3.0)

# Compiler options
# C99 compliant
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -O0")

# Core library
if(UNIX)
  set(PLATFORM_DIR linux)
elseif(WIN32)
  set(PLATFORM_DIR windows)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

file(GLOB PLATFORM_SRCS "src/helpers_platform_dep/${PLATFORM_DIR}/*.c")

if(UNIX)
  # add POSIX support
  set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_XOPEN_SOURCE=600")
elseif(WIN32)
  # minimum Vista for IPV6 support
  set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_WIN32_WINNT=0x0600")
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

file(GLOB_RECURSE UASTACK_SRCS
    "stack/src/API/*.c"
    "stack/src/APIwrappers/*.c"
    "stack/src/core_tools/*.c"
    "stack/src/core_types/*.c"
    "stack/src/stack_core/*.c"
    "stack/src/tools/*.c"
    "toolkit/src/genc/*.c"
    "toolkit/src/impl/*.c"
)

file(GLOB_RECURSE INGOPCS_SRCS
    "src/configuration/*.c"
    "src/helpers/*.c"
    "src/helpers_crypto/*.c"
    "src/helpers_opcua_types/*.c"
    "src/sockets/*.c"
    "src/secure_channels/*.c"
)

add_library(ingopcs STATIC ${PLATFORM_SRCS} ${INGOPCS_SRCS} ${UASTACK_SRCS})
set(INGOPCS_PUBLIC_INCLUDES
    "stack/src/API"
    "stack/src/APIwrappers"
    "stack/src/core_types"
    "stack/src/core_tools"
    "stack/src/tools"
    "toolkit/src/genc"
    "toolkit/src/impl"
    "src/configuration"
    "src/helpers"
    "src/helpers_opcua_types"
    "src/helpers_crypto"
    "src/helpers_platform_dep"
    "src/sockets"
    "src/secure_channels"
)
set(INGOPCS_PRIVATE_INCLUDES
    "src/helpers_crypto/mbedtls"
    "src/helpers_platform_dep/${PLATFORM_DIR}"
    "stack/src/stack_core"
    "libraries/mbedtls-2.3.0/include" # For some reason, the CMakeLists from mbedtls does not export that
)
target_include_directories(ingopcs BEFORE
    PUBLIC  ${INGOPCS_PUBLIC_INCLUDES}
    PRIVATE ${INGOPCS_PRIVATE_INCLUDES}
)
target_link_libraries(ingopcs PUBLIC mbedtls)
target_link_libraries(ingopcs PUBLIC pthread)
target_link_libraries(ingopcs PUBLIC m)
set_property(TARGET ingopcs APPEND PROPERTY COMPILE_DEFINITIONS ${stack_version})

if (wrapper_reception_thread)
    set_property(TARGET ingopcs APPEND PROPERTY COMPILE_DEFINITIONS "WRAPPER_RECEPTION_THREAD")
endif()

# Stack

# Stub client
#add_executable(stub_client_ingopcs "stack/stub_client/stub_client_ingopcs.c")
#target_link_libraries(stub_client_ingopcs PRIVATE ingopcs)
#target_include_directories(stub_client_ingopcs BEFORE
#    PRIVATE "stack/stub_client"
#)

# Stub server
#add_executable(stub_server_ingopcs "stack/stub_server/stub_server_ingopcs.c")
#target_link_libraries(stub_server_ingopcs PRIVATE ingopcs)
#target_include_directories(stub_server_ingopcs BEFORE
#    PRIVATE "stack/stub_server"
#)

# Client/Server tests
#add_custom_target(client_server_test
#    "./stack/run_client_server_test.sh"
#    DEPENDS stub_client_ingopcs stub_server_ingopcs
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    COMMENT "Runs the client/server tests")

# Unit tests
if (NOT CMAKE_CROSSCOMPILING)
    file(GLOB_RECURSE INTERNAL_TESTS_SRCS "stack/test/ingopcs/*.c")
    add_executable(check_stack ${INTERNAL_TESTS_SRCS})
    target_include_directories(check_stack PRIVATE ${INGOPCS_PRIVATE_INCLUDES})
    target_link_libraries(check_stack PRIVATE ingopcs check)
endif()

# Toolkit tests

# Toolkit Client
add_executable(toolkit_test_client "toolkit/src/test/toolkit_test_client.c"
                                   "toolkit/src/test/wrap_read.c"
                                   "toolkit/src/test/testlib_write.c"
                                   "toolkit/src/test/test_results.c"
                                   "toolkit/src/test/testlib_read_response.c"
                                   "toolkit/address_space_generation/genc/test_read_write_addspace.c")
target_link_libraries(toolkit_test_client PRIVATE ingopcs)
target_include_directories(toolkit_test_client PRIVATE "toolkit/address_space_generation/"
                                                       ${INGOPCS_PUBLIC_INCLUDES} 
                                                       ${INGOPCS_PRIVATE_INCLUDES})

# Toolkit Server
add_executable(toolkit_test_server "toolkit/src/test/toolkit_test_server.c"
                                   "toolkit/address_space_generation/genc/add.c")
target_link_libraries(toolkit_test_server PRIVATE ingopcs)
target_include_directories(toolkit_test_server PRIVATE "toolkit/address_space_generation/"
                                                       ${INGOPCS_PUBLIC_INCLUDES} 
                                                       ${INGOPCS_PRIVATE_INCLUDES})

# Tookit read service
add_executable(toolkit_test_read "toolkit/src/test/toolkit_test_read.c"
                                 "toolkit/src/test/wrap_read.c"
                                 "toolkit/src/test/testlib_read_response.c"
                                 "toolkit/address_space_generation/genc/test_read_write_addspace.c")
target_link_libraries(toolkit_test_read PRIVATE ingopcs)
target_include_directories(toolkit_test_read PRIVATE "toolkit/address_space_generation/"
                                                     ${INGOPCS_PUBLIC_INCLUDES} 
                                                     ${INGOPCS_PRIVATE_INCLUDES})

# Toolkit write service
add_executable(toolkit_test_write "toolkit/src/test/toolkit_test_write.c"
                                  "toolkit/src/test/testlib_write.c"
                                  "toolkit/address_space_generation/genc/test_read_write_addspace.c")
target_link_libraries(toolkit_test_write PRIVATE ingopcs)
target_include_directories(toolkit_test_write PRIVATE "toolkit/address_space_generation/"
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
    target_link_libraries(check_sockets PRIVATE pthread check)
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
target_link_libraries(test_secure_channels_client PRIVATE mbedtls pthread)

# Secure channels tests: SERVER executable
add_executable(test_secure_channels_server ${SOCKETS_SRC}
                                           ${SECURE_CHANNELS_SRC}
                                           ${SECURE_CHANNELS_SERVER_TEST})

target_include_directories(test_secure_channels_server PRIVATE "tests/secure_channels/stubs" 
                                                               ${INGOPCS_PUBLIC_INCLUDES} 
                                                               ${INGOPCS_PRIVATE_INCLUDES})
target_link_libraries(test_secure_channels_server PRIVATE mbedtls pthread)
