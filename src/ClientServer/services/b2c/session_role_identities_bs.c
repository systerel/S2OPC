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

#include "session_role_identities_bs.h"
#include "address_space_impl.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_role_identities_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

// Variant is a list of ExtensionObject.
static SOPC_Variant* session_role_identities_bs__check_and_get_variant(
    const constants__t_Variant_i session_role_identities_bs__p_variant)
{
    SOPC_Variant* variant = session_role_identities_bs__p_variant;
    // + add check good type of variant first element
    if (NULL == variant || SOPC_ExtensionObject_Id != variant->BuiltInTypeId)
    {
        return NULL;
    }
    return variant;
}

extern void session_role_identities_bs__read_variant_identity(
    const constants__t_Variant_i session_role_identities_bs__p_variant,
    const t_entier4 session_role_identities_bs__p_index,
    constants__t_Identity_i* const session_role_identities_bs__p_identity)
{
    SOPC_ASSERT(NULL != session_role_identities_bs__p_identity);

    SOPC_Variant* variant = session_role_identities_bs__check_and_get_variant(session_role_identities_bs__p_variant);
    SOPC_ASSERT(NULL != variant);
    SOPC_ExtensionObject* extObjectArr = NULL;
    if (SOPC_VariantArrayType_SingleValue == variant->ArrayType && 1 == session_role_identities_bs__p_index)
    {
        extObjectArr = variant->Value.ExtObject;
    }
    else
    {
        int32_t length = variant->Value.Array.Length;
        SOPC_ASSERT(0 < session_role_identities_bs__p_index && session_role_identities_bs__p_index <= length);
        extObjectArr = &variant->Value.Array.Content.ExtObjectArr[session_role_identities_bs__p_index - 1];
    }
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == extObjectArr->Encoding); // .. encoded in object ..
    SOPC_ASSERT(&OpcUa_IdentityMappingRuleType_EncodeableType ==
                extObjectArr->Body.Object.ObjType); // .. of type IdentityMappingRule
    *session_role_identities_bs__p_identity = (OpcUa_IdentityMappingRuleType*) extObjectArr->Body.Object.Value;
    SOPC_ASSERT(NULL != *session_role_identities_bs__p_identity);
    SOPC_ASSERT(&OpcUa_IdentityMappingRuleType_EncodeableType ==
                (*session_role_identities_bs__p_identity)->encodeableType);
}

extern void session_role_identities_bs__read_variant_nb_identities(
    const constants__t_Variant_i session_role_identities_bs__p_variant,
    const constants__t_Node_i session_role_identities_bs__p_node,
    t_entier4* const session_role_identities_bs__p_nb)
{
    SOPC_ASSERT(NULL != session_role_identities_bs__p_nb);

    SOPC_Variant* variant = session_role_identities_bs__check_and_get_variant(session_role_identities_bs__p_variant);
    if (NULL == variant)
    {
        char* nodeId = SOPC_NodeId_ToCString(
            SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, session_role_identities_bs__p_node));
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "identities of node %s cannot be found or have incorrect format in address space",
                               nodeId);
        SOPC_Free(nodeId);
        *session_role_identities_bs__p_nb = 0;
    }
    else
    {
        if (SOPC_VariantArrayType_SingleValue == variant->ArrayType)
        {
            *session_role_identities_bs__p_nb = 1;
        }
        else
        {
            *session_role_identities_bs__p_nb = variant->Value.Array.Length;
        }
    }
}
