cmake_minimum_required(VERSION 3.20.0)

# include(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.s2opc.txt)
set(ENV{CONF_FILE} ${CMAKE_CURRENT_LIST_DIR}/prj.conf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(${SAMPLE_PROJECT_NAME})

if (NOT CONFIG_S2OPC)
message(FATAL_ERROR "CONFIG_S2OPC is expected to be set in $ENV{CONF_FILE}")
endif()

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

SET(mbedtls_config ${CMAKE_CURRENT_LIST_DIR}/../tls_config)
# target_include_directories(app PRIVATE ${tls_config}) # TODO remove
zephyr_include_directories(${mbedtls_config})

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
