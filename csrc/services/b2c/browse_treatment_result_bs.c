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

#include "browse_treatment_result_bs.h"

#include "util_b2c.h"

#include <assert.h>
#include <stdlib.h>

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
        references = calloc((size_t) browse_treatment_result_bs__p_maxResultRefs, sizeof(*references));
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
    free(references);
    nbMaxReferences = 0;
}

void browse_treatment_result_bs__get_browse_result_nb_references(
    t_entier4* const browse_treatment_result_bs__p_nb_references)
{
    *browse_treatment_result_bs__p_nb_references = nbReferences;
}

void browse_treatment_result_bs__getall_and_clear_browse_result(
    t_entier4* const browse_treatment_result_bs__p_nb_references,
    constants__t_BrowseResultReferences_i* const browse_treatment_result_bs__p_browseResult)
{
    *browse_treatment_result_bs__p_browseResult = references;
    *browse_treatment_result_bs__p_nb_references = nbReferences;
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
    if (refDesc->DisplayName.Text.Length > 0 || refDesc->DisplayName.Locale.Length > 0)
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

void browse_treatment_result_bs__setall_browse_result_reference_at(
    const t_entier4 browse_treatment_result_bs__p_refIndex,
    const constants__t_NodeId_i browse_treatment_result_bs__p_refTypeId,
    const t_bool browse_treatment_result_bs__p_isForward,
    const constants__t_ExpandedNodeId_i browse_treatment_result_bs__p_NodeId,
    const constants__t_QualifiedName_i browse_treatment_result_bs__p_BrowseName,
    const constants__t_LocalizedText_i browse_treatment_result_bs__p_DisplayName,
    const constants__t_NodeClass_i browse_treatment_result_bs__p_NodeClass,
    const constants__t_ExpandedNodeId_i browse_treatment_result_bs__p_TypeDefinition)
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Mandatory ReferenceTypeId: cannot be indet (ensured by following assertion)
    status = SOPC_NodeId_Copy(&refDesc->ReferenceTypeId, browse_treatment_result_bs__p_refTypeId);
    assert(SOPC_STATUS_OK == status);
    // Mandatory IsForward
    refDesc->IsForward = browse_treatment_result_bs__p_isForward;
    // Mandatory TargetNodeId: cannot be indet (ensured by following assertion)
    status = SOPC_ExpandedNodeId_Copy(&refDesc->NodeId, browse_treatment_result_bs__p_NodeId);
    assert(SOPC_STATUS_OK == status);
    // Optional BrowseName
    if (browse_treatment_result_bs__p_BrowseName != constants__c_QualifiedName_indet)
    {
        status = SOPC_QualifiedName_Copy(&refDesc->BrowseName, browse_treatment_result_bs__p_BrowseName);
        assert(SOPC_STATUS_OK == status);
    }
    // Optional DisplayName
    if (browse_treatment_result_bs__p_DisplayName != constants__c_LocalizedText_indet)
    {
        status = SOPC_LocalizedText_Copy(&refDesc->DisplayName, browse_treatment_result_bs__p_DisplayName);
        assert(SOPC_STATUS_OK == status);
    }
    // Optional NodeClass
    if (browse_treatment_result_bs__p_NodeClass != constants__c_NodeClass_indet)
    {
        bool res = util_NodeClass__B_to_C(browse_treatment_result_bs__p_NodeClass, &refDesc->NodeClass);
        assert(res);
    }
    // Optional TypeDefinition
    if (browse_treatment_result_bs__p_TypeDefinition != constants__c_ExpandedNodeId_indet)
    {
        status = SOPC_ExpandedNodeId_Copy(&refDesc->TypeDefinition, browse_treatment_result_bs__p_TypeDefinition);
        assert(SOPC_STATUS_OK == status);
    }
    nbReferences = browse_treatment_result_bs__p_refIndex;
}

void browse_treatment_result_bs__unused_browse_view(
    const constants__t_NodeId_i browse_treatment_result_bs__p_browseView)
{
    (void) browse_treatment_result_bs__p_browseView;
}
