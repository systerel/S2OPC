if (CONFIG_S2OPC)

#TODO make this a wrapper around the main CMakeLists.txt

zephyr_interface_library_named(S2OPC)

add_definitions (-DS2OPC_CLIENTSERVER_ONLY=1)
add_definitions (-DWITH_STATIC_SECURITY_DATA=1)
add_definitions (-DPUBSUB_STATIC_CONFIG=1)
add_definitions (-DWITH_NANO_EXTENDED=1)
add_definitions (-DWITH_CONST_ADDSPACE=1)
add_definitions (-DSOPC_TIMER_RESOLUTION_MS=50)
add_definitions (-DSOPC_SC_CONNECTION_TIMEOUT_MS=60000)
add_definitions (-DSOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE=8192)
add_definitions (-DSOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH=8192)
add_definitions (-DSOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS=1)
add_definitions (-DSOPC_DEFAULT_SEND_MAX_NB_CHUNKS=1)
#add_definitions (-DSOPC_DEFAULT_MAX_STRING_LENGTH=256)
add_definitions (-DSOPC_DEFAULT_MAX_ARRAY_LENGTH=128)
add_definitions (-DSOPC_MAX_SECURE_CONNECTIONS=10)
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
    ${S2OPC_SRC}/Common/crypto/mbedtls
    ${S2OPC_SRC}/ClientServer/api_toolkit
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper
    ${S2OPC_SRC}/ClientServer/frontend/common_wrapper
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper
    ${S2OPC_SRC}/Common/helpers
    ${S2OPC_SRC}/Common/helpers_platform_dep
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr
    ${S2OPC_SRC}/ClientServer/configuration
    ${S2OPC_SRC}/Common/configuration
    ${S2OPC_SRC}/Common/opcua_types
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


zephyr_library_sources(
    ${S2OPC_SRC}/Common/helpers/sopc_helper_endianness_cfg.c
    ${S2OPC_SRC}/Common/helpers/sopc_numeric_range.c
    ${S2OPC_SRC}/Common/helpers/sopc_helper_uri.c
    ${S2OPC_SRC}/Common/helpers/sopc_event_handler.c
    ${S2OPC_SRC}/Common/helpers/sopc_singly_linked_list.c
    ${S2OPC_SRC}/Common/helpers/sopc_event_timer_manager.c
    ${S2OPC_SRC}/Common/helpers/sopc_dict.c
    ${S2OPC_SRC}/Common/helpers/sopc_async_queue.c
    ${S2OPC_SRC}/Common/helpers/sopc_hash.c
    ${S2OPC_SRC}/Common/helpers/sopc_buffer.c
    ${S2OPC_SRC}/Common/helpers/sopc_logger.c
    ${S2OPC_SRC}/Common/helpers/sopc_helper_string.c
    ${S2OPC_SRC}/Common/helpers/sopc_log_manager.c
    ${S2OPC_SRC}/Common/helpers/sopc_array.c
    ${S2OPC_SRC}/Common/helpers/sopc_assert.c
    ${S2OPC_SRC}/Common/helpers/sopc_time.c
    ${S2OPC_SRC}/Common/opcua_types/sopc_encodeabletype.c
    ${S2OPC_SRC}/Common/opcua_types/sopc_encoder.c
    ${S2OPC_SRC}/Common/opcua_types/sopc_types.c
    ${S2OPC_SRC}/Common/opcua_types/sopc_encodeable.c
    ${S2OPC_SRC}/Common/opcua_types/sopc_builtintypes.c
    ${S2OPC_SRC}/Common/crypto/sopc_pki.c
    ${S2OPC_SRC}/Common/crypto/sopc_key_manager.c
    ${S2OPC_SRC}/Common/crypto/mbedtls/crypto_profiles_lib.c
    ${S2OPC_SRC}/Common/crypto/mbedtls/crypto_functions_lib.c
    ${S2OPC_SRC}/Common/crypto/mbedtls/crypto_provider_lib.c
    ${S2OPC_SRC}/Common/crypto/mbedtls/key_manager_lib.c
    ${S2OPC_SRC}/Common/crypto/mbedtls/pki_stack.c
    ${S2OPC_SRC}/Common/crypto/sopc_secret_buffer.c
    ${S2OPC_SRC}/Common/crypto/sopc_key_sets.c
    ${S2OPC_SRC}/Common/crypto/sopc_crypto_provider.c
    ${S2OPC_SRC}/Common/crypto/sopc_crypto_profiles.c
    ${S2OPC_SRC}/Common/configuration/sopc_common_constants.c
    ${S2OPC_SRC}/Common/configuration/sopc_ieee_check.c
    ${S2OPC_SRC}/Common/configuration/sopc_common.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_mem_alloc.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_threads.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_filesystem.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_eth_sockets.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_sockets.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_synchro.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_time.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_atomic.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_udp_sockets.c
    ${S2OPC_SRC}/Common/helpers_platform_dep/zephyr/p_multicast.c
    ${S2OPC_SRC}/ClientServer/sockets/sopc_sockets_network_event_mgr.c
    ${S2OPC_SRC}/ClientServer/sockets/sopc_sockets_api.c
    ${S2OPC_SRC}/ClientServer/sockets/sopc_sockets_internal_ctx.c
    ${S2OPC_SRC}/ClientServer/sockets/sopc_sockets_event_mgr.c
    ${S2OPC_SRC}/ClientServer/services/b2c/app_cb_call_context_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/publish_request_queue_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/node_id_pointer_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/address_space_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_session_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_register_server2_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_register_nodes_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_subscription_publish_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/gen_subscription_event_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/response_write_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/monitored_item_pointer_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/user_authentication_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/call_method_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_unregister_nodes_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/service_response_cb_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_find_servers_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/util_discovery_services.c
    ${S2OPC_SRC}/ClientServer/services/b2c/write_value_pointer_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/argument_pointer_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_subscription_delete_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/channel_mgr_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/session_core_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/service_register_server2_set_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/session_request_handle_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/monitored_item_notification_queue_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_read_response_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_subscription_create_monitored_item_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_read_request_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/util_address_space.c
    ${S2OPC_SRC}/ClientServer/services/b2c/util_user.c
    ${S2OPC_SRC}/ClientServer/services/b2c/message_in_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/util_b2c.c
    ${S2OPC_SRC}/ClientServer/services/b2c/browse_treatment_result_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/service_mgr_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_call_method_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/time_reference_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_translate_browse_path_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/message_out_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/constants_statuscodes_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/monitored_item_queue_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_browse_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/notification_republish_queue_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/constants_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/browse_treatment_continuation_points_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_subscription_set_publishing_mode_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/user_authorization_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/util_variant.c
    ${S2OPC_SRC}/ClientServer/services/b2c/continuation_point_impl.c
    ${S2OPC_SRC}/ClientServer/services/b2c/service_write_decode_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/request_handle_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_browse_next_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_find_servers_on_network_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/address_space_typing_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/data_value_pointer_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/service_set_discovery_server_data_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/service_get_endpoints_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/subscription_core_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_subscription_create_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/msg_subscription_publish_ack_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/notification_republish_queue_it_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/browse_treatment_context_bs.c
    ${S2OPC_SRC}/ClientServer/services/b2c/monitored_item_queue_it_bs.c
    ${S2OPC_SRC}/ClientServer/services/sopc_services_api.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/unregister_nodes_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/io_dispatch_mgr.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/address_space_local.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment_continuation_points.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_register_nodes.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/msg_register_nodes.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/call_method_mgr.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/address_space_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/toolkit_header.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment_result_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/address_space.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment_1.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/register_nodes_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment_context.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/call_method_result_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_mgr_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_read_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_core_2.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_unregister_nodes.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/subscription_core_1.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/translate_browse_path.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/channel_mgr_1.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/user_authentication.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_set_discovery_server.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/channel_mgr.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/subscription_create_monitored_item_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/msg_unregister_nodes.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_channel_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/translate_browse_path_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_core_1_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment_continuation_points_session_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_core.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_browse_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/browse_treatment_target_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_register_server2.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/translate_browse_path_element_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_mgr.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_read.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/subscription_mgr.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/msg_register_server2.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/subscription_core.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_set_view.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/service_mgr.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_core_1.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/subscription_core_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/address_space_typing.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/constants.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/channel_mgr_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/msg_read_request.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/call_method_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/translate_browse_path_result_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/toolkit_header_init.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/session_core_it.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/translate_browse_path_1.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/msg_subscription_create_monitored_item.c
    ${S2OPC_SRC}/ClientServer/services/bgenc/translate_browse_path_source_it.c
    ${S2OPC_SRC}/ClientServer/loaders/address_space_loaders/embedded/sopc_addspace_loader.c
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/pki_permissive.c
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/toolkit_helpers.c
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/libs2opc_client_common.c
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/libs2opc_client_cmds.c
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/libs2opc_client.c
    ${S2OPC_SRC}/ClientServer/frontend/client_wrapper/state_machine.c
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper/libs2opc_server_config_custom.c
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper/libs2opc_server_config.c
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper/libs2opc_server_runtime_variables.c
    ${S2OPC_SRC}/ClientServer/frontend/server_wrapper/libs2opc_server.c
    ${S2OPC_SRC}/ClientServer/configuration/sopc_toolkit_config.c
    ${S2OPC_SRC}/ClientServer/configuration/sopc_call_method_manager.c
    ${S2OPC_SRC}/ClientServer/configuration/sopc_user.c
    ${S2OPC_SRC}/ClientServer/configuration/sopc_address_space.c
    ${S2OPC_SRC}/ClientServer/configuration/sopc_user_manager.c
    ${S2OPC_SRC}/ClientServer/api_toolkit/sopc_toolkit_async_api.c
    ${S2OPC_SRC}/ClientServer/api_toolkit/sopc_user_app_itf.c
    ${S2OPC_SRC}/ClientServer/api_toolkit/sopc_internal_app_dispatcher.c
    ${S2OPC_SRC}/ClientServer/secure_channels/sopc_chunks_mgr.c
    ${S2OPC_SRC}/ClientServer/secure_channels/sopc_secure_connection_state_mgr.c
    ${S2OPC_SRC}/ClientServer/secure_channels/sopc_secure_channels_internal_ctx.c
    ${S2OPC_SRC}/ClientServer/secure_channels/sopc_secure_listener_state_mgr.c
    ${S2OPC_SRC}/ClientServer/secure_channels/sopc_secure_channels_api.c
    ${S2OPC_SRC}/PubSub/common/sopc_pubsub_helpers.c
    ${S2OPC_SRC}/PubSub/common/sopc_pubsub_conf.c
    ${S2OPC_SRC}/PubSub/network/sopc_network_layer.c
    ${S2OPC_SRC}/PubSub/publisher/sopc_pub_source_variable.c
    ${S2OPC_SRC}/PubSub/publisher/sopc_pub_scheduler.c
    ${S2OPC_SRC}/PubSub/subscriber/sopc_reader_layer.c
    ${S2OPC_SRC}/PubSub/subscriber/sopc_sub_scheduler.c
    ${S2OPC_SRC}/PubSub/subscriber/sopc_sub_target_variable.c
    ${S2OPC_SRC}/PubSub/dataset/sopc_dataset_ll_layer.c
    ${S2OPC_SRC}/PubSub/dataset/sopc_dataset_layer.c
    ${S2OPC_SRC}/PubSub/protocol/sopc_pubsub_protocol.c
    ${S2OPC_SRC}/PubSub/protocol/sopc_sub_sockets_mgr.c
#Add security module files
    ${S2OPC_SRC}/PubSub/security/sopc_pubsub_security.c
    ${S2OPC_SRC}/PubSub/security/sopc_pubsub_local_sks.c
#Add mqtt module files
    ${S2OPC_SRC}/PubSub/protocol/sopc_mqtt_transport_layer.c
)

################################
#example of conditional include:
#zephyr_library_sources_ifdef(CONFIG_ZSL_SINGLE_PRECISION ${S2OPC_SRC}/xxx.c)

zephyr_library_link_libraries(S2OPC)
target_link_libraries(S2OPC INTERFACE zephyr_interface)

endif()
