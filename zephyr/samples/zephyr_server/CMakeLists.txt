cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(my_zephyr_app)

SET(COMMON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../zephyr_common_src)

# =====================
# S2OPC CONFIGURATION
# =====================
# Note: The constants used within sample code (e.g WITH_STATIC_SECURITY_DATA) have to be
# redefined here because the values provided in s2opc/zephyr/CMakelist.txt only applies
# to "MODULE" source code reference in the same CMakelist.txt (typically : s2opc/src)
add_definitions (-DWITH_STATIC_SECURITY_DATA=1)
add_definitions (-DWITH_USER_ASSERT=1)
add_definitions (-UNDEBUG)
add_definitions (-Werror)

# =====================
# MBEDTLS SPECIFIC DEFS
# =====================
# MBEDTLS requires an applicative-defined configuration
# See CONFIG_MBEDTLS_USER_CONFIG_FILE
# This file is provided here

SET(mbedtls_config ${COMMON_SRC_DIR}/tls_config)
target_include_directories(app PRIVATE ${tls_config})
zephyr_include_directories(${mbedtls_config})

# =====================
# APP INCLUDES
# =====================

SET(app_headers ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${COMMON_SRC_DIR}/tls_config
    ${COMMON_SRC_DIR}/)
target_include_directories(app PRIVATE ${app_headers})

# =====================
# APP SOURCES
# =====================

FILE(GLOB app_sources
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
     ${COMMON_SRC_DIR}/*.c
     ${COMMON_SRC_DIR}/tls_config/*.c)
target_sources(app PRIVATE ${app_sources})

# =====================
# ZEPHYR-GENERATED INCLUDES
# =====================

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)
