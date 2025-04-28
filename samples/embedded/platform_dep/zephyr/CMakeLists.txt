cmake_minimum_required(VERSION 3.20.0)

# include(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.s2opc.txt)
set(BOARD_CFG $ENV{BOARD})
if(BOARD_CFG)
else()
message(FATAL_ERROR "BOARD must be set (and exported)")
endif()

set(CONF_FILE_CRYPTO "prj.conf")

if($ENV{NO_CRYTO})
    set(CONF_FILE_CRYPTO "prj-nocrypto.conf")
endif()

set(ENV{CONF_FILE} "${CMAKE_CURRENT_LIST_DIR}/${CONF_FILE_CRYPTO};${CMAKE_CURRENT_LIST_DIR}/boards/$ENV{BOARD}.conf")

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/boards/$ENV{BOARD}.overlay")
set(ENV{DTC_OVERLAY_FILE} "${CMAKE_CURRENT_LIST_DIR}/boards/$ENV{BOARD}.overlay")
endif()

set(S2OPC_ROOT_PATH ${SAMPLE_SRC_DIR}/../../..)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(${SAMPLE_PROJECT_NAME})

if (NOT CONFIG_S2OPC)
message(FATAL_ERROR "CONFIG_S2OPC is expected to be set in $ENV{CONF_FILE}")
endif()

# =====================
# S2OPC CONFIGURATION600820
# =====================
# Note: The constants used within sample code (e.g WITH_STATIC_SECURITY_DATA) have to be
# redefined here because the values provided in s2opc/zephyr/CMakelist.txt only applies
# to "MODULE" source code reference in the same CMakelist.txt (typically : s2opc/src)
add_definitions (-DWITH_STATIC_SECURITY_DATA=1)
add_definitions (-DWITH_USER_ASSERT=1)
add_definitions (-DSOPC_HAS_FILESYSTEM=0)
add_definitions (-UNDEBUG)
add_definitions (-Werror)

# =====================
# S2OPC BUILD VARIABLES
# =====================
execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE BUILD_SRC_COMMIT)
string(REGEX REPLACE "\n$" "" BUILD_SRC_COMMIT "${BUILD_SRC_COMMIT}")
if ("${BUILD_SRC_COMMIT}" STREQUAL "")
    set(BUILD_SRC_COMMIT "UNDEFINED")
endif()

add_definitions (-DBUILD_SRC_COMMIT="${BUILD_SRC_COMMIT}")
add_definitions (-DBUILD_DOCKER_ID="N.A.")
add_definitions (-DBUILD_BUILD_DATE=__DATE__\"\ \"__TIME__)

# =====================
# MBEDTLS SPECIFIC DEFS
# =====================
# MBEDTLS requires an applicative-defined configuration
# See CONFIG_MBEDTLS_USER_CONFIG_FILE
# This file is provided here

SET(mbedtls_config ${CMAKE_CURRENT_LIST_DIR}/../mbedtls_config)
zephyr_include_directories(${mbedtls_config})

SET(mbedtls "$ENV{ZEPHYR_BASE}/../modules/crypto/mbedtls/include")

zephyr_include_directories(${mbedtls})

# =====================
# APP INCLUDES
# =====================

SET(app_headers ${CMAKE_CURRENT_LIST_DIR}/../include/
    ${mbedtls_config}/
    ${CMAKE_CURRENT_SOURCE_DIR}/)
target_include_directories(app PRIVATE ${app_headers})

# =====================
# APP SOURCES
# =====================

FILE(GLOB app_sources
     ${CMAKE_CURRENT_LIST_DIR}/src/*.c
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
     ${mbedtls_config}/*.c)

target_sources(app PRIVATE ${app_sources} ${SAMPLES_SRC})

# =====================
# ZEPHYR-GENERATED INCLUDES
# =====================

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)
