if (CONFIG_S2OPC)

zephyr_interface_library_named(S2OPC)

# This macro is only available for MbedTLS versions 3.5.0 and higher
add_definitions (-DMBEDTLS_CONFIG_FILE="${ZEPHYR_BASE}/modules/mbedtls/configs/config-tls-generic.h")

add_definitions (-DWITH_STATIC_SECURITY_DATA=1)
add_definitions (-DPUBSUB_STATIC_CONFIG=1)
add_definitions (-DWITH_CONST_ADDSPACE=1)
add_definitions (-DSOPC_TIMER_RESOLUTION_MS=50)
add_definitions (-DSOPC_SC_CONNECTION_TIMEOUT_MS=60000)
add_definitions (-DSOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE=8192)
add_definitions (-DSOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH=8192)
add_definitions (-DSOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS=1)
add_definitions (-DSOPC_HAS_FILESYSTEM=0)
add_definitions (-DSOPC_DEFAULT_SEND_MAX_NB_CHUNKS=1)
#add_definitions (-DSOPC_DEFAULT_MAX_STRING_LENGTH=256)
add_definitions (-DSOPC_DEFAULT_MAX_ARRAY_LENGTH=128)
add_definitions (-DSOPC_MAX_SECURE_CONNECTIONS=11)
add_definitions (-DSOPC_MAX_SOCKETS=50)
add_definitions (-DSOPC_MAX_SESSIONS=10)
add_definitions (-DSOPC_MAX_PENDING_REQUESTS=64)

add_definitions (-DSOPC_PUBSUB_BUFFER_SIZE=256)
add_definitions (-DSOPC_PUBSUB_MAX_PUBLISHER_PER_SCHEDULER=1)
add_definitions (-DSOPC_PUBSUB_MAX_MESSAGE_PER_PUBLISHER=4)

#add_definitions (-DWITH_NO_ASSERT=1)
add_definitions (-DWITH_USER_ASSERT=1)
add_definitions (-DWITH_MINIMAL_FOOTPRINT=0)

#add_definitions (-DSOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL=100)
#add_definitions (-DSOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL=50)
add_definitions (-DSOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE=16)
add_definitions (-DUSE_MQTT_PAHO=0)
add_definitions (-Werror)
# UNDEBUG prevents "assert" to be removed (this generates warnings on unused variables)
add_definitions (-UNDEBUG)

set(S2OPC_SRC ${ZEPHYR_CURRENT_MODULE_DIR}/src)

# Add S2OPC includes directories
zephyr_include_directories(
    ${S2OPC_SRC}/ClientServer/loaders/address_space_loaders
    ${S2OPC_SRC}/ClientServer/services
    ${S2OPC_SRC}/ClientServer/services/bgenc
    ${S2OPC_SRC}/ClientServer/services/b2c
    ${S2OPC_SRC}/ClientServer/sockets
    ${S2OPC_SRC}/ClientServer/secure_channels
    ${S2OPC_SRC}/Common/crypto
    ${S2OPC_SRC}/Common/crypto/lib_itf
    ${S2OPC_SRC}/Common/crypto/lib_dep/${CONFIG_SOPC_CRYPTO_LIB_NAME}
    ${S2OPC_SRC}/ClientServer/api_toolkit
    ${S2OPC_SRC}/ClientServer/frontend/helper_wrapper
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/deprecated
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/internal
    ${S2OPC_SRC}/ClientServer/frontend/common_wrapper
    ${S2OPC_SRC}/ClientServer/frontend/common_wrapper/internal
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper/internal
    ${S2OPC_SRC}/Common/helpers
    ${S2OPC_SRC}/Common/helpers_platform_dep
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr
    ${S2OPC_SRC}/ClientServer/address_space
    ${S2OPC_SRC}/ClientServer/address_space/internal
    ${S2OPC_SRC}/ClientServer/configuration
    ${S2OPC_SRC}/Common/configuration
    ${S2OPC_SRC}/Common/opcua_types
    ${S2OPC_SRC}/Common/sks
    ${S2OPC_SRC}/PubSub
    ${S2OPC_SRC}/PubSub/config_loaders/xml_expat
    ${S2OPC_SRC}/PubSub/common
    ${S2OPC_SRC}/PubSub/dataset
    ${S2OPC_SRC}/PubSub/network
    ${S2OPC_SRC}/PubSub/protocol
    ${S2OPC_SRC}/PubSub/publisher
    ${S2OPC_SRC}/PubSub/subscriber
    ${S2OPC_SRC}/PubSub/real_time_api
    ${S2OPC_SRC}/PubSub/security
)

zephyr_library()

# Include all Common sources, except "helpers_platform_dep"
file(GLOB_RECURSE SOPC_COMMON_SRC ${S2OPC_SRC}/Common/*.c)
list(FILTER SOPC_COMMON_SRC EXCLUDE REGEX "${S2OPC_SRC}/Common/helpers_platform_dep/.*" )
list(FILTER SOPC_COMMON_SRC EXCLUDE REGEX "${S2OPC_SRC}/Common/crypto/lib_dep/.*" )

# Include specific "helpers_platform_dep" files
file(GLOB SOPC_ZEPHYR_SRC ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/*.c
     ${S2OPC_SRC}/Common/helpers_platform_dep/*.c)

# Include Crypto library specific files
if(NOT IS_DIRECTORY ${S2OPC_SRC}/Common/crypto/lib_dep/${CONFIG_SOPC_CRYPTO_LIB_NAME})
  message(FATAL_ERROR "Crypto library '${CONFIG_SOPC_CRYPTO_LIB_NAME}' not found. Please check the parameter 'CONFIG_SOPC_CRYPTO_LIB_NAME'")
else()
 message("-- SOPC is using crypto library '${CONFIG_SOPC_CRYPTO_LIB_NAME}'")
endif()
file(GLOB SOPC_CRYPTO_SRC ${S2OPC_SRC}/Common/crypto/lib_dep/${CONFIG_SOPC_CRYPTO_LIB_NAME}/*.c)

# Include other files
file(GLOB_RECURSE SOPC_CLIENT_SERVER_SRC ${S2OPC_SRC}/ClientServer/*.c)
# Exclude files for XML configuration management
list(FILTER SOPC_CLIENT_SERVER_SRC EXCLUDE REGEX "${S2OPC_SRC}/ClientServer/loaders/config_loaders/.*" )
list(FILTER SOPC_CLIENT_SERVER_SRC EXCLUDE REGEX "${S2OPC_SRC}/ClientServer/loaders/address_space_loaders/xml_expat/.*" )
list(FILTER SOPC_CLIENT_SERVER_SRC EXCLUDE REGEX "${S2OPC_SRC}/ClientServer/loaders/helpers/.*" )
list(FILTER SOPC_CLIENT_SERVER_SRC EXCLUDE REGEX "${S2OPC_SRC}/ClientServer/frontend/server_wrapper/libs2opc_server_config_xml.*" )
list(FILTER SOPC_CLIENT_SERVER_SRC EXCLUDE REGEX "${S2OPC_SRC}/ClientServer/frontend/client_wrapper/libs2opc_client_config_xml.*" )

file(GLOB_RECURSE SOPC_PUBSUB_SRC ${S2OPC_SRC}/PubSub/*.c)
list(FILTER SOPC_PUBSUB_SRC EXCLUDE REGEX "${S2OPC_SRC}/PubSub/config_loaders/.*" )

zephyr_library_sources(
    ${SOPC_COMMON_SRC}
    ${SOPC_CRYPTO_SRC}
    ${SOPC_ZEPHYR_SRC}
    ${SOPC_CLIENT_SERVER_SRC}
    ${SOPC_PUBSUB_SRC}
)

################################
#example of conditional include:
#zephyr_library_sources_ifdef(CONFIG_ZSL_SINGLE_PRECISION ${S2OPC_SRC}/xxx.c)

zephyr_library_link_libraries(S2OPC)
target_link_libraries(S2OPC INTERFACE zephyr_interface)

endif()
