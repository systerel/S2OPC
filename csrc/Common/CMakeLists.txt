cmake_minimum_required(VERSION 3.5)
project(S2OPC_Common LANGUAGES C)

##                                                ##
#    S2OPC common library contains shared code     #
# between OPC UA client/server and pub/sub modules #
##                                                ##
if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../CommonDefs.cmake)
endif()

### Dependencies ###
include (CheckTypeSize)

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

### Compilation of S2OPC common library source files ###

## Build info source file generation (version, commit checksum, docker id, date) ##

if(CMAKE_HOST_UNIX)
  add_custom_command(OUTPUT configuration/sopc_toolkit_build_info.h
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/configuration/sopc_toolkit_build_info.h_ ${CMAKE_CURRENT_SOURCE_DIR}/configuration/sopc_toolkit_build_info.h
                     COMMAND ${S2OPC_ROOT_PATH}/scripts/gen_build_info_file.sh ${CMAKE_CURRENT_SOURCE_DIR}/configuration
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  add_custom_command(OUTPUT configuration/sopc_toolkit_build_info.h
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/configuration/sopc_toolkit_build_info.h_ ${CMAKE_CURRENT_SOURCE_DIR}/configuration/sopc_toolkit_build_info.h
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

add_custom_target(build_info ALL
                   DEPENDS configuration/sopc_toolkit_build_info.h
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

## Platform dependent source files and library dependencies ##

# Define platform dependent source files and libraries/definitions associated
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

# Update variable containing S2OPC library dependencies
list(APPEND S2OPC_COMMON_LINK_LIBRARIES ${MBEDTLS_LIBRARIES} ${MATH_LIB} ${THREAD_LIB} ${SOCK_LIB})

file(GLOB PLATFORM_SRCS "helpers_platform_dep/*.c" "helpers_platform_dep/${PLATFORM_DIR}/*.c")

# Set specialized compilation flags for platform dependent source files
if(UNIX)
  # add POSIX support
  set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_GNU_SOURCE")
elseif(WIN32)
  # minimum Vista for IPV6 support
  if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    # TODO: use COMPILE_FLAGS or COMPILE_DEFINITIONS with set_source_files_properties
    add_definitions(/DWIN32_LEAN_AND_MEAN)
    add_definitions(/D_WIN32_WINNT=0x0600)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
  elseif(MINGW)
    set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_FLAGS "-D_WIN32_WINNT=0x0600")
  endif()
endif()

# Add definitions for platform dependant code
check_type_size("time_t" CMAKE_TIME_T_SIZE) # retrieve time_t size
set_source_files_properties(${PLATFORM_SRCS} PROPERTIES COMPILE_DEFINITIONS "SOPC_PTR_SIZE=${CMAKE_SIZEOF_VOID_P};SOPC_TIME_T_SIZE=${CMAKE_TIME_T_SIZE}")
                 
## C99 compliant source code ##

# Define non-platform dependent source files
file(GLOB_RECURSE S2OPC_COMMON_SRCS
    "configuration/*.c"
    "helpers/*.c"
    "crypto/*.c"
    "opcua_types/*.c"
)

set(S2OPC_COMMON_PUBLIC_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/configuration"
    "${CMAKE_CURRENT_SOURCE_DIR}/helpers"
    "${CMAKE_CURRENT_SOURCE_DIR}/opcua_types"
    "${CMAKE_CURRENT_SOURCE_DIR}/crypto"
    "${CMAKE_CURRENT_SOURCE_DIR}/helpers_platform_dep"
    "${CMAKE_CURRENT_SOURCE_DIR}/helpers_platform_dep/${PLATFORM_DIR}"
)

set(S2OPC_COMMON_PRIVATE_INCLUDES
    "crypto/mbedtls"
)

### Configure S2OPC library and its properties ###

add_library(s2opc_common ${PLATFORM_SRCS} ${S2OPC_COMMON_SRCS})
target_compile_options(s2opc_common PRIVATE ${S2OPC_COMPILER_FLAGS})
target_compile_definitions(s2opc_common PRIVATE ${S2OPC_DEFINITIONS})
target_link_libraries(s2opc_common PUBLIC ${S2OPC_LINKER_FLAGS} ${S2OPC_LINK_LIBRARIES}
                                          ${S2OPC_COMMON_LINK_LIBRARIES})

target_include_directories(s2opc_common
  PUBLIC
  "$<BUILD_INTERFACE:${S2OPC_COMMON_PUBLIC_INCLUDES}>" # when building (quotes needed to interpret list correclty)
  $<INSTALL_INTERFACE:include> # when installed
  PRIVATE ${S2OPC_COMMON_PRIVATE_INCLUDES} ${MBEDTLS_INCLUDE_DIRS})

# Needed if we want to use the objects in a shared library
set_target_properties(s2opc_common PROPERTIES POSITION_INDEPENDENT_CODE ON)
# TODO: Should we add POSITION_INDEPENDENT to all internal libraries onto S2OPC instead ?
set_target_properties(s2opc_common PROPERTIES INTERFACE_POSITION_INDEPENDENT_CODE ON)

add_dependencies(s2opc_common build_info)

### Installation and export of S2OPC common library ###
foreach(dir ${S2OPC_COMMON_PUBLIC_INCLUDES})
    file(GLOB temp_files "${dir}/*.h")
    list(APPEND S2OPC_COMMON_INCLUDE_FILES ${temp_files})
endforeach()
# set public header property
set_target_properties(s2opc_common PROPERTIES PUBLIC_HEADER "${S2OPC_COMMON_INCLUDE_FILES}")

install(TARGETS s2opc_common
  EXPORT s2opc_common-export # export our project to be importable after install
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include)

# install the export as a cmake file
install(EXPORT s2opc_common-export
  FILE s2opc_common-export.cmake
  DESTINATION cmake)

# export cmake file for use in build tree without install
export(EXPORT s2opc_common-export
  FILE "${CMAKE_BINARY_DIR}/s2opc_common-export.cmake"
  )
