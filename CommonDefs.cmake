##                                   ##
# S2OPC libraries common definitions  # 
##                                   ##

set(S2OPC_COMMON_DEFS_SET TRUE)

### Define root path and CMake module path for external libraries ###

set(S2OPC_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_MODULE_PATH "${S2OPC_ROOT_PATH}/CMake;${CMAKE_MODULE_PATH}")

### Output directories ###

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


### Define default S2OPC compilation flags for several compilers ###

# Define variables to store S2OPC specific definitions, compiler and linker flags
set(S2OPC_DEFINITIONS)
set(S2OPC_COMPILER_FLAGS)
set(S2OPC_LINKER_FLAGS)
set(S2OPC_LINK_LIBRARIES)

# Identify compiler: variable set only on expected compiler
set(IS_GNU $<C_COMPILER_ID:GNU>)
set(IS_CLANG $<C_COMPILER_ID:Clang>)
set(IS_MSVC $<C_COMPILER_ID:MSVC>)
set(IS_MINGW $<BOOL:${MINGW}>) # MINGW set by cmake

# make the warnings as errors a default behavior
option(WARNINGS_AS_ERRORS "Treat warnings as errors when building" ON)
set(IS_WARNINGS_AS_ERRORS $<STREQUAL:${WARNINGS_AS_ERRORS},ON>)

# Set GNU compiler flags
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_GNU}:-std=c99 -pedantic -Wall -Wextra>)
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_GNU}:$<${IS_WARNINGS_AS_ERRORS}:-Werror>>)
# Specific flags for CERT rules
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_GNU}:-Wimplicit -Wreturn-type -Wsequence-point -Wcast-qual -Wuninitialized -Wcast-align -Wstrict-prototypes -Wchar-subscripts -Wformat=2 -Wconversion -Wshadow -Wmissing-prototypes>)

# Set Clang compiler flags
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_CLANG}:-std=c99 -pedantic -Wall -Wextra -Wunreachable-code>)
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_CLANG}:$<${IS_WARNINGS_AS_ERRORS}:-Werror>>)
# Specific flags for CERT rules
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_CLANG}:-Wunicode -Wimplicit-int -Wreserved-id-macro -Wsometimes-uninitialized -Wunsequenced -Wincompatible-pointer-types-discards-qualifiers -Wunevaluated-expression -Wparentheses -Wint-conversion -Wint-to-pointer-cast -Wincompatible-pointer-types -Wvla -Wconversion>)

# Set MSVC compiler flags
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_MSVC}:/W3 /Zi /sdl>)
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_MSVC}:$<${IS_WARNINGS_AS_ERRORS}:/WX>>)
# Set MSVC definitions (lean_and_mean avoid issue on order of import of Windows.h and Winsock2.h)
# TODO: nor COMPILE_FLAGS, COMPILE_DEFINITIONS or use of ${IS_MSCV} works, to be investigated
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/DWIN32_LEAN_AND_MEAN)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

# Add flags when MINGW compiler (IS_GNU is also valid for MINGW)
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_MINGW}:-Wno-pedantic-ms-format>)
# Always link MINGW libc statically (even in case of shared library): avoid dependency on external libgcc library
list(APPEND S2OPC_LINKER_FLAGS $<${IS_MINGW}:-static-libgcc>)

# Add -fno-omit-frame-pointer when build type is RelWithDebInfo or Debug
list(APPEND S2OPC_COMPILER_FLAGS $<$<STREQUAL:"${CMAKE_BUILD_TYPE}","RelWithDebInfo">:-fno-omit-frame-pointer>)
list(APPEND S2OPC_COMPILER_FLAGS $<$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">:-fno-omit-frame-pointer>)

# TODO: avoid modifying CMAKE_CFLAGS_* variables ? create new CMAKE_CONFIGURATION_TYPES equivalent to the 2 following but without DNDEBUG ?
# Re-enable asserts for Release and RelWithDebInfo builds
string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})


### Manage options for S2OPC compilation ###

# compilation options for S2OPC library code analysis purpose (mutually exclusive options)
option(WITH_ASAN "build with ASAN" OFF) # address sanitizer
option(WITH_TSAN "build with TSAN" OFF) # thread sanitizer
option(WITH_UBSAN "build with UBSAN" OFF) # undefined behavior sanitizer
option(WITH_COVERAGE "build with COVERAGE" OFF) # code coverage
option(WITH_COVERITY "set this flag when built with coverity" OFF) # indicates build for coverity: no incompatible with others WITH_* options
option(WITH_GPERF_PROFILER "link against the gperftool profiler") # activates link against gperf
option(WITH_CLANG_SOURCE_COVERAGE "build with Clang source coverage" OFF)
# S2OPC library extension option (also mutually exclusive with above options)
option(WITH_OSS_FUZZ "Add the fuzzers target when building for OSS-Fuzz" OFF)
option(WITH_PYS2OPC "Also builds PyS2OPC" OFF)
# S2OPC client/server library scope option
option(WITH_NANO_EXTENDED "Use Nano profile with additional services out of Nano scope" OFF)

# Check project and option(s) are compatible

# Function to check only one option (option name provided) is activated for all calls to this function
function(check_mutually_exclusive_options option_name)
  if(${${option_name}}) # Check variable of the option_name is defined
    if(NOT WITH_OPTION_MUTUALLY_EXCLUSIVE) # Check no other option was recorded before
      message("-- ${option_name} S2OPC option set")
      set(WITH_OPTION_MUTUALLY_EXCLUSIVE ${option_name} PARENT_SCOPE)
    else()
      message(FATAL_ERROR "${option_name} option set with mutually exclusive option ${WITH_OPTION_MUTUALLY_EXCLUSIVE}")
    endif()
  endif()
endfunction()

# Function to check no mutually exclusive option is set if given option is active
function(check_no_compilation_option option_name)
  if(${${option_name}}) # Check variable of the option_name is defined
    if(NOT WITH_OPTION_MUTUALLY_EXCLUSIVE) # Check no compilation option was recorded before
      message("-- ${option_name} S2OPC option set")
    else()
      message(FATAL_ERROR "${option_name} option set with incompatible compilation option ${WITH_OPTION_MUTUALLY_EXCLUSIVE}")
    endif()
  endif()
endfunction()

# Function to check given option is not active
function(check_not_activated_option option_name reason)
  if(${${option_name}}) # Check variable of the option_name is defined
    message(FATAL_ERROR "${option_name} incompatible option set: ${reason}")
  endif()
endfunction()

# Function to check build mode is debug
function(check_debug_build_type option_name reason)
  if(${${option_name}})
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
      message(FATAL_ERROR "${option_name} requires Debug or RelWithDebInfo build type (found: '${CMAKE_BUILD_TYPE}'): ${reason}")
    endif()
  endif()
endfunction()

function(print_if_activated option_name)
  if(${${option_name}})
    message("-- ${option_name} S2OPC option set")
  endif()
endfunction()

# Check for incompatible options activated
check_mutually_exclusive_options("WITH_ASAN")
check_mutually_exclusive_options("WITH_TSAN")
check_mutually_exclusive_options("WITH_UBSAN")
check_mutually_exclusive_options("WITH_COVERAGE")
check_mutually_exclusive_options("WITH_COVERITY")
check_mutually_exclusive_options("WITH_GPERF_PROFILER")
check_mutually_exclusive_options("WITH_CLANG_SOURCE_COVERAGE")
check_no_compilation_option("WITH_OSS_FUZZ")
check_no_compilation_option("WITH_PYS2OPC")
if(NOT UNIX)
  check_not_activated_option("WITH_ASAN" "not a unix system")
  check_not_activated_option("WITH_TSAN" "not a unix system")
  check_not_activated_option("WITH_UBSAN" "not a unix system")
  check_not_activated_option("WITH_COVERAGE" "not a unix system")
  check_not_activated_option("WITH_COVERITY" "not a unix system")
  check_not_activated_option("WITH_GPERF_PROFILER" "not a unix system")
  check_not_activated_option("WITH_CLANG_SOURCE_COVERAGE" "not a unix system") 
endif()
check_debug_build_type("WITH_ASAN" "to set compilation flag '-fno-omit-frame-pointer'")
check_debug_build_type("WITH_TSAN" "to set compilation flag '-fno-omit-frame-pointer'")
check_debug_build_type("WITH_UBSAN" "to set compilation flag '-fno-omit-frame-pointer'")
# print options with no incompatibility
print_if_activated("WITH_NANO_EXTENDED")
print_if_activated("WITH_CONST_ADDSPACE")
print_if_activated("WITH_STATIC_SECURITY_DATA")

# Check specific options constraints and set necessary compilation flags

# check if compiler support new sanitization options
include(CheckCCompilerFlag)
set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address")
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-compare" COMPILER_SUPPORTS_SAN_PC)
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-subtract" COMPILER_SUPPORTS_SAN_PS)
unset(CMAKE_REQUIRED_LIBRARIES)

if(WITH_ASAN)
  list(APPEND S2OPC_COMPILER_FLAGS -fsanitize=address)
  list(APPEND S2OPC_LINKER_FLAGS -fsanitize=address)
  if(COMPILER_SUPPORTS_SAN_PC)
    list(APPEND S2OPC_COMPILER_FLAGS -fsanitize=pointer-compare)
    list(APPEND S2OPC_LINKER_FLAGS -fsanitize=pointer-compare)
  endif()
  if(COMPILER_SUPPORTS_SAN_PS)
    list(APPEND S2OPC_COMPILER_FLAGS -fsanitize=pointer-subtract)
    list(APPEND S2OPC_LINKER_FLAGS -fsanitize=pointer-subtract)
  endif()
endif()

if(WITH_TSAN)
  list(APPEND S2OPC_COMPILER_FLAGS -fsanitize=thread)
  list(APPEND S2OPC_LINKER_FLAGS -fsanitize=thread -pie)
endif()

if(WITH_UBSAN)
  list(APPEND S2OPC_DEFINITIONS ROCKSDB_UBSAN_RUN)
  list(APPEND S2OPC_COMPILER_FLAGS -fsanitize=undefined)
  list(APPEND S2OPC_LINKER_FLAGS -fsanitize=undefined)
endif()

if(WITH_COVERAGE)
  list(APPEND S2OPC_COMPILER_FLAGS --coverage)
  list(APPEND S2OPC_LINK_LIBRARIES gcov)
endif()

if(WITH_GPERF_PROFILER)
  find_library(GPERF_PROFILER profiler)

  if (NOT GPERF_PROFILER)
    message(FATAL_ERROR "Could not find libprofiler")
  endif()

  list(APPPEND S2OPC_LINK_LIBRARIES profiler)
endif()

if(WITH_CLANG_SOURCE_COVERAGE)
  if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    message(FATAL_ERROR "Clang compiler is required to enable Clang source coverage")
  endif()

  list(APPEND S2OPC_COMPILER_FLAGS -fprofile-instr-generate -fcoverage-mapping)
  list(APPEND S2OPC_LINKER_FLAGS -fprofile-instr-generate -fcoverage-mapping)
endif()

if(WITH_PYS2OPC AND NOT WITH_NANO_EXTENDED)
  message(WARNING "PyS2OPC validation tests will fail if S2OPC test server compiled without WITH_NANO_EXTENDED set") 
endif()

# Add WITH_NANO_EXTENDED to compilation definition if option activated
list(APPEND S2OPC_DEFINITIONS $<$<BOOL:${WITH_NANO_EXTENDED}>:WITH_NANO_EXTENDED>)

### Define common functions ###

# Function to generate a C structure address space from an XML UA nodeset file to be loaded by embedded loader
function(s2opc_embed_address_space c_file_name xml_uanodeset_path)

  if(WITH_CONST_ADDSPACE)
    set(const_addspace "--const_addspace")
  endif()

  add_custom_command(
    OUTPUT ${c_file_name}
    DEPENDS ${xml_uanodeset_path}
    COMMAND ${PYTHON_EXECUTABLE} ${S2OPC_ROOT_PATH}/scripts/generate-s2opc-address-space.py ${xml_uanodeset_path} ${c_file_name} ${const_addspace}
    COMMENT "Generating address space ${c_file_name}"
    VERBATIM
    )

  set_source_files_properties(${c_file_name} PROPERTIES GENERATED TRUE)
  if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set_source_files_properties(${c_file_name} PROPERTIES COMPILE_FLAGS -Wno-missing-field-initializers)
  endif()

endfunction()
