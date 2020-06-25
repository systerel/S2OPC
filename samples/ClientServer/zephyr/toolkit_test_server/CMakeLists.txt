# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(toolkit_test_server)

add_definitions (-DS2OPC_CLIENTSERVER_ONLY=1)
add_definitions (-DWITH_STATIC_SECURITY_DATA=1 -DPUBSUB_STATIC_CONFIG=1 -DWITH_NANO_EXTENDED=1)
add_definitions (-DMY_IP_MASK="255.255.255.0" -DMY_IP_ETH0="192.0.2.10" -DMY_IP_GW="192.0.2.2" -DMY_IP6_ETH0="2001:db8::2" -DMY_IP6_MASK="ffff:ffff:")

# Add TEST include directory

SET(app_headers ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(app PRIVATE ${app_headers})

# =====================
# MBEDTLS SPECIFIC DEFS
# =====================

# custom config file
SET(mbedtls_config ${CMAKE_CURRENT_SOURCE_DIR}/src/tls_config)
target_include_directories(app PRIVATE ${tls_config})
zephyr_include_directories(${mbedtls_config})

FILE(GLOB tls_files ${CMAKE_CURRENT_SOURCE_DIR}/src/tls_config/*.c)
target_sources(app PRIVATE ${tls_files})

# ============================
# END OF MBEDTLS SPECIFIC DEFS
# ============================

# Add TEST source directory

FILE(GLOB app_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

target_sources(app PRIVATE ${app_sources})

# Add other path.
set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)
