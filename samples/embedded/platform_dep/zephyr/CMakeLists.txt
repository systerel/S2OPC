cmake_minimum_required(VERSION 3.20.0)

set(S2OPC_SAMPLE_ZEPHYR ON)

if(DEFINED BOARD AND NOT "${BOARD}" STREQUAL "")
  set(BOARD_CFG "${BOARD}")
elseif(DEFINED ENV{BOARD} AND NOT "$ENV{BOARD}" STREQUAL "")
  set(BOARD_CFG "$ENV{BOARD}")
  set(BOARD "${BOARD_CFG}" CACHE STRING "Zephyr board" FORCE)
else()
  message(FATAL_ERROR "BOARD must be provided (for example: west build -b <board>)")
endif()

set(CONF_FILE_CRYPTO "prj.conf")
if(DEFINED ENV{NOCRYTO} AND NOT "$ENV{NOCRYTO}" STREQUAL "")
  set(CONF_FILE_CRYPTO "prj-nocrypto.conf")
  set(S2OPC_CRYPTO_LIB "nocrypto")
else()
  add_compile_definitions(S2OPC_CRYPTO_MBEDTLS=1)
endif()

set(CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/${CONF_FILE_CRYPTO}")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/boards/${BOARD_CFG}.conf")
  list(APPEND CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/boards/${BOARD_CFG}.conf")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/boards/${BOARD_CFG}.overlay")
  set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_LIST_DIR}/boards/${BOARD_CFG}.overlay")
endif()

set(S2OPC_ROOT_PATH ${SAMPLE_SRC_DIR}/../../..)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(${SAMPLE_PROJECT_NAME})

if(NOT CONFIG_S2OPC)
  message(FATAL_ERROR "CONFIG_S2OPC is expected to be set in ${CONF_FILE}")
endif()

# App-level configuration used by the Zephyr sample sources.
add_compile_definitions(
  WITH_STATIC_SECURITY_DATA=1
  WITH_USER_ASSERT=1
  SOPC_HAS_FILESYSTEM=0
)
add_compile_options(-UNDEBUG -Werror)

# =====================
# S2OPC BUILD VARIABLES
# =====================
execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE BUILD_SRC_COMMIT)
string(REGEX REPLACE "\n$" "" BUILD_SRC_COMMIT "${BUILD_SRC_COMMIT}")
if("${BUILD_SRC_COMMIT}" STREQUAL "")
  set(BUILD_SRC_COMMIT "UNDEFINED")
endif()

add_compile_definitions(
  BUILD_SRC_COMMIT="${BUILD_SRC_COMMIT}"
  BUILD_DOCKER_ID="N.A."
  BUILD_BUILD_DATE=__DATE__" "__TIME__
)

set(common_include_dirs
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${S2OPC_ROOT_PATH}/samples/ClientServer/data/cert
)

zephyr_include_directories(${common_include_dirs})

file(GLOB app_sources
  ${CMAKE_CURRENT_LIST_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

if("${CONFIG_SOPC_CRYPTO_LIB_NAME}" STREQUAL "mbedtls")
  set(mbedtls_config ${CMAKE_CURRENT_LIST_DIR}/../mbedtls_config)

  if(EXISTS "${ZEPHYR_BASE}/../modules/mbedtls/configs/config-mbedtls.h")
    set(ZEPHYR_MBEDTLS_CONFIG_DIR "${ZEPHYR_BASE}/../modules/mbedtls/configs")
  elseif(EXISTS "${ZEPHYR_BASE}/modules/mbedtls/configs/config-mbedtls.h")
    set(ZEPHYR_MBEDTLS_CONFIG_DIR "${ZEPHYR_BASE}/modules/mbedtls/configs")
  elseif(EXISTS "${ZEPHYR_BASE}/../modules/crypto/mbedtls/configs/config-mbedtls.h")
    set(ZEPHYR_MBEDTLS_CONFIG_DIR "${ZEPHYR_BASE}/../modules/crypto/mbedtls/configs")
  else()
    message(FATAL_ERROR "config-mbedtls.h introuvable")
  endif()

  zephyr_include_directories(
    ${mbedtls_config}
    ${ZEPHYR_MBEDTLS_CONFIG_DIR}
  )

  if(EXISTS "${ZEPHYR_BASE}/../modules/mbedtls/include")
    zephyr_include_directories("${ZEPHYR_BASE}/../modules/mbedtls/include")
  endif()

  if(EXISTS "${ZEPHYR_BASE}/../modules/crypto/mbedtls/include")
    zephyr_include_directories("${ZEPHYR_BASE}/../modules/crypto/mbedtls/include")
  endif()

  file(GLOB mbedtls_sources
    ${mbedtls_config}/*.c
  )
  list(APPEND app_sources ${mbedtls_sources})
endif()

target_sources(app PRIVATE ${app_sources})
