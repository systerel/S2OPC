cmake_minimum_required (VERSION 3.5)

# The caller of this CMakeLists must define:
#   SAMPLE_PROJECT_NAME     as the project name
#   SAMPLE_SRC_DIR          as the source path of sample

# Demo CLI-PubSub-Server
set(SAMPLE_PTF_DEP ${CMAKE_CURRENT_LIST_DIR}/../../platform_dep)

# Handle ZEPHYR case forsample build
set(ZEPHYR_BASE_EXIST $ENV{ZEPHYR_BASE})
if(ZEPHYR_BASE_EXIST)
  include(../platform_dep/zephyr/CMakeLists.txt)
  return()
endif()

project (${SAMPLE_PROJECT_NAME} LANGUAGES C)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../../CommonDefs.cmake)
endif()

#### S2OPC CLI-PubSub-Server sample ####

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we built s2opc and sample separately
find_package(s2opc)

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

#### Samples for S2OPC PubSub library ####

# Options reserved to samples
option(ENABLE_SAMPLES "Build the S2OPC PubSub library samples" ON)

option(PUBSUB_STATIC_CONFIG "Load PubSub configuration statically" ON)

if(ENABLE_SAMPLES)

  if(UNIX)
    set(PLATFORM_DIR linux)
    set(S2OPC_COMPILER_FLAGS ${S2OPC_COMPILER_FLAGS} -D_GNU_SOURCE -Wno-error=date-time -Wno-date-time)
  elseif(WIN32)
    set(PLATFORM_DIR windows)
  else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
  endif()

  FILE(GLOB samples_src
    ${SAMPLE_SRC_DIR}/src/*.c
    ${SAMPLE_PTF_DEP}/${PLATFORM_DIR}/src/*.c
    )

  add_executable(${SAMPLE_PROJECT_NAME} ${samples_src})

  target_compile_options(${SAMPLE_PROJECT_NAME} PRIVATE ${S2OPC_COMPILER_FLAGS})
  target_compile_definitions(${SAMPLE_PROJECT_NAME} PRIVATE ${S2OPC_DEFINITIONS})
  target_include_directories(${SAMPLE_PROJECT_NAME} PRIVATE
        ${SAMPLE_SRC_DIR}/src
        ${SAMPLE_PTF_DEP}/include
        ${SAMPLE_PTF_DEP}/${PLATFORM_DIR}/src
        )
  if (WITH_STATIC_SECURITY_DATA)
    target_compile_definitions(${SAMPLE_PROJECT_NAME} PRIVATE "WITH_STATIC_SECURITY_DATA")
  endif()

  target_link_libraries(${SAMPLE_PROJECT_NAME} PRIVATE
  s2opc_pubsub
  s2opc_clientwrapper
  s2opc_serverwrapper
  s2opc_clientserver
  s2opc_clientserver-loader-embedded)


endif(ENABLE_SAMPLES)
