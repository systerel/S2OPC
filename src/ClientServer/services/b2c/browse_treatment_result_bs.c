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

#include <assert.h>

#include "browse_treatment_result_bs.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

#define RESULT_MASK_REFERENCE_TYPE 1   // bit 0
#define RESULT_MASK_IS_FORWARD 2       // bit 1
#define RESULT_MASK_NODE_CLASS 4       // bit 2
#define RESULT_MASK_BROWSE_NAME 8      // bit 3
#define RESULT_MASK_DISPLAY_NAME 16    // bit 4
#define RESULT_MASK_TYPE_DEFINITION 32 // bit 5

static OpcUa_ReferenceDescription* references = NULL;
static int32_t nbReferences = 0;
static int32_t nbMaxReferences = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_result_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_result_bs__alloc_browse_result(const t_entier4 browse_treatment_result_bs__p_maxResultRefs,
                                                     t_bool* const browse_treatment_result_bs__p_alloc_bres)
{
    *browse_treatment_result_bs__p_alloc_bres = false;

    if (browse_treatment_result_bs__p_maxResultRefs > 0)
    {
        assert((uint64_t) browse_treatment_result_bs__p_maxResultRefs <= SIZE_MAX);
        references = SOPC_Calloc((size_t) browse_treatment_result_bs__p_maxResultRefs, sizeof(*references));
        if (NULL != references)
        {
            nbReferences = 0;
            nbMaxReferences = browse_treatment_result_bs__p_maxResultRefs;
            *browse_treatment_result_bs__p_alloc_bres = true;
        }
    }
    else
    {
        references = NULL;
        nbReferences = 0;
        nbMaxReferences = 0;
        *browse_treatment_result_bs__p_alloc_bres = true;
    }
}

void browse_treatment_result_bs__clear_browse_result(void)
{
    for (int32_t i = 0; i < nbReferences; i++)
    {
        OpcUa_ReferenceDescription_Clear(&references[i]);
    }
    SOPC_Free(references);
    nbMaxReferences = 0;
}

void browse_treatment_result_bs__get_browse_result_nb_references(
    t_entier4* const browse_treatment_result_bs__p_nb_references)
{
    *browse_treatment_result_bs__p_nb_references = nbReferences;
}

void browse_treatment_result_bs__getall_and_move_browse_result(
    t_entier4* const browse_treatment_result_bs__p_nb_references,
    constants__t_BrowseResultReferences_i* const browse_treatment_result_bs__p_browseResult)
{
    *browse_treatment_result_bs__p_browseResult = references;
    *browse_treatment_result_bs__p_nb_references = nbReferences;
    references = NULL;
    nbReferences = 0;
    nbMaxReferences = 0;
}

/*
 * Note: Borrowed references: to be copied when used
 */
void browse_treatment_result_bs__getall_browse_result_reference_at(
    const t_entier4 browse_treatment_result_bs__p_refIndex,
    constants__t_NodeId_i* const browse_treatment_result_bs__p_refTypeId,
    t_bool* const browse_treatment_result_bs__p_isForward,
    constants__t_ExpandedNodeId_i* const browse_treatment_result_bs__p_NodeId,
    constants__t_QualifiedName_i* const browse_treatment_result_bs__p_BrowseName,
    constants__t_LocalizedText_i* const browse_treatment_result_bs__p_DisplayName,
    constants__t_NodeClass_i* const browse_treatment_result_bs__p_NodeClass,
    constants__t_ExpandedNodeId_i* const browse_treatment_result_bs__p_TypeDefinition)
{
    // Note browse_treatment_result_bs__p_refIndex : 1..nbMaxReferences to be translated in C array idx by adding (-1)
    assert(browse_treatment_result_bs__p_refIndex > 0);
    assert(browse_treatment_result_bs__p_refIndex <= nbReferences);
    OpcUa_ReferenceDescription* refDesc = &references[browse_treatment_result_bs__p_refIndex - 1];

    // Mandatory ReferenceTypeId
    *browse_treatment_result_bs__p_refTypeId = &refDesc->ReferenceTypeId;
    // Mandatory IsForward
    *browse_treatment_result_bs__p_isForward = refDesc->IsForward;
    // Mandatory TargetNodeId
    *browse_treatment_result_bs__p_NodeId = &refDesc->NodeId;

    // Optional BrowseName
    if (refDesc->BrowseName.Name.Length > 0 || refDesc->BrowseName.NamespaceIndex > 0)
    {
        *browse_treatment_result_bs__p_BrowseName = &refDesc->BrowseName;
    }
    else
    {
        *browse_treatment_result_bs__p_BrowseName = constants__c_QualifiedName_indet;
    }
    // Optional DisplayName
    if (refDesc->DisplayName.defaultText.Length > 0 || refDesc->DisplayName.defaultLocale.Length > 0)
    {
        *browse_treatment_result_bs__p_DisplayName = &refDesc->DisplayName;
    }
    else
    {
        *browse_treatment_result_bs__p_DisplayName = constants__c_LocalizedText_indet;
    }
    // Optional NodeClass
    if (OpcUa_NodeClass_Unspecified != refDesc->NodeClass)
    {
        bool res = util_NodeClass__C_to_B(refDesc->NodeClass, browse_treatment_result_bs__p_NodeClass);
        assert(res);
    }
    // Optional TypeDefinition
    if (0 == refDesc->TypeDefinition.NamespaceUri.Length && 0 == refDesc->TypeDefinition.ServerIndex &&
        SOPC_NodeId_IsNull(&refDesc->TypeDefinition.NodeId))
    {
        *browse_treatment_result_bs__p_TypeDefinition = constants__c_ExpandedNodeId_indet;
    }
    else
    {
        *browse_treatment_result_bs__p_TypeDefinition = &refDesc->TypeDefinition;
    }
}

void browse_treatment_result_bs__is_BrowseName_in_mask(
    const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
    t_bool* const browse_treatment_result_bs__bres)
{
    *browse_treatment_result_bs__bres = 0 != (browse_treatment_result_bs__p_resultMask & RESULT_MASK_BROWSE_NAME);
}

void browse_treatment_result_bs__is_DisplayName_in_mask(
    const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
    t_bool* const browse_treatment_result_bs__bres)
{
    *browse_treatment_result_bs__bres = 0 != (browse_treatment_result_bs__p_resultMask & RESULT_MASK_DISPLAY_NAME);
}

void browse_treatment_result_bs__is_IsForward_in_mask(
    const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
    t_bool* const browse_treatment_result_bs__bres)
{
    *browse_treatment_result_bs__bres = 0 != (browse_treatment_result_bs__p_resultMask & RESULT_MASK_IS_FORWARD);
}

void browse_treatment_result_bs__is_NodeClass_in_mask(
    const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
    t_bool* const browse_treatment_result_bs__bres)
{
    *browse_treatment_result_bs__bres = 0 != (browse_treatment_result_bs__p_resultMask & RESULT_MASK_NODE_CLASS);
}

void browse_treatment_result_bs__is_ReferenceType_in_mask(
    const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
    t_bool* const browse_treatment_result_bs__bres)
{
    *browse_treatment_result_bs__bres = 0 != (browse_treatment_result_bs__p_resultMask & RESULT_MASK_REFERENCE_TYPE);
}

void browse_treatment_result_bs__is_TypeDefinition_in_mask(
    const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
    t_bool* const browse_treatment_result_bs__bres)
{
    *browse_treatment_result_bs__bres = 0 != (browse_treatment_result_bs__p_resultMask & RESULT_MASK_TYPE_DEFINITION);
}

void browse_treatment_result_bs__setall_browse_result_reference_at(
    const t_entier4 browse_treatment_result_bs__p_refIndex,
    const constants__t_NodeId_i browse_treatment_result_bs__p_refTypeId,
    const t_bool browse_treatment_result_bs__p_isForward,
    const constants__t_ExpandedNodeId_i browse_treatment_result_bs__p_NodeId,
    const constants__t_QualifiedName_i browse_treatment_result_bs__p_BrowseName,
    const constants__t_LocalizedText_i browse_treatment_result_bs__p_DisplayName,
    const constants__t_NodeClass_i browse_treatment_result_bs__p_NodeClass,
    const constants__t_ExpandedNodeId_i browse_treatment_result_bs__p_TypeDefinition,
    t_bool* const browse_treatment_result_bs__p_alloc_failed)
{
    assert(NULL != references);
    /* Notes:
     * - browse_treatment_result_bs__p_refIndex : 1..nbMaxReferences to be translated in C array idx by adding (-1)
     * - index are set by the B model in the increasing order from 1 to <N>
     */
    assert(browse_treatment_result_bs__p_refIndex > 0);
    assert(browse_treatment_result_bs__p_refIndex - 1 == nbReferences);
    assert(browse_treatment_result_bs__p_refIndex <= nbMaxReferences);
    OpcUa_ReferenceDescription* refDesc = &references[browse_treatment_result_bs__p_refIndex - 1];
    OpcUa_ReferenceDescription_Initialize(refDesc);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // ReferenceTypeId: mandatory (except if filtered by ResultMask)
    if (constants__c_NodeId_indet != browse_treatment_result_bs__p_refTypeId)
    {
        status = SOPC_NodeId_Copy(&refDesc->ReferenceTypeId, browse_treatment_result_bs__p_refTypeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        // IsForward: mandatory (except if filtered by ResultMask => false value in this case)
        refDesc->IsForward = browse_treatment_result_bs__p_isForward;
        // TargetNodeId: cannot be indet (ensured by following assertion)
        assert(constants_bs__c_ExpandedNodeId_indet != browse_treatment_result_bs__p_NodeId);
        status = SOPC_ExpandedNodeId_Copy(&refDesc->NodeId, browse_treatment_result_bs__p_NodeId);
    }

    // Optional BrowseName
    if (SOPC_STATUS_OK == status && browse_treatment_result_bs__p_BrowseName != constants__c_QualifiedName_indet)
    {
        status = SOPC_QualifiedName_Copy(&refDesc->BrowseName, browse_treatment_result_bs__p_BrowseName);
    }
    // Optional DisplayName
    if (SOPC_STATUS_OK == status && browse_treatment_result_bs__p_DisplayName != constants__c_LocalizedText_indet)
    {
        status = SOPC_LocalizedText_Copy(&refDesc->DisplayName, browse_treatment_result_bs__p_DisplayName);
    }
    // Optional NodeClass
    if (SOPC_STATUS_OK == status && browse_treatment_result_bs__p_NodeClass != constants__c_NodeClass_indet)
    {
        bool res = util_NodeClass__B_to_C(browse_treatment_result_bs__p_NodeClass, &refDesc->NodeClass);
        assert(res);
    }
    // Optional TypeDefinition
    if (SOPC_STATUS_OK == status && browse_treatment_result_bs__p_TypeDefinition != constants__c_ExpandedNodeId_indet)
    {
        status = SOPC_ExpandedNodeId_Copy(&refDesc->TypeDefinition, browse_treatment_result_bs__p_TypeDefinition);
    }

    if (SOPC_STATUS_OK == status)
    {
        nbReferences = browse_treatment_result_bs__p_refIndex;
        *browse_treatment_result_bs__p_alloc_failed = false;
    }
    else
    {
        *browse_treatment_result_bs__p_alloc_failed = true;
        OpcUa_ReferenceDescription_Clear(refDesc);
    }
}

void browse_treatment_result_bs__unused_browse_view(
    const constants__t_NodeId_i browse_treatment_result_bs__p_browseView)
{
    SOPC_UNUSED_ARG(browse_treatment_result_bs__p_browseView);
}
