if (CONFIG_S2OPC)

#TODO make this a wrapper around the main CMakeLists.txt

zephyr_interface_library_named(S2OPC)

add_definitions (-DSOPC_PTR_SIZE=4)
add_definitions (-DS2OPC_CLIENTSERVER_ONLY=1)
add_definitions (-DWITH_STATIC_SECURITY_DATA=1 -DPUBSUB_STATIC_CONFIG=1 -DWITH_NANO_EXTENDED=1)
add_definitions (-DSOPC_TIMER_RESOLUTION_MS=50)
add_definitions (-DSOPC_SC_CONNECTION_TIMEOUT_MS=60000)
add_definitions (-DSOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE=8192)
add_definitions (-DSOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS=1)
add_definitions (-DSOPC_DEFAULT_SEND_MAX_NB_CHUNKS=1)
add_definitions (-DSOPC_DEFAULT_MAX_STRING_LENGTH=8192)
add_definitions (-DSOPC_DEFAULT_MAX_ARRAY_LENGTH=128)
#add_definitions (-DSOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL=100)
#add_definitions (-DSOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL=50)
add_definitions (-DSOPC_MAX_NB_ELEMENTS_ASYNC_QUEUE=128)



# Add S2OPC includes directories
zephyr_include_directories(
    ../src/ClientServer/loaders/address_space_loaders
    ../src/ClientServer/services
    ../src/ClientServer/services/bgenc
    ../src/ClientServer/services/b2c
    ../src/ClientServer/sockets
    ../src/ClientServer/secure_channels
    ../src/Common/crypto
    ../src/Common/crypto/mbedtls
    ../src/ClientServer/api_toolkit
    ../src/Common/helpers
    ../src/Common/helpers_platform_dep
    ../src/Common/helpers_platform_dep/zephyr
    ../src/ClientServer/configuration
    ../src/Common/configuration
    ../src/Common/opcua_types
    ../src/PubSub
    ../src/PubSub/common
    ../src/PubSub/dataset
    ../src/PubSub/network
    ../src/PubSub/protocol
    ../src/PubSub/publisher
    ../src/PubSub/subscriber
    ../src/PubSub/real_time_api
)

# Add S2OPC sources

#FILE(GLOB app_sources_csrc_loaders_address_space_loaders_embedded ../src/ClientServer/loaders/address_space_loaders/embedded/*.c)
#FILE(GLOB app_sources_csrc_services ../src/ClientServer/services/*.c)
#FILE(GLOB app_sources_csrc_services_bgenc ../src/ClientServer/services/bgenc/*.c)
#FILE(GLOB app_sources_csrc_services_b2c ../src/ClientServer/services/b2c/*.c)
#FILE(GLOB app_sources_csrc_sockets ../src/ClientServer/sockets/*.c)
#FILE(GLOB app_sources_csrc_secure_channels ../src/ClientServer/secure_channels/*.c)
#FILE(GLOB app_sources_csrc_crypto ../src/Common/crypto/*.c)
#FILE(GLOB app_sources_csrc_crypto_mbedtls ../src/Common/crypto/mbedtls/*.c)
#FILE(GLOB app_sources_csrc_api_toolkit ../src/ClientServer/api_toolkit/*.c)
#FILE(GLOB app_sources_csrc_helpers ../src/Common/helpers/*.c)
#FILE(GLOB app_sources_csrc_helpers_platform_dep ../src/Common/helpers_platform_dep/*.c)
#FILE(GLOB app_sources_csrc_helpers_platform_dep_zephyr ../src/Common/helpers_platform_dep/zephyr/*.c)
#FILE(GLOB app_sources_csrc_configuration ../src/ClientServer/configuration/*.c)
#FILE(GLOB app_sources_src_common_configuration ../src/Common/configuration/*.c)
#FILE(GLOB app_sources_csrc_opcua_types ../src/Common/opcua_types/*.c)

zephyr_library()

zephyr_library_sources(
../src/Common/helpers/sopc_helper_endianness_cfg.c
../src/Common/helpers/sopc_numeric_range.c
../src/Common/helpers/sopc_helper_uri.c
../src/Common/helpers/sopc_event_handler.c
../src/Common/helpers/sopc_singly_linked_list.c
../src/Common/helpers/sopc_event_timer_manager.c
../src/Common/helpers/sopc_dict.c
../src/Common/helpers/sopc_async_queue.c
../src/Common/helpers/sopc_hash.c
../src/Common/helpers/sopc_buffer.c
../src/Common/helpers/sopc_logger.c
../src/Common/helpers/sopc_helper_string.c
../src/Common/helpers/sopc_log_manager.c
../src/Common/helpers/sopc_array.c
../src/Common/opcua_types/sopc_encodeabletype.c
../src/Common/opcua_types/sopc_encoder.c
../src/Common/opcua_types/sopc_types.c
../src/Common/opcua_types/sopc_encodeable.c
../src/Common/opcua_types/sopc_builtintypes.c
../src/Common/crypto/sopc_pki.c
../src/Common/crypto/sopc_key_manager.c
../src/Common/crypto/mbedtls/crypto_profiles_lib.c
../src/Common/crypto/mbedtls/crypto_functions_lib.c
../src/Common/crypto/mbedtls/crypto_provider_lib.c
../src/Common/crypto/mbedtls/key_manager_lib.c
../src/Common/crypto/mbedtls/pki_stack.c
../src/Common/crypto/sopc_secret_buffer.c
../src/Common/crypto/sopc_key_sets.c
../src/Common/crypto/sopc_crypto_provider.c
../src/Common/crypto/sopc_crypto_profiles.c
../src/Common/configuration/sopc_common_constants.c
../src/Common/configuration/sopc_ieee_check.c
../src/Common/configuration/sopc_common.c
../src/Common/helpers_platform_dep/sopc_time.c
../src/Common/helpers_platform_dep/zephyr/p_mem_alloc.c
../src/Common/helpers_platform_dep/zephyr/p_threads.c
../src/Common/helpers_platform_dep/zephyr/p_filesystem.c
../src/Common/helpers_platform_dep/zephyr/p_sockets.c
../src/Common/helpers_platform_dep/zephyr/p_synchro.c
../src/Common/helpers_platform_dep/zephyr/p_time.c
../src/Common/helpers_platform_dep/zephyr/p_atomic.c
../src/Common/helpers_platform_dep/zephyr/p_udp_sockets.c
../src/Common/helpers_platform_dep/zephyr/p_multicast.c
../src/ClientServer/sockets/sopc_sockets_network_event_mgr.c
../src/ClientServer/sockets/sopc_sockets_api.c
../src/ClientServer/sockets/sopc_sockets_internal_ctx.c
../src/ClientServer/sockets/sopc_sockets_event_mgr.c
../src/ClientServer/services/b2c/publish_request_queue_bs.c
../src/ClientServer/services/b2c/node_id_pointer_bs.c
../src/ClientServer/services/b2c/address_space_bs.c
../src/ClientServer/services/b2c/msg_register_server2_bs.c
../src/ClientServer/services/b2c/msg_register_nodes_bs.c
../src/ClientServer/services/b2c/msg_subscription_publish_bs.c
../src/ClientServer/services/b2c/gen_subscription_event_bs.c
../src/ClientServer/services/b2c/response_write_bs.c
../src/ClientServer/services/b2c/monitored_item_pointer_bs.c
../src/ClientServer/services/b2c/user_authentication_bs.c
../src/ClientServer/services/b2c/call_method_bs.c
../src/ClientServer/services/b2c/msg_unregister_nodes_bs.c
../src/ClientServer/services/b2c/service_response_cb_bs.c
../src/ClientServer/services/b2c/msg_find_servers_bs.c
../src/ClientServer/services/b2c/util_discovery_services.c
../src/ClientServer/services/b2c/write_value_pointer_bs.c
../src/ClientServer/services/b2c/argument_pointer_bs.c
../src/ClientServer/services/b2c/msg_subscription_delete_bs.c
../src/ClientServer/services/b2c/channel_mgr_bs.c
../src/ClientServer/services/b2c/session_core_bs.c
../src/ClientServer/services/b2c/service_register_server2_set_bs.c
../src/ClientServer/services/b2c/session_request_handle_bs.c
../src/ClientServer/services/b2c/monitored_item_notification_queue_bs.c
../src/ClientServer/services/b2c/msg_read_response_bs.c
../src/ClientServer/services/b2c/msg_subscription_create_monitored_item_bs.c
../src/ClientServer/services/b2c/msg_read_request_bs.c
../src/ClientServer/services/b2c/util_address_space.c
../src/ClientServer/services/b2c/message_in_bs.c
../src/ClientServer/services/b2c/util_b2c.c
../src/ClientServer/services/b2c/browse_treatment_result_bs.c
../src/ClientServer/services/b2c/service_mgr_bs.c
../src/ClientServer/services/b2c/msg_call_method_bs.c
../src/ClientServer/services/b2c/time_reference_bs.c
../src/ClientServer/services/b2c/msg_translate_browse_path_bs.c
../src/ClientServer/services/b2c/message_out_bs.c
../src/ClientServer/services/b2c/constants_statuscodes_bs.c
../src/ClientServer/services/b2c/monitored_item_queue_bs.c
../src/ClientServer/services/b2c/msg_browse_bs.c
../src/ClientServer/services/b2c/notification_republish_queue_bs.c
../src/ClientServer/services/b2c/constants_bs.c
../src/ClientServer/services/b2c/browse_treatment_continuation_points_bs.c
../src/ClientServer/services/b2c/msg_subscription_set_publishing_mode_bs.c
../src/ClientServer/services/b2c/user_authorization_bs.c
../src/ClientServer/services/b2c/util_variant.c
../src/ClientServer/services/b2c/continuation_point_impl.c
../src/ClientServer/services/b2c/service_write_decode_bs.c
../src/ClientServer/services/b2c/request_handle_bs.c
../src/ClientServer/services/b2c/msg_browse_next_bs.c
../src/ClientServer/services/b2c/msg_find_servers_on_network_bs.c
../src/ClientServer/services/b2c/address_space_typing_bs.c
../src/ClientServer/services/b2c/data_value_pointer_bs.c
../src/ClientServer/services/b2c/service_set_discovery_server_data_bs.c
../src/ClientServer/services/b2c/service_get_endpoints_bs.c
../src/ClientServer/services/b2c/subscription_core_bs.c
../src/ClientServer/services/b2c/msg_subscription_create_bs.c
../src/ClientServer/services/b2c/msg_subscription_publish_ack_bs.c
../src/ClientServer/services/b2c/notification_republish_queue_it_bs.c
../src/ClientServer/services/b2c/browse_treatment_context_bs.c
../src/ClientServer/services/b2c/monitored_item_queue_it_bs.c
../src/ClientServer/services/sopc_services_api.c
../src/ClientServer/services/bgenc/unregister_nodes_it.c
../src/ClientServer/services/bgenc/io_dispatch_mgr.c
../src/ClientServer/services/bgenc/address_space_local.c
../src/ClientServer/services/bgenc/browse_treatment_continuation_points.c
../src/ClientServer/services/bgenc/service_register_nodes.c
../src/ClientServer/services/bgenc/msg_register_nodes.c
../src/ClientServer/services/bgenc/call_method_mgr.c
../src/ClientServer/services/bgenc/address_space_it.c
../src/ClientServer/services/bgenc/toolkit_header.c
../src/ClientServer/services/bgenc/browse_treatment_result_it.c
../src/ClientServer/services/bgenc/address_space.c
../src/ClientServer/services/bgenc/browse_treatment.c
../src/ClientServer/services/bgenc/browse_treatment_1.c
../src/ClientServer/services/bgenc/register_nodes_it.c
../src/ClientServer/services/bgenc/browse_treatment_context.c
../src/ClientServer/services/bgenc/call_method_result_it.c
../src/ClientServer/services/bgenc/session_mgr_it.c
../src/ClientServer/services/bgenc/service_read_it.c
../src/ClientServer/services/bgenc/session_core_2.c
../src/ClientServer/services/bgenc/service_unregister_nodes.c
../src/ClientServer/services/bgenc/subscription_core_1.c
../src/ClientServer/services/bgenc/translate_browse_path.c
../src/ClientServer/services/bgenc/channel_mgr_1.c
../src/ClientServer/services/bgenc/user_authentication.c
../src/ClientServer/services/bgenc/service_set_discovery_server.c
../src/ClientServer/services/bgenc/channel_mgr.c
../src/ClientServer/services/bgenc/subscription_create_monitored_item_it.c
../src/ClientServer/services/bgenc/msg_unregister_nodes.c
../src/ClientServer/services/bgenc/session_channel_it.c
../src/ClientServer/services/bgenc/translate_browse_path_it.c
../src/ClientServer/services/bgenc/session_core_1_it.c
../src/ClientServer/services/bgenc/browse_treatment_continuation_points_session_it.c
../src/ClientServer/services/bgenc/session_core.c
../src/ClientServer/services/bgenc/service_browse_it.c
../src/ClientServer/services/bgenc/browse_treatment_target_it.c
../src/ClientServer/services/bgenc/service_register_server2.c
../src/ClientServer/services/bgenc/translate_browse_path_element_it.c
../src/ClientServer/services/bgenc/session_mgr.c
../src/ClientServer/services/bgenc/service_read.c
../src/ClientServer/services/bgenc/subscription_mgr.c
../src/ClientServer/services/bgenc/msg_register_server2.c
../src/ClientServer/services/bgenc/subscription_core.c
../src/ClientServer/services/bgenc/service_set_view.c
../src/ClientServer/services/bgenc/service_mgr.c
../src/ClientServer/services/bgenc/session_core_1.c
../src/ClientServer/services/bgenc/subscription_core_it.c
../src/ClientServer/services/bgenc/address_space_typing.c
../src/ClientServer/services/bgenc/constants.c
../src/ClientServer/services/bgenc/channel_mgr_it.c
../src/ClientServer/services/bgenc/msg_read_request.c
../src/ClientServer/services/bgenc/call_method_it.c
../src/ClientServer/services/bgenc/translate_browse_path_result_it.c
../src/ClientServer/services/bgenc/toolkit_header_init.c
../src/ClientServer/services/bgenc/session_core_it.c
../src/ClientServer/services/bgenc/translate_browse_path_1.c
../src/ClientServer/services/bgenc/msg_subscription_create_monitored_item.c
../src/ClientServer/services/bgenc/translate_browse_path_source_it.c
../src/ClientServer/loaders/address_space_loaders/embedded/sopc_addspace_loader.c
../src/ClientServer/frontend/client_wrapper/pki_permissive.c
../src/ClientServer/frontend/client_wrapper/toolkit_helpers.c
../src/ClientServer/frontend/client_wrapper/libs2opc_client_common.c
../src/ClientServer/frontend/client_wrapper/libs2opc_client_cmds.c
../src/ClientServer/frontend/client_wrapper/libs2opc_client.c
../src/ClientServer/frontend/client_wrapper/state_machine.c
../src/ClientServer/configuration/sopc_toolkit_config.c
../src/ClientServer/configuration/sopc_call_method_manager.c
../src/ClientServer/configuration/sopc_user.c
../src/ClientServer/configuration/sopc_address_space.c
../src/ClientServer/configuration/sopc_user_manager.c
../src/ClientServer/api_toolkit/sopc_toolkit_async_api.c
../src/ClientServer/api_toolkit/sopc_user_app_itf.c
../src/ClientServer/api_toolkit/sopc_internal_app_dispatcher.c
../src/ClientServer/secure_channels/sopc_chunks_mgr.c
../src/ClientServer/secure_channels/sopc_secure_connection_state_mgr.c
../src/ClientServer/secure_channels/sopc_secure_channels_internal_ctx.c
../src/ClientServer/secure_channels/sopc_secure_listener_state_mgr.c
../src/ClientServer/secure_channels/sopc_secure_channels_api.c
../src/PubSub/common/sopc_pubsub_helpers.c
../src/PubSub/common/sopc_pubsub_conf.c
../src/PubSub/real_time_api/sopc_rt_subscriber.c
../src/PubSub/real_time_api/sopc_msgbox.c
../src/PubSub/real_time_api/sopc_rt_publisher.c
../src/PubSub/real_time_api/sopc_interrupttimer.c
../src/PubSub/real_time_api/sopc_doublebuffer.c
../src/PubSub/network/sopc_network_layer.c
../src/PubSub/publisher/sopc_pub_source_variable.c
../src/PubSub/publisher/sopc_pub_scheduler.c
../src/PubSub/subscriber/sopc_reader_layer.c
../src/PubSub/subscriber/sopc_sub_scheduler.c
../src/PubSub/subscriber/sopc_sub_target_variable.c
../src/PubSub/dataset/sopc_dataset_ll_layer.c
../src/PubSub/dataset/sopc_dataset_layer.c
../src/PubSub/protocol/sopc_pubsub_protocol.c
../src/PubSub/protocol/sopc_sub_udp_sockets_mgr.c
)

zephyr_library_link_libraries(S2OPC)
target_link_libraries(S2OPC INTERFACE zephyr_interface)

endif()
