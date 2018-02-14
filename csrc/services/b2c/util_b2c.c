/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util_b2c.h"

#include "opcua_identifiers.h"
#include "sopc_crypto_profiles.h"

void util_message__get_encodeable_type(const constants__t_msg_type_i message__msg_type,
                                       SOPC_EncodeableType** reqEncType,
                                       SOPC_EncodeableType** respEncType,
                                       t_bool* isRequest)
{
    switch (message__msg_type)
    {
    case constants__e_msg_discovery_find_servers_req:
        *reqEncType = &OpcUa_FindServersRequest_EncodeableType;
        *respEncType = &OpcUa_FindServersResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_discovery_find_servers_resp:
        *reqEncType = &OpcUa_FindServersRequest_EncodeableType;
        *respEncType = &OpcUa_FindServersResponse_EncodeableType;
        break;
    case constants__e_msg_discovery_find_servers_on_network_req:
        *reqEncType = &OpcUa_FindServersOnNetworkRequest_EncodeableType;
        *respEncType = &OpcUa_FindServersOnNetworkResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_discovery_find_servers_on_network_resp:
        *reqEncType = &OpcUa_FindServersOnNetworkRequest_EncodeableType;
        *respEncType = &OpcUa_FindServersOnNetworkResponse_EncodeableType;
        break;
    case constants__e_msg_discovery_get_endpoints_req:
        *reqEncType = &OpcUa_GetEndpointsRequest_EncodeableType;
        *respEncType = &OpcUa_GetEndpointsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_discovery_get_endpoints_resp:
        *reqEncType = &OpcUa_GetEndpointsRequest_EncodeableType;
        *respEncType = &OpcUa_GetEndpointsResponse_EncodeableType;
        break;
    case constants__e_msg_discovery_register_server_req:
        *reqEncType = &OpcUa_RegisterServerRequest_EncodeableType;
        *respEncType = &OpcUa_RegisterServerResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_discovery_register_server_resp:
        *reqEncType = &OpcUa_RegisterServerRequest_EncodeableType;
        *respEncType = &OpcUa_RegisterServerResponse_EncodeableType;
        break;
    case constants__e_msg_discovery_register_server2_req:
        *reqEncType = &OpcUa_RegisterServer2Request_EncodeableType;
        *respEncType = &OpcUa_RegisterServer2Response_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_discovery_register_server2_resp:
        *reqEncType = &OpcUa_RegisterServer2Request_EncodeableType;
        *respEncType = &OpcUa_RegisterServer2Response_EncodeableType;
        break;
    case constants__e_msg_session_create_req:
        *reqEncType = &OpcUa_CreateSessionRequest_EncodeableType;
        *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_session_create_resp:
        *reqEncType = &OpcUa_CreateSessionRequest_EncodeableType;
        *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
        break;
    case constants__e_msg_session_activate_req:
        *reqEncType = &OpcUa_ActivateSessionRequest_EncodeableType;
        *respEncType = &OpcUa_ActivateSessionResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_session_activate_resp:
        *reqEncType = &OpcUa_ActivateSessionRequest_EncodeableType;
        *respEncType = &OpcUa_ActivateSessionResponse_EncodeableType;
        break;
    case constants__e_msg_session_close_req:
        *reqEncType = &OpcUa_CloseSessionRequest_EncodeableType;
        *respEncType = &OpcUa_CloseSessionResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_session_close_resp:
        *reqEncType = &OpcUa_CloseSessionRequest_EncodeableType;
        *respEncType = &OpcUa_CloseSessionResponse_EncodeableType;
        break;
    case constants__e_msg_session_cancel_req:
        *reqEncType = &OpcUa_CancelRequest_EncodeableType;
        *respEncType = &OpcUa_CancelResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_session_cancel_resp:
        *reqEncType = &OpcUa_CancelRequest_EncodeableType;
        *respEncType = &OpcUa_CancelResponse_EncodeableType;
        break;
    case constants__e_msg_node_add_nodes_req:
        *reqEncType = &OpcUa_AddNodesRequest_EncodeableType;
        *respEncType = &OpcUa_AddNodesResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_node_add_nodes_resp:
        *reqEncType = &OpcUa_AddNodesRequest_EncodeableType;
        *respEncType = &OpcUa_AddNodesResponse_EncodeableType;
        break;
    case constants__e_msg_node_add_references_req:
        *reqEncType = &OpcUa_AddReferencesRequest_EncodeableType;
        *respEncType = &OpcUa_AddReferencesResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_node_add_references_resp:
        *reqEncType = &OpcUa_AddReferencesRequest_EncodeableType;
        *respEncType = &OpcUa_AddReferencesResponse_EncodeableType;
        break;
    case constants__e_msg_node_delete_nodes_req:
        *reqEncType = &OpcUa_DeleteNodesRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteNodesResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_node_delete_nodes_resp:
        *reqEncType = &OpcUa_DeleteNodesRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteNodesResponse_EncodeableType;
        break;
    case constants__e_msg_node_delete_references_req:
        *reqEncType = &OpcUa_DeleteReferencesRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteReferencesResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_node_delete_references_resp:
        *reqEncType = &OpcUa_DeleteReferencesRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteReferencesResponse_EncodeableType;
        break;
    case constants__e_msg_view_browse_req:
        *reqEncType = &OpcUa_BrowseRequest_EncodeableType;
        *respEncType = &OpcUa_BrowseResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_view_browse_resp:
        *reqEncType = &OpcUa_BrowseRequest_EncodeableType;
        *respEncType = &OpcUa_BrowseResponse_EncodeableType;
        break;
    case constants__e_msg_view_browse_next_req:
        *reqEncType = &OpcUa_BrowseNextRequest_EncodeableType;
        *respEncType = &OpcUa_BrowseNextResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_view_browse_next_resp:
        *reqEncType = &OpcUa_BrowseNextRequest_EncodeableType;
        *respEncType = &OpcUa_BrowseNextResponse_EncodeableType;
        break;
    case constants__e_msg_view_translate_browse_paths_to_node_ids_req:
        *reqEncType = &OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType;
        *respEncType = &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_view_translate_browse_paths_to_node_ids_resp:
        *reqEncType = &OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType;
        *respEncType = &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType;
        break;
    case constants__e_msg_view_register_nodes_req:
        *reqEncType = &OpcUa_RegisterNodesRequest_EncodeableType;
        *respEncType = &OpcUa_RegisterNodesResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_view_register_nodes_resp:
        *reqEncType = &OpcUa_RegisterNodesRequest_EncodeableType;
        *respEncType = &OpcUa_RegisterNodesResponse_EncodeableType;
        break;
    case constants__e_msg_view_unregister_nodes_req:
        *reqEncType = &OpcUa_UnregisterNodesRequest_EncodeableType;
        *respEncType = &OpcUa_UnregisterNodesResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_view_unregister_nodes_resp:
        *reqEncType = &OpcUa_UnregisterNodesRequest_EncodeableType;
        *respEncType = &OpcUa_UnregisterNodesResponse_EncodeableType;
        break;
    case constants__e_msg_query_first_req:
        *reqEncType = &OpcUa_QueryFirstRequest_EncodeableType;
        *respEncType = &OpcUa_QueryFirstResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_query_first_resp:
        *reqEncType = &OpcUa_QueryFirstRequest_EncodeableType;
        *respEncType = &OpcUa_QueryFirstResponse_EncodeableType;
        break;
    case constants__e_msg_query_next_req:
        *reqEncType = &OpcUa_QueryNextRequest_EncodeableType;
        *respEncType = &OpcUa_QueryNextResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_query_next_resp:
        *reqEncType = &OpcUa_QueryNextRequest_EncodeableType;
        *respEncType = &OpcUa_QueryNextResponse_EncodeableType;
        break;
    case constants__e_msg_attribute_read_req:
        *reqEncType = &OpcUa_ReadRequest_EncodeableType;
        *respEncType = &OpcUa_ReadResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_attribute_read_resp:
        *reqEncType = &OpcUa_ReadRequest_EncodeableType;
        *respEncType = &OpcUa_ReadResponse_EncodeableType;
        break;
    case constants__e_msg_attribute_history_read_req:
        *reqEncType = &OpcUa_HistoryReadRequest_EncodeableType;
        *respEncType = &OpcUa_HistoryReadResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_attribute_history_read_resp:
        *reqEncType = &OpcUa_HistoryReadRequest_EncodeableType;
        *respEncType = &OpcUa_HistoryReadResponse_EncodeableType;
        break;
    case constants__e_msg_attribute_write_req:
        *reqEncType = &OpcUa_WriteRequest_EncodeableType;
        *respEncType = &OpcUa_WriteResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_attribute_write_resp:
        *reqEncType = &OpcUa_WriteRequest_EncodeableType;
        *respEncType = &OpcUa_WriteResponse_EncodeableType;
        break;
    case constants__e_msg_attribute_history_update_req:
        *reqEncType = &OpcUa_HistoryUpdateRequest_EncodeableType;
        *respEncType = &OpcUa_HistoryUpdateResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_attribute_history_update_resp:
        *reqEncType = &OpcUa_HistoryUpdateRequest_EncodeableType;
        *respEncType = &OpcUa_HistoryUpdateResponse_EncodeableType;
        break;
    case constants__e_msg_method_call_req:
        *reqEncType = &OpcUa_CallRequest_EncodeableType;
        *respEncType = &OpcUa_CallResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_method_call_resp:
        *reqEncType = &OpcUa_CallRequest_EncodeableType;
        *respEncType = &OpcUa_CallResponse_EncodeableType;
        break;
    case constants__e_msg_monitored_items_create_req:
        *reqEncType = &OpcUa_CreateMonitoredItemsRequest_EncodeableType;
        *respEncType = &OpcUa_CreateMonitoredItemsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_monitored_items_create_resp:
        *reqEncType = &OpcUa_CreateMonitoredItemsRequest_EncodeableType;
        *respEncType = &OpcUa_CreateMonitoredItemsResponse_EncodeableType;
        break;
    case constants__e_msg_monitored_items_modify_req:
        *reqEncType = &OpcUa_ModifyMonitoredItemsRequest_EncodeableType;
        *respEncType = &OpcUa_ModifyMonitoredItemsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_monitored_items_modify_resp:
        *reqEncType = &OpcUa_ModifyMonitoredItemsRequest_EncodeableType;
        *respEncType = &OpcUa_ModifyMonitoredItemsResponse_EncodeableType;
        break;
    case constants__e_msg_monitored_items_set_monitoring_mode_req:
        *reqEncType = &OpcUa_SetMonitoringModeRequest_EncodeableType;
        *respEncType = &OpcUa_SetMonitoringModeResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_monitored_items_set_monitoring_mode_resp:
        *reqEncType = &OpcUa_SetMonitoringModeRequest_EncodeableType;
        *respEncType = &OpcUa_SetMonitoringModeResponse_EncodeableType;
        break;
    case constants__e_msg_monitored_items_set_triggering_req:
        *reqEncType = &OpcUa_SetTriggeringRequest_EncodeableType;
        *respEncType = &OpcUa_SetTriggeringResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_monitored_items_set_triggering_resp:
        *reqEncType = &OpcUa_SetTriggeringRequest_EncodeableType;
        *respEncType = &OpcUa_SetTriggeringResponse_EncodeableType;
        break;
    case constants__e_msg_monitored_items_delete_req:
        *reqEncType = &OpcUa_DeleteMonitoredItemsRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteMonitoredItemsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_monitored_items_delete_resp:
        *reqEncType = &OpcUa_DeleteMonitoredItemsRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteMonitoredItemsResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_create_req:
        *reqEncType = &OpcUa_CreateSubscriptionRequest_EncodeableType;
        *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_create_resp:
        *reqEncType = &OpcUa_CreateSubscriptionRequest_EncodeableType;
        *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_modify_req:
        *reqEncType = &OpcUa_ModifySubscriptionRequest_EncodeableType;
        *respEncType = &OpcUa_ModifySubscriptionResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_modify_resp:
        *reqEncType = &OpcUa_ModifySubscriptionRequest_EncodeableType;
        *respEncType = &OpcUa_ModifySubscriptionResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_set_publishing_mode_req:
        *reqEncType = &OpcUa_SetPublishingModeRequest_EncodeableType;
        *respEncType = &OpcUa_SetPublishingModeResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_set_publishing_mode_resp:
        *reqEncType = &OpcUa_SetPublishingModeRequest_EncodeableType;
        *respEncType = &OpcUa_SetPublishingModeResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_publish_req:
        *reqEncType = &OpcUa_PublishRequest_EncodeableType;
        *respEncType = &OpcUa_PublishResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_publish_resp:
        *reqEncType = &OpcUa_PublishRequest_EncodeableType;
        *respEncType = &OpcUa_PublishResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_republish_req:
        *reqEncType = &OpcUa_RepublishRequest_EncodeableType;
        *respEncType = &OpcUa_RepublishResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_republish_resp:
        *reqEncType = &OpcUa_RepublishRequest_EncodeableType;
        *respEncType = &OpcUa_RepublishResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_transfer_subscriptions_req:
        *reqEncType = &OpcUa_TransferSubscriptionsRequest_EncodeableType;
        *respEncType = &OpcUa_TransferSubscriptionsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_transfer_subscriptions_resp:
        *reqEncType = &OpcUa_TransferSubscriptionsRequest_EncodeableType;
        *respEncType = &OpcUa_TransferSubscriptionsResponse_EncodeableType;
        break;
    case constants__e_msg_subscription_delete_subscriptions_req:
        *reqEncType = &OpcUa_DeleteSubscriptionsRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteSubscriptionsResponse_EncodeableType;
        *isRequest = true;
        break;
    case constants__e_msg_subscription_delete_subscriptions_resp:
        *reqEncType = &OpcUa_DeleteSubscriptionsRequest_EncodeableType;
        *respEncType = &OpcUa_DeleteSubscriptionsResponse_EncodeableType;
        break;
    case constants__e_msg_service_fault_resp:
        *reqEncType = NULL;
        *respEncType = &OpcUa_ServiceFault_EncodeableType;
        break;
    default:
        printf("util_message__get_encodeable_type: not implemented message type required\n");
        exit(1);
        ;
    }
}

void util_message__get_message_type(SOPC_EncodeableType* encType, constants__t_msg_type_i* message__msg_type)
{
    switch (encType->TypeId)
    {
    case OpcUaId_FindServersRequest:
        *message__msg_type = constants__e_msg_discovery_find_servers_req;
        break;
    case OpcUaId_FindServersResponse:
        *message__msg_type = constants__e_msg_discovery_find_servers_resp;
        break;
    case OpcUaId_FindServersOnNetworkRequest:
        *message__msg_type = constants__e_msg_discovery_find_servers_on_network_req;
        break;
    case OpcUaId_FindServersOnNetworkResponse:
        *message__msg_type = constants__e_msg_discovery_find_servers_on_network_resp;
        break;
    case OpcUaId_GetEndpointsRequest:
        *message__msg_type = constants__e_msg_discovery_get_endpoints_req;
        break;
    case OpcUaId_GetEndpointsResponse:
        *message__msg_type = constants__e_msg_discovery_get_endpoints_resp;
        break;
    case OpcUaId_RegisterServerRequest:
        *message__msg_type = constants__e_msg_discovery_register_server_req;
        break;
    case OpcUaId_RegisterServerResponse:
        *message__msg_type = constants__e_msg_discovery_register_server_resp;
        break;
    case OpcUaId_RegisterServer2Request:
        *message__msg_type = constants__e_msg_discovery_register_server2_req;
        break;
    case OpcUaId_RegisterServer2Response:
        *message__msg_type = constants__e_msg_discovery_register_server2_resp;
        break;
    case OpcUaId_CreateSessionRequest:
        *message__msg_type = constants__e_msg_session_create_req;
        break;
    case OpcUaId_CreateSessionResponse:
        *message__msg_type = constants__e_msg_session_create_resp;
        break;
    case OpcUaId_ActivateSessionRequest:
        *message__msg_type = constants__e_msg_session_activate_req;
        break;
    case OpcUaId_ActivateSessionResponse:
        *message__msg_type = constants__e_msg_session_activate_resp;
        break;
    case OpcUaId_CloseSessionRequest:
        *message__msg_type = constants__e_msg_session_close_req;
        break;
    case OpcUaId_CloseSessionResponse:
        *message__msg_type = constants__e_msg_session_close_resp;
        break;
    case OpcUaId_CancelRequest:
        *message__msg_type = constants__e_msg_session_cancel_req;
        break;
    case OpcUaId_CancelResponse:
        *message__msg_type = constants__e_msg_session_cancel_resp;
        break;
    case OpcUaId_AddNodesRequest:
        *message__msg_type = constants__e_msg_node_add_nodes_req;
        break;
    case OpcUaId_AddNodesResponse:
        *message__msg_type = constants__e_msg_node_add_nodes_resp;
        break;
    case OpcUaId_AddReferencesRequest:
        *message__msg_type = constants__e_msg_node_add_references_req;
        break;
    case OpcUaId_AddReferencesResponse:
        *message__msg_type = constants__e_msg_node_add_references_resp;
        break;
    case OpcUaId_DeleteNodesRequest:
        *message__msg_type = constants__e_msg_node_delete_nodes_req;
        break;
    case OpcUaId_DeleteNodesResponse:
        *message__msg_type = constants__e_msg_node_delete_nodes_resp;
        break;
    case OpcUaId_DeleteReferencesRequest:
        *message__msg_type = constants__e_msg_node_delete_references_req;
        break;
    case OpcUaId_DeleteReferencesResponse:
        *message__msg_type = constants__e_msg_node_delete_references_resp;
        break;
    case OpcUaId_BrowseRequest:
        *message__msg_type = constants__e_msg_view_browse_req;
        break;
    case OpcUaId_BrowseResponse:
        *message__msg_type = constants__e_msg_view_browse_resp;
        break;
    case OpcUaId_BrowseNextRequest:
        *message__msg_type = constants__e_msg_view_browse_next_req;
        break;
    case OpcUaId_BrowseNextResponse:
        *message__msg_type = constants__e_msg_view_browse_next_resp;
        break;
    case OpcUaId_TranslateBrowsePathsToNodeIdsRequest:
        *message__msg_type = constants__e_msg_view_translate_browse_paths_to_node_ids_req;
        break;
    case OpcUaId_TranslateBrowsePathsToNodeIdsResponse:
        *message__msg_type = constants__e_msg_view_translate_browse_paths_to_node_ids_resp;
        break;
    case OpcUaId_RegisterNodesRequest:
        *message__msg_type = constants__e_msg_view_register_nodes_req;
        break;
    case OpcUaId_RegisterNodesResponse:
        *message__msg_type = constants__e_msg_view_register_nodes_resp;
        break;
    case OpcUaId_UnregisterNodesRequest:
        *message__msg_type = constants__e_msg_view_unregister_nodes_req;
        break;
    case OpcUaId_UnregisterNodesResponse:
        *message__msg_type = constants__e_msg_view_unregister_nodes_resp;
        break;
    case OpcUaId_QueryFirstRequest:
        *message__msg_type = constants__e_msg_query_first_req;
        break;
    case OpcUaId_QueryFirstResponse:
        *message__msg_type = constants__e_msg_query_first_resp;
        break;
    case OpcUaId_QueryNextRequest:
        *message__msg_type = constants__e_msg_query_next_req;
        break;
    case OpcUaId_QueryNextResponse:
        *message__msg_type = constants__e_msg_query_next_resp;
        break;
    case OpcUaId_ReadRequest:
        *message__msg_type = constants__e_msg_attribute_read_req;
        break;
    case OpcUaId_ReadResponse:
        *message__msg_type = constants__e_msg_attribute_read_resp;
        break;
    case OpcUaId_HistoryReadRequest:
        *message__msg_type = constants__e_msg_attribute_history_read_req;
        break;
    case OpcUaId_HistoryReadResponse:
        *message__msg_type = constants__e_msg_attribute_history_read_resp;
        break;
    case OpcUaId_WriteRequest:
        *message__msg_type = constants__e_msg_attribute_write_req;
        break;
    case OpcUaId_WriteResponse:
        *message__msg_type = constants__e_msg_attribute_write_resp;
        break;
    case OpcUaId_HistoryUpdateRequest:
        *message__msg_type = constants__e_msg_attribute_history_update_req;
        break;
    case OpcUaId_HistoryUpdateResponse:
        *message__msg_type = constants__e_msg_attribute_history_update_resp;
        break;
    case OpcUaId_CallRequest:
        *message__msg_type = constants__e_msg_method_call_req;
        break;
    case OpcUaId_CallResponse:
        *message__msg_type = constants__e_msg_method_call_resp;
        break;
    case OpcUaId_CreateMonitoredItemsRequest:
        *message__msg_type = constants__e_msg_monitored_items_create_req;
        break;
    case OpcUaId_CreateMonitoredItemsResponse:
        *message__msg_type = constants__e_msg_monitored_items_create_resp;
        break;
    case OpcUaId_ModifyMonitoredItemsRequest:
        *message__msg_type = constants__e_msg_monitored_items_modify_req;
        break;
    case OpcUaId_ModifyMonitoredItemsResponse:
        *message__msg_type = constants__e_msg_monitored_items_modify_resp;
        break;
    case OpcUaId_SetMonitoringModeRequest:
        *message__msg_type = constants__e_msg_monitored_items_set_monitoring_mode_req;
        break;
    case OpcUaId_SetMonitoringModeResponse:
        *message__msg_type = constants__e_msg_monitored_items_set_monitoring_mode_resp;
        break;
    case OpcUaId_SetTriggeringRequest:
        *message__msg_type = constants__e_msg_monitored_items_set_triggering_req;
        break;
    case OpcUaId_SetTriggeringResponse:
        *message__msg_type = constants__e_msg_monitored_items_set_triggering_resp;
        break;
    case OpcUaId_DeleteMonitoredItemsRequest:
        *message__msg_type = constants__e_msg_monitored_items_delete_req;
        break;
    case OpcUaId_DeleteMonitoredItemsResponse:
        *message__msg_type = constants__e_msg_monitored_items_delete_resp;
        break;
    case OpcUaId_CreateSubscriptionRequest:
        *message__msg_type = constants__e_msg_subscription_create_req;
        break;
    case OpcUaId_CreateSubscriptionResponse:
        *message__msg_type = constants__e_msg_subscription_create_resp;
        break;
    case OpcUaId_ModifySubscriptionRequest:
        *message__msg_type = constants__e_msg_subscription_modify_req;
        break;
    case OpcUaId_ModifySubscriptionResponse:
        *message__msg_type = constants__e_msg_subscription_modify_resp;
        break;
    case OpcUaId_SetPublishingModeRequest:
        *message__msg_type = constants__e_msg_subscription_set_publishing_mode_req;
        break;
    case OpcUaId_SetPublishingModeResponse:
        *message__msg_type = constants__e_msg_subscription_set_publishing_mode_resp;
        break;
    case OpcUaId_PublishRequest:
        *message__msg_type = constants__e_msg_subscription_publish_req;
        break;
    case OpcUaId_PublishResponse:
        *message__msg_type = constants__e_msg_subscription_publish_resp;
        break;
    case OpcUaId_RepublishRequest:
        *message__msg_type = constants__e_msg_subscription_republish_req;
        break;
    case OpcUaId_RepublishResponse:
        *message__msg_type = constants__e_msg_subscription_republish_resp;
        break;
    case OpcUaId_TransferSubscriptionsRequest:
        *message__msg_type = constants__e_msg_subscription_transfer_subscriptions_req;
        break;
    case OpcUaId_TransferSubscriptionsResponse:
        *message__msg_type = constants__e_msg_subscription_transfer_subscriptions_resp;
        break;
    case OpcUaId_DeleteSubscriptionsRequest:
        *message__msg_type = constants__e_msg_subscription_delete_subscriptions_req;
        break;
    case OpcUaId_DeleteSubscriptionsResponse:
        *message__msg_type = constants__e_msg_subscription_delete_subscriptions_resp;
        break;
    case OpcUaId_ServiceFault:
        *message__msg_type = constants__e_msg_service_fault_resp;
        break;
    default:
        *message__msg_type = constants__c_msg_type_indet;
    }
}

void util_status_code__B_to_C(constants__t_StatusCode_i bstatus, SOPC_StatusCode* status)
{
    switch (bstatus)
    {
    case constants__e_sc_ok:
        *status = SOPC_GoodGenericStatus;
        break;
    case constants__e_sc_bad_generic:
        *status = SOPC_BadStatusMask; // generic bad status
        break;
    case constants__e_sc_uncertain_generic:
        *status = SOPC_UncertainStatusMask; // generic uncertain status
        break;
    case constants__e_sc_bad_internal_error:
        *status = OpcUa_BadInternalError;
        break;
    case constants__e_sc_bad_secure_channel_id_invalid:
        *status = OpcUa_BadSecureChannelIdInvalid;
        break;
    case constants__e_sc_bad_secure_channel_closed:
        *status = OpcUa_BadSecureChannelClosed;
        break;
    case constants__e_sc_bad_connection_closed:
        *status = OpcUa_BadConnectionClosed;
        break;
    case constants__e_sc_bad_invalid_state:
        *status = OpcUa_BadInvalidState;
        break;
    case constants__e_sc_bad_session_id_invalid:
        *status = OpcUa_BadSessionIdInvalid;
        break;
    case constants__e_sc_bad_session_closed:
        *status = OpcUa_BadSessionClosed;
        break;
    case constants__e_sc_bad_session_not_activated:
        *status = OpcUa_BadSessionNotActivated;
        break;
    case constants__e_sc_bad_identity_token_invalid:
        *status = OpcUa_BadIdentityTokenInvalid;
        break;
    case constants__e_sc_bad_encoding_error:
        *status = OpcUa_BadEncodingError;
        break;
    case constants__e_sc_bad_decoding_error:
        *status = OpcUa_BadDecodingError;
        break;
    case constants__e_sc_bad_invalid_argument:
        *status = OpcUa_BadInvalidArgument;
        break;
    case constants__e_sc_bad_unexpected_error:
        *status = OpcUa_BadUnexpectedError;
        break;
    case constants__e_sc_bad_out_of_memory:
        *status = OpcUa_BadOutOfMemory;
        break;
    case constants__e_sc_bad_nothing_to_do:
        *status = OpcUa_BadNothingToDo;
        break;
    case constants__e_sc_bad_too_many_ops:
        *status = OpcUa_BadTooManyOperations;
        break;
    case constants__e_sc_bad_max_age_invalid:
        *status = OpcUa_BadMaxAgeInvalid;
        break;
    case constants__e_sc_bad_timestamps_to_return_invalid:
        *status = OpcUa_BadTimestampsToReturnInvalid;
        break;
    case constants__e_sc_bad_node_id_unknown:
        *status = OpcUa_BadNodeIdUnknown;
        break;
    case constants__e_sc_bad_node_id_invalid:
        *status = OpcUa_BadNodeIdInvalid;
        break;
    case constants__e_sc_bad_view_id_unknown:
        *status = OpcUa_BadViewIdUnknown;
        break;
    case constants__e_sc_bad_attribute_id_invalid:
        *status = OpcUa_BadAttributeIdInvalid;
        break;
    case constants__e_sc_bad_browse_direction_invalid:
        *status = OpcUa_BadBrowseDirectionInvalid;
        break;
    case constants__e_sc_bad_service_unsupported:
        *status = OpcUa_BadServiceUnsupported;
        break;
    case constants__e_sc_bad_write_not_supported:
        *status = OpcUa_BadWriteNotSupported;
        break;
    default:
        *status = OpcUa_BadInternalError;
    }
}

SOPC_ReturnStatus util_status_code__B_to_return_status_C(constants__t_StatusCode_i bstatus)
{
    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
    switch (bstatus)
    {
    case constants__e_sc_ok:
        result = SOPC_STATUS_OK;
        break;
    case constants__e_sc_bad_generic:
        result = SOPC_STATUS_NOK;
        break;
    case constants__e_sc_uncertain_generic:
        result = SOPC_STATUS_NOK;
        break;
    case constants__e_sc_bad_internal_error:
        result = SOPC_STATUS_NOK;
        break;
    case constants__e_sc_bad_secure_channel_id_invalid:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case constants__e_sc_bad_secure_channel_closed:
        result = SOPC_STATUS_CLOSED;
        break;
    case constants__e_sc_bad_connection_closed:
        result = SOPC_STATUS_CLOSED;
        break;
    case constants__e_sc_bad_invalid_state:
        result = SOPC_STATUS_INVALID_STATE;
        break;
    case constants__e_sc_bad_session_id_invalid:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case constants__e_sc_bad_session_closed:
        result = SOPC_STATUS_INVALID_STATE;
        break;
    case constants__e_sc_bad_session_not_activated:
        result = SOPC_STATUS_INVALID_STATE;
        break;
    case constants__e_sc_bad_identity_token_invalid:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case constants__e_sc_bad_encoding_error:
        result = SOPC_STATUS_ENCODING_ERROR;
        break;
    case constants__e_sc_bad_decoding_error:
        result = SOPC_STATUS_ENCODING_ERROR;
        break;
    case constants__e_sc_bad_invalid_argument:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case constants__e_sc_bad_out_of_memory:
        result = SOPC_STATUS_OUT_OF_MEMORY;
        break;
    case constants__e_sc_bad_too_many_ops:
        result = SOPC_STATUS_OUT_OF_MEMORY;
        break;
    default:
        result = SOPC_STATUS_NOK;
    }
    return result;
}

void util_status_code__C_to_B(SOPC_StatusCode status, constants__t_StatusCode_i* bstatus)
{
    switch (status)
    {
    case OpcUa_BadInternalError:
        *bstatus = constants__e_sc_bad_internal_error;
        break;
    case OpcUa_BadSecureChannelClosed:
        *bstatus = constants__e_sc_bad_secure_channel_closed;
        break;
    case OpcUa_BadSecureChannelIdInvalid:
        *bstatus = constants__e_sc_bad_secure_channel_id_invalid;
        break;
    case OpcUa_BadConnectionClosed:
        *bstatus = constants__e_sc_bad_connection_closed;
        break;
    case OpcUa_BadInvalidState:
        *bstatus = constants__e_sc_bad_invalid_state;
        break;
    case OpcUa_BadSessionIdInvalid:
        *bstatus = constants__e_sc_bad_session_id_invalid;
        break;
    case OpcUa_BadSessionClosed:
        *bstatus = constants__e_sc_bad_session_closed;
        break;
    case OpcUa_BadSessionNotActivated:
        *bstatus = constants__e_sc_bad_session_not_activated;
        break;
    case OpcUa_BadIdentityTokenInvalid:
        *bstatus = constants__e_sc_bad_identity_token_invalid;
        break;
    case OpcUa_BadEncodingError:
        *bstatus = constants__e_sc_bad_encoding_error;
        break;
    case OpcUa_BadDecodingError:
        *bstatus = constants__e_sc_bad_decoding_error;
        break;
    case OpcUa_BadInvalidArgument:
        *bstatus = constants__e_sc_bad_invalid_argument;
        break;
    case OpcUa_BadUnexpectedError:
        *bstatus = constants__e_sc_bad_unexpected_error;
        break;
    case OpcUa_BadOutOfMemory:
        *bstatus = constants__e_sc_bad_out_of_memory;
        break;
    case OpcUa_BadNothingToDo:
        *bstatus = constants__e_sc_bad_nothing_to_do;
        break;
    case OpcUa_BadTooManyOperations:
        *bstatus = constants__e_sc_bad_too_many_ops;
        break;
    case OpcUa_BadMaxAgeInvalid:
        *bstatus = constants__e_sc_bad_max_age_invalid;
        break;
    case OpcUa_BadTimestampsToReturnInvalid:
        *bstatus = constants__e_sc_bad_timestamps_to_return_invalid;
        break;
    case OpcUa_BadNodeIdUnknown:
        *bstatus = constants__e_sc_bad_node_id_unknown;
        break;
    case OpcUa_BadNodeIdInvalid:
        *bstatus = constants__e_sc_bad_node_id_invalid;
        break;
    case OpcUa_BadViewIdUnknown:
        *bstatus = constants__e_sc_bad_view_id_unknown;
        break;
    case OpcUa_BadAttributeIdInvalid:
        *bstatus = constants__e_sc_bad_attribute_id_invalid;
        break;
    case OpcUa_BadBrowseDirectionInvalid:
        *bstatus = constants__e_sc_bad_browse_direction_invalid;
        break;
    case OpcUa_BadServiceUnsupported:
        *bstatus = constants__e_sc_bad_service_unsupported;
        break;
    case OpcUa_BadWriteNotSupported:
        *bstatus = constants__e_sc_bad_write_not_supported;
        break;
    default:
        if ((status & SOPC_GoodStatusOppositeMask) == 0)
        {
            *bstatus = constants__e_sc_ok;
        }
        else if ((status & SOPC_BadStatusMask) != 0)
        {
            *bstatus = constants__e_sc_bad_generic;
        }
        else if ((status & SOPC_UncertainStatusMask) != 0)
        {
            *bstatus = constants__e_sc_uncertain_generic;
        }
        else
        {
            // Not identified status code
            *bstatus = constants__c_StatusCode_indet;
        }
    }
}

bool util_channel__SecurityPolicy_C_to_B(const char* uri, constants__t_SecurityPolicy* secpol)
{
    if (NULL == uri || NULL == secpol)
        return false;

    if (strncmp(uri, SOPC_SecurityPolicy_None_URI, strlen(SOPC_SecurityPolicy_None_URI)) == 0)
    {
        *secpol = constants__e_secpol_None;
        return true;
    }
    if (strncmp(uri, SOPC_SecurityPolicy_Basic256_URI, strlen(SOPC_SecurityPolicy_Basic256_URI)) == 0)
    {
        *secpol = constants__e_secpol_B256;
        return true;
    }
    if (strncmp(uri, SOPC_SecurityPolicy_Basic256Sha256_URI, strlen(SOPC_SecurityPolicy_Basic256Sha256_URI)) == 0)
    {
        *secpol = constants__e_secpol_B256S256;
        return true;
    }

    return false;
}

bool util_BrowseDirection__B_to_C(constants__t_BrowseDirection_i bdir, OpcUa_BrowseDirection* cdir)
{
    if (NULL == cdir)
        return false;

    switch (bdir)
    {
    case constants__e_bd_forward:
        *cdir = OpcUa_BrowseDirection_Forward;
        break;
    case constants__e_bd_inverse:
        *cdir = OpcUa_BrowseDirection_Inverse;
        break;
    case constants__e_bd_both:
        *cdir = OpcUa_BrowseDirection_Both;
        break;
    case constants__e_bd_indet:
    default:
        return false;
    }

    return true;
}

bool util_BrowseDirection__C_to_B(OpcUa_BrowseDirection cdir, constants__t_BrowseDirection_i* bdir)
{
    if (NULL == bdir)
        return false;

    switch (cdir)
    {
    case OpcUa_BrowseDirection_Forward:
        *bdir = constants__e_bd_forward;
        break;
    case OpcUa_BrowseDirection_Inverse:
        *bdir = constants__e_bd_inverse;
        break;
    case OpcUa_BrowseDirection_Both:
        *bdir = constants__e_bd_both;
        break;
    default:
        return false;
    }

    return true;
}

bool util_NodeClass__B_to_C(constants__t_NodeClass_i bncl, OpcUa_NodeClass* cncl)
{
    if (NULL == cncl)
        return false;

    switch (bncl)
    {
    case constants__e_ncl_Object:
        *cncl = OpcUa_NodeClass_Object;
        break;
    case constants__e_ncl_Variable:
        *cncl = OpcUa_NodeClass_Variable;
        break;
    case constants__e_ncl_Method:
        *cncl = OpcUa_NodeClass_Method;
        break;
    case constants__e_ncl_ObjectType:
        *cncl = OpcUa_NodeClass_ObjectType;
        break;
    case constants__e_ncl_VariableType:
        *cncl = OpcUa_NodeClass_VariableType;
        break;
    case constants__e_ncl_ReferenceType:
        *cncl = OpcUa_NodeClass_ReferenceType;
        break;
    case constants__e_ncl_DataType:
        *cncl = OpcUa_NodeClass_DataType;
        break;
    case constants__e_ncl_View:
        *cncl = OpcUa_NodeClass_View;
        break;
    case constants__c_NodeClass_indet:
    default:
        return false;
    }

    return true;
}

bool util_NodeClass__C_to_B(OpcUa_NodeClass cncl, constants__t_NodeClass_i* bncl)
{
    if (NULL == bncl)
        return false;

    switch (cncl)
    {
    case OpcUa_NodeClass_Object:
        *bncl = constants__e_ncl_Object;
        break;
    case OpcUa_NodeClass_Variable:
        *bncl = constants__e_ncl_Variable;
        break;
    case OpcUa_NodeClass_Method:
        *bncl = constants__e_ncl_Method;
        break;
    case OpcUa_NodeClass_ObjectType:
        *bncl = constants__e_ncl_ObjectType;
        break;
    case OpcUa_NodeClass_VariableType:
        *bncl = constants__e_ncl_VariableType;
        break;
    case OpcUa_NodeClass_ReferenceType:
        *bncl = constants__e_ncl_ReferenceType;
        break;
    case OpcUa_NodeClass_DataType:
        *bncl = constants__e_ncl_DataType;
        break;
    case OpcUa_NodeClass_View:
        *bncl = constants__e_ncl_View;
        break;
    default:
        return false;
    }

    return true;
}

bool util_TimestampsToReturn__B_to_C(constants__t_TimestampsToReturn_i bttr, OpcUa_TimestampsToReturn* pcttr)
{
    bool status = NULL != pcttr && constants__c_TimestampsToReturn_indet == bttr;

    if (status)
    {
        switch (bttr)
        {
        case constants__e_ttr_source:
            *pcttr = OpcUa_TimestampsToReturn_Source;
            break;
        case constants__e_ttr_server:
            *pcttr = OpcUa_TimestampsToReturn_Server;
            break;
        case constants__e_ttr_both:
            *pcttr = OpcUa_TimestampsToReturn_Both;
            break;
        case constants__e_ttr_neither:
            *pcttr = OpcUa_TimestampsToReturn_Neither;
            break;
        case constants__c_TimestampsToReturn_indet:
        default:
            status = false;
            break;
        }
    }

    return status;
}

bool util_TimestampsToReturn__C_to_B(OpcUa_TimestampsToReturn cttr, constants__t_TimestampsToReturn_i* pbttr)
{
    bool status = NULL != pbttr;

    if (NULL != pbttr)
    {
        switch (cttr)
        {
        case OpcUa_TimestampsToReturn_Source:
            *pbttr = constants__e_ttr_source;
            break;
        case OpcUa_TimestampsToReturn_Server:
            *pbttr = constants__e_ttr_server;
            break;
        case OpcUa_TimestampsToReturn_Both:
            *pbttr = constants__e_ttr_both;
            break;
        case OpcUa_TimestampsToReturn_Neither:
            *pbttr = constants__e_ttr_neither;
            break;
        default:
            status = false;
            break;
        }
    }

    return status;
}

bool util_AttributeId__B_to_C(constants__t_AttributeId_i baid, uint32_t* pcaid)
{
    bool status = NULL != pcaid;

    if (status)
    {
        switch (baid)
        {
        case constants__e_aid_NodeId:
            *pcaid = e_aid_NodeId;
            break;
        case constants__e_aid_NodeClass:
            *pcaid = e_aid_NodeClass;
            break;
        case constants__e_aid_BrowseName:
            *pcaid = e_aid_BrowseName;
            break;
        case constants__e_aid_DisplayName:
            *pcaid = e_aid_DisplayName;
            break;
        case constants__e_aid_Value:
            *pcaid = e_aid_Value;
            break;
        case constants__e_aid_AccessLevel:
            *pcaid = e_aid_AccessLevel;
            break;
        case constants__c_AttributeId_indet:
        default:
            status = false;
            break;
        }
    }

    return status;
}

bool util_AttributeId__C_to_B(uint32_t caid, constants__t_AttributeId_i* pbaid)
{
    bool status = NULL != pbaid;

    if (status)
    {
        switch (caid)
        {
        case e_aid_NodeId:
            *pbaid = constants__e_aid_NodeId;
            break;
        case e_aid_NodeClass:
            *pbaid = constants__e_aid_NodeClass;
            break;
        case e_aid_BrowseName:
            *pbaid = constants__e_aid_BrowseName;
            break;
        case e_aid_DisplayName:
            *pbaid = constants__e_aid_DisplayName;
            break;
        case e_aid_Value:
            *pbaid = constants__e_aid_Value;
            break;
        case e_aid_AccessLevel:
            *pbaid = constants__e_aid_AccessLevel;
            break;
        default:
            status = false;
            break;
        }
    }

    return status;
}
