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

# Manage paths that includes the multi-config name (Debug, Release, etc.) in output directory path such as MSVC:
# => We do not want to have the config subdirectory (Debug, etc.) to run the tests
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

### Common dependencies ###

## Manage static/shared property of external libraries

# Make CMake use static version of all dependencies by default
option(USE_STATIC_EXT_LIBS "S2OPC libraries and binaries depend on static version of external libraries" ON)
option(BUILD_SHARED_LIBS "Build dynamic libraries for S2OPC instead of static libraries" OFF)
if(USE_STATIC_EXT_LIBS)
  set(USE_STATIC_MBEDTLS_LIB ${USE_STATIC_EXT_LIBS})
  set(USE_STATIC_EXPAT_LIB ${USE_STATIC_EXT_LIBS})

  if(BUILD_SHARED_LIBS)
    message(WARNING "Both BUILD_SHARED_LIBS and USE_STATIC_EXT_LIBS are active: external libraries will still be linked statically (if available)")
  endif()

# else: USE_STATIC_<LIBNAME>_LIB can be set in a custom way, otherwise default CMake behavior is kept
endif()

## Expat dependency management

# redefine CMake behavior for find_package(expat ...) calling find_library(...) if needed
if (USE_STATIC_EXPAT_LIB)
  set(_expat_orig_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})

  if(WIN32)
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib .a)
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  endif()
endif()

option(S2OPC_CLIENTSERVER_ONLY "Only build the common library and client server libraries, tests, and samples (effectively excludes pubsub)." OFF)
option(S2OPC_PUBSUB_ONLY "Only build the common library and the pubsub library, tests, and samples (effectively excludes clientserver)." OFF)
if(S2OPC_CLIENTSERVER_ONLY AND NOT WITH_PYS2OPC)
  find_package(expat CONFIG) # if not found XML loaders will not be compiled
else()
  find_package(expat REQUIRED CONFIG)
endif()

if(NOT S2OPC_CLIENTSERVER_ONLY)
  # Paho library is used if available to add MQTT transport to the PubSub module
  find_package(eclipse-paho-mqtt-c CONFIG)
endif()

# Detect limitation on PubSub
if(WIN32 AND NOT S2OPC_CLIENTSERVER_ONLY)
  message(FATAL_ERROR "PubSub module requires platform dependent code not implemented yet on windows (see issue #747). Please set S2OPC_CLIENTSERVER_ONLY to exclude PubSub module.")
endif()

# Expat have specific requirements for static library and MSCV: XML_STATIC shall be defined by application using it
if(expat_FOUND)
  if(WIN32 AND NOT MINGW)
    if (USE_STATIC_EXPAT_LIB)
      target_compile_definitions(expat::expat INTERFACE "XML_STATIC")
    endif()
  endif()
endif()

# redefine CMake behavior for find_library(*)
if (USE_STATIC_EXPAT_LIB)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_expat_orig_lib_suffixes})
endif()

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
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_GNU}:-Wno-reserved-macro-identifier -Wimplicit -Wreturn-type -Wsequence-point -Wcast-qual -Wuninitialized -Wcast-align -Wstrict-prototypes -Wchar-subscripts -Wformat=2 -Wconversion -Wshadow -Wmissing-prototypes -Wdate-time -Wfloat-equal -Winit-self -Wjump-misses-init -Wlogical-op -Wnested-externs -Wnormalized -Wold-style-definition -Wpointer-arith -Wswitch-default -Wtrampolines >)
# Make optional some flags not supported with GCC < 6
list(APPEND S2OPC_COMPILER_FLAGS $<$<AND:${IS_GNU},$<VERSION_GREATER:${CMAKE_C_COMPILER_VERSION},6>>:-Wduplicated-cond -Wnull-dereference >)

# Add security hardening compilation options
option(SECURITY_HARDENING "Harden compilation options" OFF)
if (SECURITY_HARDENING)
  list(APPEND S2OPC_COMPILER_FLAGS $<$<AND:${IS_GNU},$<NOT:${IS_MINGW}>>:-fcf-protection -fstack-clash-protection -fstack-protector-strong>)
  # Set GNU definitions for security hardening
  list(APPEND S2OPC_DEFINITIONS $<${IS_GNU}:_FORTIFY_SOURCE=2 _GLIBCXX_ASSERTIONS>)
  # Force symbol stripping (only available for GNU compatible compilers)
  list(APPEND S2OPC_COMPILER_FLAGS $<${IS_GNU}:-s>)
  list(APPEND S2OPC_LINKER_FLAGS $<${IS_GNU}:-s>)
endif()
# Set GNU linker flags
list(APPEND S2OPC_LINKER_FLAGS $<$<AND:${IS_GNU},$<NOT:${IS_MINGW}>>:-Wl,-z,relro,-z,now>)

# If PIE explicitly requested, activate it for binaries linking
option(POSITION_INDEPENDENT_EXECUTABLE "Build position independent executable (-pie) which is a security good practice, but requires dependencies to be compiled with position independent code (-fPIC)" OFF)
if (POSITION_INDEPENDENT_EXECUTABLE)
  # necessary to build binaries as PIE (see CMake CMP0083)
  list(APPEND S2OPC_LINKER_FLAGS $<$<AND:${IS_GNU},$<NOT:${IS_MINGW}>>:-pie>)
endif()

# Set Clang compiler flags
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_CLANG}:-std=c99 -pedantic -fstack-protector -Wall -Wextra -Wunreachable-code>)
# Add security hardening compilation options
if (SECURITY_HARDENING)
  list(APPEND S2OPC_COMPILER_FLAGS $<${IS_CLANG}:-mspeculative-load-hardening>)
  # Set Clang definitions
  list(APPEND S2OPC_DEFINITIONS $<${IS_CLANG}:_FORTIFY_SOURCE=2>)
endif()
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
list(APPEND S2OPC_COMPILER_FLAGS $<${IS_MINGW}: -Wno-pedantic-ms-format>)
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
option(WITH_ASAN "build with ASAN/LSAN" OFF) # address sanitizer / leak sanitizer
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
# option to load static security data for embedded systems without filesystems
option(WITH_STATIC_SECURITY_DATA "Use static security data" OFF)
# option to put non-writeable data in const part of the memory
option(WITH_CONST_ADDSPACE "Generate an address space where non writeable attributes are const" OFF)

option(WITH_NO_ASSERT "Disable asserts" OFF)
option(WITH_USER_ASSERT "Enable user-defined failed assertion event" OFF)
option(WITH_MINIMAL_FOOTPRINT "Limit software footprint" OFF)

# option to enable GCC static analyser. The value from 1 to 5 corresponds to the verbosity level.
option(WITH_GCC_STATIC_ANALYSIS "activate GCC static analysis during compilation" OFF)

# Check project and option(s) are compatible

# Function to check only one option (option name provided) is activated for all calls to this function
function(check_mutually_exclusive_options options_names)
  foreach(option_name IN LISTS options_names)
    if(${${option_name}}) # Check variable of the option_name is defined
      message("-- ${option_name} S2OPC option set")
      set(OPTIONS_SET TRUE)
    endif()
  endforeach()
  if(${OPTIONS_SET})
    if(NOT WITH_OPTION_MUTUALLY_EXCLUSIVE) # Check no other option was recorded before
      set(WITH_OPTION_MUTUALLY_EXCLUSIVE ${options_names} PARENT_SCOPE)
    else()
      message(FATAL_ERROR "${options_names} option(s) set with mutually exclusive option ${WITH_OPTION_MUTUALLY_EXCLUSIVE}")
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
check_mutually_exclusive_options("WITH_ASAN;WITH_UBSAN")
check_mutually_exclusive_options("WITH_TSAN")
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
  check_not_activated_option("WITH_GCC_STATIC_ANALYSIS" "not a unix system")
endif()
check_debug_build_type("WITH_ASAN" "to set compilation flag '-fno-omit-frame-pointer'")
check_debug_build_type("WITH_TSAN" "to set compilation flag '-fno-omit-frame-pointer'")
check_debug_build_type("WITH_UBSAN" "to set compilation flag '-fno-omit-frame-pointer'")
# print options with no incompatibility
print_if_activated("WITH_NANO_EXTENDED")
print_if_activated("WITH_CONST_ADDSPACE")
print_if_activated("WITH_STATIC_SECURITY_DATA")
print_if_activated("SECURITY_HARDENING")

# Check specific options constraints and set necessary compilation flags

# check assertion redirection
print_if_activated("WITH_NO_ASSERT")
print_if_activated("WITH_USER_ASSERT")
print_if_activated("WITH_MINIMAL_FOOTPRINT")
if(WITH_NO_ASSERT)
  if (WITH_USER_ASSERT)
    message(FATAL_ERROR "WITH_USER_ASSERT incompatible option along with WITH_NO_ASSERT")
  endif() 
  list(APPEND S2OPC_COMPILER_FLAGS -DWITH_NO_ASSERT)
elseif(WITH_USER_ASSERT)
  list(APPEND S2OPC_COMPILER_FLAGS -DWITH_USER_ASSERT)
endif()
if(WITH_MINIMAL_FOOTPRINT)
  list(APPEND S2OPC_COMPILER_FLAGS -DWITH_MINIMAL_FOOTPRINT)
endif()

# Display remaining configuration flags
print_if_activated("PUBSUB_STATIC_CONFIG")

# check if compiler support new sanitization options
include(CheckCCompilerFlag)
set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address")
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-compare" COMPILER_SUPPORTS_SAN_PC)
CHECK_C_COMPILER_FLAG("-fsanitize=address -fsanitize=pointer-subtract" COMPILER_SUPPORTS_SAN_PS)
unset(CMAKE_REQUIRED_LIBRARIES)

if(WITH_ASAN)
  list(APPEND S2OPC_COMPILER_FLAGS -fsanitize=address)
  list(APPEND S2OPC_LINKER_FLAGS -fsanitize=address)
  list(APPEND S2OPC_LINKER_FLAGS -fsanitize=leak)
  list(APPEND S2OPC_LINKER_FLAGS -static-libasan)
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

if(WITH_GCC_STATIC_ANALYSIS)
  if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    message(FATAL_ERROR "GCC compiler is required to enable GCC Static Analysis")
  endif()  
  
  message("-- GCC Static Analysis activated")
  list(APPEND S2OPC_COMPILER_FLAGS -fanalyzer -fanalyzer-verbosity=${WITH_GCC_STATIC_ANALYSIS})
endif()

# Add WITH_NANO_EXTENDED to compilation definition if option activated
list(APPEND S2OPC_DEFINITIONS $<$<BOOL:${WITH_NANO_EXTENDED}>:WITH_NANO_EXTENDED>)

### Define common functions ###

# Function to generate build info C structure and function
function(s2opc_gen_build_info c_model_source_path c_file_target_path)
  if(CMAKE_HOST_UNIX)
    add_custom_command(OUTPUT ${c_file_target_path}
      COMMAND ${CMAKE_COMMAND} -E copy ${c_model_source_path} ${c_file_target_path}
      COMMAND ${S2OPC_ROOT_PATH}/scripts/gen_build_info_file.sh ${c_file_target_path}
      WORKING_DIRECTORY ${S2OPC_ROOT_PATH})
  else()
    add_custom_command(OUTPUT ${c_file_target_path}
      COMMAND ${CMAKE_COMMAND} -E copy  ${c_model_source_path} ${c_file_target_path}
      WORKING_DIRECTORY ${S2OPC_ROOT_PATH})
  endif()
  set_source_files_properties(${c_file_target_path} PROPERTIES GENERATED TRUE)
endfunction()

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

# Function that generates the expanded include required by CFFI to compile PyS2OPC
function(s2opc_expand_header h_input context_targets h_expanded)
  # Note: this function may seem overkill, as there are only a few PyS2OPC headers.
  # It was the basis of tests to avoid manual maintenance of the PyS2OPC's headers, required by CFFI.
  # CFFI is based on pycparser, and their combined purposes make this clearly and intentionally unfeasible.
  # pycparser does not do preprocessing work and does not support all tweaks that may reside in the standard library headers.
  # It results that it can parse C with tweaks, such as using fake stdlib headers.
  # However CFFI uses pycparser to obtain real types and generate some C afterwards with them.
  # This makes it clear that we cannot fake types of the stdlib
  # (otherwise, there are be conflicting types definitions at compile time).

  # To conclude, we shall only use CFFI on headers that don't rely on the libc,
  # except for the types it already knows: [u]int(8|16|32|64)_t.
  # (Even bool is not supported and its support in our headers should be platform-dependent)

  # Evaluate properties of target to get the ;-list, and produce the '-I;'-list,
  # that will be expanded as MULTIPLE ARGUMENTS thanks to the COMMAND_EXPAND_LISTS
  foreach(_context_target IN LISTS context_targets)
    set(_expand_eval_includes "$<TARGET_PROPERTY:${_context_target},INCLUDE_DIRECTORIES>")
    list(APPEND _expand_includes "$<$<BOOL:${_expand_eval_includes}>:-I$<JOIN:${_expand_eval_includes},\;-I>>")
    # Same for defines
    set(_expand_eval_defines "$<TARGET_PROPERTY:${_context_target},COMPILE_DEFINITIONS>")
    list(APPEND _expand_defines "$<$<BOOL:${_expand_eval_defines}>:-D$<JOIN:${_expand_eval_defines},\;-D>>")
  endforeach(_context_target)

  # Of course, M$ deprecated '-o', so we have to handle this manually... But at least there exists equivalents.
  if(WIN32 AND NOT MINGW)
    set(_output_switches /P /Fi${h_expanded})
  else()
    set(_output_switches -o ${h_expanded})
  endif()
  add_custom_command(DEPENDS ${h_input}
                     OUTPUT ${h_expanded}
                     COMMAND ${CMAKE_C_COMPILER} ${_expand_includes} ${_expand_defines}
                             -DS2OPC_PYEXPANSION
                             -E ${h_input} ${_output_switches}
                     COMMENT "Expending header file to ${h_expanded}"
                     VERBATIM
                     COMMAND_EXPAND_LISTS
                     )
endfunction()

## S2OPC fuzzing tests: to be run manually ##

# options with impact on the dependencies
option(ENABLE_TESTING "Build the S2OPC tests" ON)
option(ENABLE_FUZZING "Build the S2OPC fuzzers" OFF)

# Enable fuzzing if WITH_OSS_FUZZ is ON
if(WITH_OSS_FUZZ AND NOT ENABLE_FUZZING)
  message(FATAL_ERROR "'WITH_OSS_FUZZ' needs 'ENABLE_FUZZING'. Check that you have enabled it, or remove 'WITH_OSS_FUZZ'")
endif()

if(ENABLE_FUZZING)
  if (NOT WITH_OSS_FUZZ)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
      if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS "6.0.0")
        # We need at least CLang 6 to get LibFuzzer
        message(FATAL_ERROR "Your version of Clang (${CMAKE_C_COMPILER_VERSION}) is not recent enough to have LibFuzzer. You need at leats 6.0.0")
      else()
        # Local fuzzing
        function(s2opc_fuzzer fuzz_target_name src_file library includes)
          add_executable(${fuzz_target_name} "${src_file}")
          set_target_properties(${fuzz_target_name} PROPERTIES COMPILE_FLAGS "-fsanitize=fuzzer")
          set_target_properties(${fuzz_target_name} PROPERTIES LINK_FLAGS "-fsanitize=fuzzer")
          target_link_libraries(${fuzz_target_name} ${library})
          target_include_directories(${fuzz_target_name} PRIVATE ${includes})
          add_dependencies(fuzzers ${fuzz_target_name})
        endfunction()
      endif()
    else()
      # Clang is needed to get LibFuzzer
      message(FATAL_ERROR "Clang is needed to have LibFuzzer. You have ${CMAKE_C_COMPILER_ID}.")
    endif()
  else()
    # OSS Fuzzing
    function(s2opc_fuzzer fuzz_target_name src_file library includes)
      add_executable(${fuzz_target_name} "${src_file}")
      target_link_libraries(${fuzz_target_name} "${library}" $ENV{LIB_FUZZING_ENGINE})
      target_include_directories(${fuzz_target_name} PRIVATE ${includes})
      add_dependencies(fuzzers ${fuzz_target_name})
    endfunction()
  endif()

  add_custom_target(fuzzers)
endif(ENABLE_FUZZING)
