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

#ifndef UTIL_B2C_H_
#define UTIL_B2C_H_

#include <stdbool.h>

#include "b2c.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_types.h"
#include "sopc_user_manager.h"

void util_message__get_encodeable_type(const constants__t_msg_type_i message__msg_type,
                                       SOPC_EncodeableType** reqEncType,
                                       SOPC_EncodeableType** respEncType,
                                       t_bool* isRequest);

void util_message__get_message_type(SOPC_EncodeableType* encType, constants__t_msg_type_i* message__msg_type);

void util_message__copy_resp_header_into_msg(const constants__t_msg_header_i header, const constants__t_msg_i msg);

void util_status_code__B_to_C(constants_statuscodes_bs__t_StatusCode_i bstatus, SOPC_StatusCode* status);

void util_status_code__C_to_B(SOPC_StatusCode status, constants_statuscodes_bs__t_StatusCode_i* bstatus);

SOPC_ReturnStatus util_status_code__B_to_return_status_C(constants_statuscodes_bs__t_StatusCode_i bstatus);

constants_statuscodes_bs__t_StatusCode_i util_return_status__C_to_status_code_B(SOPC_ReturnStatus status);

bool util_channel__SecurityPolicy_C_to_B(const char* uri, constants__t_SecurityPolicy* secpol);

static SOPC_STRONG_INLINE const char* util_channel__SecurityPolicy_B_to_C(constants__t_SecurityPolicy secpol)
{
    switch (secpol)
    {
    case constants__e_secpol_None:
        return SOPC_SecurityPolicy_None_URI;
    case constants__e_secpol_B256:
        return SOPC_SecurityPolicy_Basic256_URI;
    case constants__e_secpol_B256S256:
        return SOPC_SecurityPolicy_Basic256Sha256_URI;
    case constants__e_secpol_Aes128Sha256RsaOaep:
        return SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI;
    case constants__e_secpol_Aes256Sha256RsaPss:
        return SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI;
    default:
        SOPC_ASSERT(false && "Invalid security policy");
        return NULL;
    }
}

static SOPC_STRONG_INLINE constants__t_BrowseDirection_i util_BrowseDirection__C_to_B(OpcUa_BrowseDirection cdir)
{
    switch (cdir)
    {
    case OpcUa_BrowseDirection_Forward:
        return constants__e_bd_forward;
    case OpcUa_BrowseDirection_Inverse:
        return constants__e_bd_inverse;
    case OpcUa_BrowseDirection_Both:
        return constants__e_bd_both;
    default:
        return constants__e_bd_indet;
    }
}

static SOPC_STRONG_INLINE OpcUa_BrowseDirection util_BrowseDirection__B_to_C(constants__t_BrowseDirection_i bdir)
{
    switch (bdir)
    {
    case constants__e_bd_forward:
        return OpcUa_BrowseDirection_Forward;
    case constants__e_bd_inverse:
        return OpcUa_BrowseDirection_Inverse;
    case constants__e_bd_both:
        return OpcUa_BrowseDirection_Both;
    default:
        SOPC_ASSERT(OpcUa_BrowseDirection_Both + 1 != OpcUa_BrowseDirection_Forward);
        SOPC_ASSERT(OpcUa_BrowseDirection_Both + 1 != OpcUa_BrowseDirection_Inverse);
        return OpcUa_BrowseDirection_Both + 1;
    }
}

/* Returns true or false upon failure (c_NodeClass_indet or invalid cncl) */
static SOPC_STRONG_INLINE bool util_NodeClass__B_to_C(constants__t_NodeClass_i bncl, OpcUa_NodeClass* cncl)
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

/* Returns true or false upon failure (invalid bncl) */
static SOPC_STRONG_INLINE bool util_NodeClass__C_to_B(OpcUa_NodeClass cncl, constants__t_NodeClass_i* bncl)
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

/* Returns true or false upon failure (c_TimestampsToReturn_indet or invalid pcttr) */
static SOPC_STRONG_INLINE bool util_TimestampsToReturn__B_to_C(constants__t_TimestampsToReturn_i bttr,
                                                               OpcUa_TimestampsToReturn* pcttr)
{
    if (pcttr == NULL || bttr == constants__c_TimestampsToReturn_indet)
    {
        return false;
    }

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
    default:
        return false;
    }

    return true;
}

/* Returns B enum value corresponding to C enum value of timestamps to return */
static SOPC_STRONG_INLINE constants__t_TimestampsToReturn_i
util_TimestampsToReturn__C_to_B(OpcUa_TimestampsToReturn cttr)
{
    constants__t_TimestampsToReturn_i result = constants__c_TimestampsToReturn_indet;

    switch (cttr)
    {
    case OpcUa_TimestampsToReturn_Source:
        result = constants__e_ttr_source;
        break;
    case OpcUa_TimestampsToReturn_Server:
        result = constants__e_ttr_server;
        break;
    case OpcUa_TimestampsToReturn_Both:
        result = constants__e_ttr_both;
        break;
    case OpcUa_TimestampsToReturn_Neither:
        result = constants__e_ttr_neither;
        break;
    default:
        result = constants__c_TimestampsToReturn_indet;
        break;
    }

    return result;
}

/* Returns a "valid" enum value in constants__t_AttributeId_i, indet value is used if not recognised integer */
static SOPC_STRONG_INLINE constants__t_AttributeId_i util_AttributeId__C_to_B(uint32_t caid)
{
    switch (caid)
    {
    case constants__e_aid_NodeId:
    case constants__e_aid_NodeClass:
    case constants__e_aid_BrowseName:
    case constants__e_aid_DisplayName:
    case constants__e_aid_Description:
    case constants__e_aid_WriteMask:
    case constants__e_aid_UserWriteMask:
    case constants__e_aid_IsAbstract:
    case constants__e_aid_Symmetric:
    case constants__e_aid_InverseName:
    case constants__e_aid_ContainsNoLoops:
    case constants__e_aid_EventNotifier:
    case constants__e_aid_Value:
    case constants__e_aid_DataType:
    case constants__e_aid_ValueRank:
    case constants__e_aid_ArrayDimensions:
    case constants__e_aid_AccessLevel:
    case constants__e_aid_UserAccessLevel:
    case constants__e_aid_MinimumSamplingInterval:
    case constants__e_aid_Historizing:
    case constants__e_aid_Executable:
    case constants__e_aid_UserExecutable:
    case constants__e_aid_DataTypeDefinition:
    case constants__e_aid_RolePermissions:
    case constants__e_aid_UserRolePermissions:
    case constants__e_aid_AccessRestrictions:
    case constants__e_aid_AccessLevelEx:
        return (constants__t_AttributeId_i) caid;
    default:
        return constants__c_AttributeId_indet;
        break;
    }
}

/* Raise exception upon failure (invalid B operation type or invalid pointer) */
static SOPC_STRONG_INLINE void util_operation_type__B_to_C(constants__t_operation_type_i boptype,
                                                           SOPC_UserAuthorization_OperationType* pcoptype)
{
    SOPC_ASSERT(NULL != pcoptype);

    switch (boptype)
    {
    case constants__e_operation_type_read:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_READ;
        break;
    case constants__e_operation_type_write:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_WRITE;
        break;
    case constants__e_operation_type_executable:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE;
        break;
    case constants__e_operation_type_addnode:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_ADDNODE;
        break;
    case constants__e_operation_type_receive_events:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_RECEIVE_EVENTS;
        break;
    case constants__e_operation_type_deletenode:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_DELETENODE;
        break;
    case constants__e_operation_type_historyread:
        *pcoptype = SOPC_USER_AUTHORIZATION_OPERATION_HISTORY_READ;
        break;
    default:
        SOPC_ASSERT(false); /* Unexpected operation type */
    }
}

/* Fill empty allocated and initialized variant dest with the given IndexRange of the source variant src */
constants_statuscodes_bs__t_StatusCode_i util_read_value_indexed_helper(SOPC_Variant* dst,
                                                                        const SOPC_Variant* src,
                                                                        const SOPC_NumericRange* range);

/* Fill empty allocated and initialized variant dest with the given IndexRange (as String) of the source variant src */
constants_statuscodes_bs__t_StatusCode_i util_read_value_string_indexed(SOPC_Variant* dst,
                                                                        const SOPC_Variant* src,
                                                                        const SOPC_String* range_str);

void util_NodeId_borrowReference_or_indet__C_to_B(constants__t_NodeId_i* bnodeId, SOPC_NodeId* nodeId);

/* Normalizes the returned value to be either 'true' or 'false' since those are tested in code translated from B model
 */
t_bool util_SOPC_Boolean_to_B(const SOPC_Boolean b);

/* Returns true if the security mode enumerated value is included in the security mode masks */
bool util_SecuModeEnumIncludedInSecuModeMasks(OpcUa_MessageSecurityMode msgSecurityMode, uint16_t securityModes);

#endif /* UTIL_B2C_H */
