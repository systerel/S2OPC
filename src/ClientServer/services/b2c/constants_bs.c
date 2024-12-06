/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** \file
 *
 * Implements the base machine for the constants
 */

#include "constants_bs.h"
#include "b2c.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_date_time.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"

static SOPC_NodeId ByteString_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_ByteString);
static SOPC_NodeId Byte_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Byte);
static SOPC_NodeId Null_Type = SOPC_NS0_NUMERIC_NODEID(0);
static SOPC_NodeId HasComponent_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_HasComponent);
static SOPC_NodeId HasProperty_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_HasProperty);
static SOPC_NodeId HierarchicalReferences_Type = SOPC_NS0_NUMERIC_NODEID(OpcUaId_HierarchicalReferences);
static SOPC_NodeId Server_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server);
static SOPC_NodeId BaseEventType_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_BaseEventType);
static SOPC_NodeId Server_ServerCapabilities_RoleSet_NodeId =
    SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server_ServerCapabilities_RoleSet);
static SOPC_NodeId RoleType_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_RoleType);
static SOPC_NodeId Server_NamespaceArray_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server_NamespaceArray);
static SOPC_NodeId Server_Namespaces_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_Server_Namespaces);
static SOPC_NodeId NamespaceMetadataType_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_NamespaceMetadataType);
static SOPC_NodeId RolePermissionType_NodeId = SOPC_NS0_NUMERIC_NODEID(OpcUaId_RolePermissionType);

const constants_bs__t_NodeId_i constants_bs__c_ByteString_Type_NodeId = &ByteString_Type;
const constants_bs__t_NodeId_i constants_bs__c_Byte_Type_NodeId = &Byte_Type;
const constants_bs__t_NodeId_i constants_bs__c_Null_Type_NodeId = &Null_Type;
const constants_bs__t_NodeId_i constants_bs__c_HasComponentType_NodeId = &HasComponent_Type;
const constants_bs__t_NodeId_i constants_bs__c_HasPropertyType_NodeId = &HasProperty_Type;
const constants_bs__t_NodeId_i constants_bs__c_HierarchicalReferences_Type_NodeId = &HierarchicalReferences_Type;
const constants_bs__t_NodeId_i constants_bs__c_Server_NodeId = &Server_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_BaseEventType_NodeId = &BaseEventType_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_Server_ServerCapabilities_RoleSet_NodeId =
    &Server_ServerCapabilities_RoleSet_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_RoleType_NodeId = &RoleType_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_NamespaceMetadataType_NodeId = &NamespaceMetadataType_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_Server_NamespaceArray_NodeId = &Server_NamespaceArray_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_Server_Namespaces_NodeId = &Server_Namespaces_NodeId;
const constants_bs__t_NodeId_i constants_bs__c_RolePermissionType_NodeId = &RolePermissionType_NodeId;

#define SOPC_NS0_QUALIFIED_NAME(name)                                        \
    {                                                                        \
        OPCUA_NAMESPACE_INDEX, { sizeof(name) - 1, true, (SOPC_Byte*) name } \
    }
static SOPC_QualifiedName Identities_QualifiedName = SOPC_NS0_QUALIFIED_NAME("Identities");
const constants_bs__t_QualifiedName_i constants_bs__c_Identities_QualifiedName = &Identities_QualifiedName;
static SOPC_QualifiedName DefaultRolePermissions_QualifiedName = SOPC_NS0_QUALIFIED_NAME("DefaultRolePermissions");
const constants_bs__t_QualifiedName_i constants_bs__c_DefaultRolePermissions_QualifiedName =
    &DefaultRolePermissions_QualifiedName;
static SOPC_QualifiedName NamespaceUri_QualifiedName = SOPC_NS0_QUALIFIED_NAME("NamespaceUri");
const constants_bs__t_QualifiedName_i constants_bs__c_NamespaceUri_QualifiedName = &NamespaceUri_QualifiedName;

static char* EmptyLocaleIds[] = {NULL};
constants_bs__t_LocaleIds_i constants_bs__c_LocaleIds_empty = EmptyLocaleIds;

void constants_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void constants_bs__getall_conv_ExpandedNodeId_NodeId(const constants_bs__t_ExpandedNodeId_i constants_bs__p_expnid,
                                                     t_bool* const constants_bs__p_local_server,
                                                     constants_bs__t_NodeId_i* const constants_bs__p_nid)
{
    *constants_bs__p_nid = constants_bs__c_ExpandedNodeId_indet;
    *constants_bs__p_local_server = false;
    if (constants_bs__p_expnid->ServerIndex == 0)
    {
        // TODO: namespaceUri.Length > 0 does not really mean it is an external node but we do not manage it:
        //       if server index indicates it is local node (index == 0) and URI is defined the URI
        //       can be either valid in local server or invalid (another degraded case).
        if (constants_bs__p_expnid->NamespaceUri.Length <= 0)
        {
            *constants_bs__p_local_server = true;
            /* Reminder: This is a borrow */
            *constants_bs__p_nid = &constants_bs__p_expnid->NodeId;
        }
        else
        {
            // Should be a local namespace URI but we do not manage it
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Conversion of Namespace URI %s from ExpandedNodeId not managed => considered as external server node.",
                SOPC_String_GetRawCString(&constants_bs__p_expnid->NamespaceUri));
        }
    }
}

void constants_bs__is_ClientNodeManagementActive(t_bool* const constants_bs__bres)
{
    *constants_bs__bres = S2OPC_NODE_MANAGEMENT;
}

void constants_bs__is_EventNotifier_SubscribeToEvents(const constants_bs__t_Byte constants_bs__p_eventNotifierByte,
                                                      t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (0 != (OpcUa_EventNotifierType_SubscribeToEvents & constants_bs__p_eventNotifierByte));
}

void constants_bs__is_t_access_level_currentRead(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                 t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_CurrentRead) != 0;
}

void constants_bs__is_t_access_level_currentWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                  t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_CurrentWrite) != 0;
}

void constants_bs__is_t_access_level_statusWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                 t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_StatusWrite) != 0;
}

void constants_bs__is_t_access_level_timestampWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                    t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_TimestampWrite) != 0;
}

void constants_bs__get_card_t_channel(t_entier4* const constants_bs__p_card_channel)
{
    *constants_bs__p_card_channel = constants_bs__t_channel_i_max;
}

void constants_bs__get_card_t_channel_config_idx(t_entier4* const constants_bs__p_card_channel_config)
{
    *constants_bs__p_card_channel_config = constants_bs__t_channel_config_idx_i_max;
}

void constants_bs__get_card_t_session(t_entier4* const constants_bs__p_card_session)
{
    *constants_bs__p_card_session = constants_bs__t_session_i_max;
}

void constants_bs__get_card_t_subscription(t_entier4* const constants_bs__p_card_subscription)
{
    *constants_bs__p_card_subscription = constants_bs__t_subscription_i_max;
}

void constants_bs__get_cast_t_channel(const t_entier4 constants_bs__p_ind,
                                      constants_bs__t_channel_i* const constants_bs__p_channel)
{
    *constants_bs__p_channel = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__get_cast_t_NamespaceIdx(const t_entier4 constants_bs__p_ind,
                                           constants_bs__t_NamespaceIdx* const constants_bs__p_nsIndex)
{
    *constants_bs__p_nsIndex = (uint16_t) constants_bs__p_ind;
}

void constants_bs__get_cast_t_channel_config_idx(const t_entier4 constants_bs__p_ind,
                                                 constants_bs__t_channel_config_idx_i* const constants_bs__p_config)
{
    *constants_bs__p_config = (SOPC_SecureChannelConfigIdx) constants_bs__p_ind;
}

void constants_bs__get_cast_t_session(const t_entier4 constants_bs__p_ind,
                                      constants_bs__t_session_i* const constants_bs__p_session)
{
    *constants_bs__p_session = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__get_cast_t_subscription(const t_entier4 constants_bs__p_ind,
                                           constants_bs__t_subscription_i* const constants_bs__p_subscription)
{
    *constants_bs__p_subscription = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__is_t_channel(const constants_bs__t_channel_i constants_bs__p_channel,
                                t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res = (constants_bs__p_channel > 0 && constants_bs__p_channel <= constants_bs__t_channel_i_max);
}

void constants_bs__is_t_channel_config_idx(const constants_bs__t_channel_config_idx_i constants_bs__p_config_idx,
                                           t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res =
        (constants_bs__p_config_idx > 0 && constants_bs__p_config_idx <= constants_bs__t_channel_config_idx_i_max);
}

void constants_bs__is_t_endpoint_config_idx(
    const constants_bs__t_endpoint_config_idx_i constants_bs__p_endpoint_config_idx,
    t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res = (constants_bs__p_endpoint_config_idx > 0 &&
                            constants_bs__p_endpoint_config_idx <= constants_bs__t_endpoint_config_idx_i_max);
}

void constants_bs__get_cast_t_BrowsePath(const t_entier4 constants_bs__p_ind,
                                         constants_bs__t_BrowsePath_i* const constants_bs__p_browsePath)
{
    *constants_bs__p_browsePath = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__get_cast_t_CallMethod(const t_entier4 constants_bs__p_ind,
                                         constants_bs__t_CallMethod_i* const constants_bs__p_callMethod)
{
    *constants_bs__p_callMethod = constants_bs__p_ind;
}

void constants_bs__is_QualifiedNames_Empty(const constants_bs__t_QualifiedName_i constants_bs__name,
                                           t_bool* const constants_bs__p_bool)
{
    if (NULL == constants_bs__name)
    {
        *constants_bs__p_bool = true;
    }
    else
    {
        *constants_bs__p_bool = (0 >= constants_bs__name->Name.Length);
    }
}

void constants_bs__is_QualifiedNames_Equal(const constants_bs__t_QualifiedName_i constants_bs__name1,
                                           const constants_bs__t_QualifiedName_i constants_bs__name2,
                                           t_bool* const constants_bs__p_bool)
{
    if (NULL == constants_bs__name1 || NULL == constants_bs__name2)
    {
        *constants_bs__p_bool = (constants_bs__name1 == constants_bs__name2);
    }
    else
    {
        int32_t comparison = 0;
        const SOPC_ReturnStatus retVal =
            SOPC_QualifiedName_Compare(constants_bs__name1, constants_bs__name2, &comparison);
        *constants_bs__p_bool = (SOPC_STATUS_OK == retVal && 0 == comparison);
    }
}

void constants_bs__is_Timestamps_Null(const constants_bs__t_Timestamp constants_bs__p_ts,
                                      t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__c_Timestamp_null.timestamp == constants_bs__p_ts.timestamp &&
                           constants_bs__c_Timestamp_null.picoSeconds == constants_bs__p_ts.picoSeconds);
}

void constants_bs__free_ExpandedNodeId(const constants_bs__t_ExpandedNodeId_i constants_bs__p_in)
{
    if (NULL != constants_bs__p_in)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_ExpandedNodeId_Clear(constants_bs__p_in);
        SOPC_Free(constants_bs__p_in);
        SOPC_GCC_DIAGNOSTIC_RESTORE
    }
}

void constants_bs__free_roles(const constants_bs__t_sessionRoles_i constants_bs__p_in)
{
    // Delete the references to the nodeIds in the LinkedList, and delete the LinkedList
    SOPC_SLinkedList_Delete(constants_bs__p_in);
}

void constants_bs__free_LocaleIds(const constants_bs__t_LocaleIds_i constants_bs__p_in)
{
    SOPC_ASSERT(constants_bs__c_LocaleIds_indet != constants_bs__p_in);
    uint32_t idx = 0;
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    char* cstring = constants_bs__p_in[idx];
    while (NULL != cstring)
    {
        SOPC_Free(cstring);
        idx++;
        cstring = constants_bs__p_in[idx];
    }
    SOPC_GCC_DIAGNOSTIC_RESTORE
    SOPC_Free(constants_bs__p_in);
}

void constants_bs__get_CurrentTimestamp(constants_bs__t_Timestamp* const constants_bs__p_currentTs)
{
    constants_bs__p_currentTs->picoSeconds = 0;
    constants_bs__p_currentTs->timestamp = SOPC_Time_GetCurrentTimeUTC();
}

void constants_bs__get_SupportedLocales(const constants_bs__t_endpoint_config_idx_i constants_bs__p_in,
                                        constants_bs__t_LocaleIds_i* const constants_bs__p_localeIds)
{
    *constants_bs__p_localeIds = constants_bs__c_LocaleIds_empty;
    SOPC_Endpoint_Config* ep = SOPC_ToolkitServer_GetEndpointConfig(constants_bs__p_in);
    if (NULL != ep && ep->serverConfigPtr->localeIds != NULL)
    {
        *constants_bs__p_localeIds = ep->serverConfigPtr->localeIds;
    }
}

void constants_bs__get_copy_ExpandedNodeId(const constants_bs__t_ExpandedNodeId_i constants_bs__p_in,
                                           t_bool* const constants_bs__p_alloc,
                                           constants_bs__t_ExpandedNodeId_i* const constants_bs__p_out)
{
    *constants_bs__p_alloc = false;
    *constants_bs__p_out = SOPC_Calloc(1, sizeof(**constants_bs__p_out));
    if (NULL != *constants_bs__p_out)
    {
        SOPC_ExpandedNodeId_Initialize(*constants_bs__p_out);
        SOPC_ReturnStatus status = SOPC_ExpandedNodeId_Copy(*constants_bs__p_out, constants_bs__p_in);
        if (SOPC_STATUS_OK == status)
        {
            *constants_bs__p_alloc = true;
        }
        else
        {
            SOPC_Free(*constants_bs__p_out);
        }
    }
}

void constants_bs__get_NodeId_NamespaceIndex(const constants_bs__t_NodeId_i constants_bs__p_nodeId,
                                             constants_bs__t_NamespaceIdx* const constants_bs__p_idx)
{
    SOPC_ASSERT(NULL != constants_bs__p_nodeId);
    *constants_bs__p_idx = constants_bs__p_nodeId->Namespace;
}
