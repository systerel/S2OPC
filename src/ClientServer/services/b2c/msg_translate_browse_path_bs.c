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

#include "msg_translate_browse_path_bs.h"
#include "sopc_mem_alloc.h"
#include "util_b2c.h"

/* Globals */
static OpcUa_TranslateBrowsePathsToNodeIdsRequest* browsePaths_request;

static struct msg_translate_browse_path_bs__BrowsePathResult
{
    int32_t NoOfResults;
    OpcUa_BrowsePathResult* Results;
} browsePaths_results = {0, NULL};

static int32_t browsePaths_nbBrowsePaths = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_translate_browse_path_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void msg_translate_browse_path_bs__add_BrowsePath_Res_Target_Common(
    const constants__t_BrowsePath_i browsePath,
    const constants__t_ExpandedNodeId_i node,
    const uint32_t remainingIndex,
    constants_statuscodes_bs__t_StatusCode_i* const statusCode);

void msg_translate_browse_path_bs__decode_translate_browse_paths_request(
    const constants__t_msg_i msg_translate_browse_path_bs__req_msg,
    constants_statuscodes_bs__t_StatusCode_i* const msg_translate_browse_path_bs__StatusCode_service)
{
    assert(NULL != msg_translate_browse_path_bs__StatusCode_service);
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) msg_translate_browse_path_bs__req_msg;
    assert(encType == &OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType);
    browsePaths_request = (OpcUa_TranslateBrowsePathsToNodeIdsRequest*) msg_translate_browse_path_bs__req_msg;

    *msg_translate_browse_path_bs__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
    if (0 < browsePaths_request->NoOfBrowsePaths)
    {
        browsePaths_nbBrowsePaths = browsePaths_request->NoOfBrowsePaths;
    }
    else
    {
        browsePaths_nbBrowsePaths = 0;
    }
}

/* output p_nb_BrowsePaths is uinnt32_t */
void msg_translate_browse_path_bs__read_nb_BrowsePaths(t_entier4* const msg_translate_browse_path_bs__p_nb_BrowsePaths)
{
    assert(NULL != browsePaths_request);
    *msg_translate_browse_path_bs__p_nb_BrowsePaths = browsePaths_nbBrowsePaths;
}

static uint32_t msg_translate_browse_path_bs__get_BrowsePathIndex(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    int32_t indexMax);

uint32_t msg_translate_browse_path_bs__get_BrowsePathIndex(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    int32_t indexMax)
{
    assert(0 < msg_translate_browse_path_bs__browsePath);
    uint32_t browsePath_index = msg_translate_browse_path_bs__browsePath - 1;
    assert(browsePath_index < (uint32_t) indexMax);
    return browsePath_index;
}

/* browsePath is index in a array starting from 1 */
void msg_translate_browse_path_bs__read_BrowsePath_StartingNode(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    constants__t_NodeId_i* const msg_translate_browse_path_bs__nodeId)
{
    assert(NULL != browsePaths_request);
    uint32_t browsePath_index = msg_translate_browse_path_bs__get_BrowsePathIndex(
        msg_translate_browse_path_bs__browsePath, browsePaths_nbBrowsePaths);
    *msg_translate_browse_path_bs__nodeId = &(browsePaths_request->BrowsePaths[browsePath_index].StartingNode);
    if (SOPC_NodeId_IsNull(*msg_translate_browse_path_bs__nodeId))
    {
        *msg_translate_browse_path_bs__nodeId = constants__c_NodeId_indet;
    }
}

void msg_translate_browse_path_bs__read_BrowsePath_RelativePath(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    constants__t_RelativePath_i* const msg_translate_browse_path_bs__relativePath)
{
    assert(NULL != browsePaths_request);
    uint32_t browsePath_index = msg_translate_browse_path_bs__get_BrowsePathIndex(
        msg_translate_browse_path_bs__browsePath, browsePaths_nbBrowsePaths);
    *msg_translate_browse_path_bs__relativePath =
        (constants__t_RelativePath_i) & (browsePaths_request->BrowsePaths[browsePath_index].RelativePath);
}

void msg_translate_browse_path_bs__read_RelativePath_Nb_RelativePathElt(
    const constants__t_RelativePath_i msg_translate_browse_path_bs__relativePath,
    t_entier4* const msg_translate_browse_path_bs__nb_relativePathElt)
{
    assert(NULL != msg_translate_browse_path_bs__relativePath);
    *msg_translate_browse_path_bs__nb_relativePathElt = msg_translate_browse_path_bs__relativePath->NoOfElements;
}

void msg_translate_browse_path_bs__read_RelativePath_RelativePathElt(
    const constants__t_RelativePath_i msg_translate_browse_path_bs__relativePath,
    const t_entier4 msg_translate_browse_path_bs__index,
    constants__t_RelativePathElt_i* const msg_translate_browse_path_bs__relativePathElt)
{
    assert(NULL != msg_translate_browse_path_bs__relativePath);
    assert(NULL != msg_translate_browse_path_bs__relativePathElt);
    assert(0 < msg_translate_browse_path_bs__index);
    assert(msg_translate_browse_path_bs__index <= msg_translate_browse_path_bs__relativePath->NoOfElements);
    const int32_t index = msg_translate_browse_path_bs__index - 1;
    *msg_translate_browse_path_bs__relativePathElt = &(msg_translate_browse_path_bs__relativePath->Elements[index]);
}

void msg_translate_browse_path_bs__read_RelativePathElt_IncludedSubtypes(
    const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
    t_bool* const msg_translate_browse_path_bs__includedSubtypes)
{
    assert(NULL != msg_translate_browse_path_bs__relativePathElt);
    assert(NULL != msg_translate_browse_path_bs__includedSubtypes);
    if (msg_translate_browse_path_bs__relativePathElt->IncludeSubtypes)
    {
        *msg_translate_browse_path_bs__includedSubtypes = true;
    }
    else
    {
        *msg_translate_browse_path_bs__includedSubtypes = false;
    }
}

void msg_translate_browse_path_bs__read_RelativePathElt_IsInverse(
    const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
    t_bool* const msg_translate_browse_path_bs__isInverse)
{
    assert(NULL != msg_translate_browse_path_bs__relativePathElt);
    assert(NULL != msg_translate_browse_path_bs__isInverse);
    if (msg_translate_browse_path_bs__relativePathElt->IsInverse)
    {
        *msg_translate_browse_path_bs__isInverse = true;
    }
    else
    {
        *msg_translate_browse_path_bs__isInverse = false;
    }
}

void msg_translate_browse_path_bs__read_RelativePathElt_ReferenceTypeId(
    const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
    constants__t_NodeId_i* const msg_translate_browse_path_bs__referenceTypeId)
{
    assert(NULL != msg_translate_browse_path_bs__relativePathElt);
    assert(NULL != msg_translate_browse_path_bs__referenceTypeId);
    *msg_translate_browse_path_bs__referenceTypeId = &(msg_translate_browse_path_bs__relativePathElt->ReferenceTypeId);
    if (SOPC_NodeId_IsNull(*msg_translate_browse_path_bs__referenceTypeId))
    {
        *msg_translate_browse_path_bs__referenceTypeId = constants__c_NodeId_indet;
    }
}

void msg_translate_browse_path_bs__read_RelativePathElt_TargetName(
    const constants__t_RelativePathElt_i msg_translate_browse_path_bs__relativePathElt,
    constants__t_QualifiedName_i* const msg_translate_browse_path_bs__targetName)
{
    assert(NULL != msg_translate_browse_path_bs__relativePathElt);
    assert(NULL != msg_translate_browse_path_bs__targetName);
    *msg_translate_browse_path_bs__targetName = &(msg_translate_browse_path_bs__relativePathElt->TargetName);
}

void msg_translate_browse_path_bs__alloc_browse_path_result(
    constants_statuscodes_bs__t_StatusCode_i* const msg_translate_browse_path_bs__statusCode)
{
    assert(browsePaths_nbBrowsePaths > 0);
    browsePaths_results.Results = SOPC_Calloc((size_t) browsePaths_nbBrowsePaths, sizeof(OpcUa_BrowsePathResult));
    if (NULL == browsePaths_results.Results)
    {
        *msg_translate_browse_path_bs__statusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        browsePaths_results.NoOfResults = 0;
        return;
    }

    *msg_translate_browse_path_bs__statusCode = constants_statuscodes_bs__e_sc_ok;
    browsePaths_results.NoOfResults = browsePaths_nbBrowsePaths;
    for (int i = 0; i < browsePaths_nbBrowsePaths; i++)
    {
        OpcUa_BrowsePathResult_Initialize(&browsePaths_results.Results[i]);
    }
}

void msg_translate_browse_path_bs__write_BrowsePath_Res_StatusCode(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    const constants_statuscodes_bs__t_StatusCode_i msg_translate_browse_path_bs__statusCode)
{
    assert(NULL != browsePaths_results.Results);
    uint32_t browsePath_index = msg_translate_browse_path_bs__get_BrowsePathIndex(
        msg_translate_browse_path_bs__browsePath, browsePaths_results.NoOfResults);

    util_status_code__B_to_C(msg_translate_browse_path_bs__statusCode,
                             &(browsePaths_results.Results[browsePath_index].StatusCode));
}

void msg_translate_browse_path_bs__alloc_BrowsePath_Res_Target(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    const t_entier4 msg_translate_browse_path_bs__nbTargetMax,
    constants_statuscodes_bs__t_StatusCode_i* const msg_translate_browse_path_bs__statusCode)
{
    assert(NULL != browsePaths_results.Results);
    assert(msg_translate_browse_path_bs__nbTargetMax > 0);
    size_t allocSize = (size_t) msg_translate_browse_path_bs__nbTargetMax;
    uint32_t browsePath_index = msg_translate_browse_path_bs__get_BrowsePathIndex(
        msg_translate_browse_path_bs__browsePath, browsePaths_results.NoOfResults);

    browsePaths_results.Results[browsePath_index].Targets = SOPC_Calloc(allocSize, sizeof(OpcUa_BrowsePathTarget));
    if (NULL == browsePaths_results.Results[browsePath_index].Targets)
    {
        *msg_translate_browse_path_bs__statusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        *msg_translate_browse_path_bs__statusCode = constants_statuscodes_bs__e_sc_ok;
    }
}

/* Caller in B shall ensure that there is enough element */
void msg_translate_browse_path_bs__add_BrowsePath_Res_Target(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    const constants__t_ExpandedNodeId_i msg_translate_browse_path_bs__node,
    constants_statuscodes_bs__t_StatusCode_i* const msg_translate_browse_path_bs__statusCode)
{
    msg_translate_browse_path_bs__add_BrowsePath_Res_Target_Common(msg_translate_browse_path_bs__browsePath,
                                                                   msg_translate_browse_path_bs__node, UINT32_MAX,
                                                                   msg_translate_browse_path_bs__statusCode);
}

/* Caller in B shall ensure that there is enough element */
void msg_translate_browse_path_bs__add_BrowsePath_Res_Target_withRemainingPath(
    const constants__t_BrowsePath_i msg_translate_browse_path_bs__browsePath,
    const constants__t_ExpandedNodeId_i msg_translate_browse_path_bs__node,
    const t_entier4 msg_translate_browse_path_bs__remainingIndex,
    constants_statuscodes_bs__t_StatusCode_i* const msg_translate_browse_path_bs__statusCode)
{
    assert(0 <= msg_translate_browse_path_bs__remainingIndex);
    msg_translate_browse_path_bs__add_BrowsePath_Res_Target_Common(
        msg_translate_browse_path_bs__browsePath, msg_translate_browse_path_bs__node,
        (uint32_t) msg_translate_browse_path_bs__remainingIndex, msg_translate_browse_path_bs__statusCode);
}

/* Caller in B shall ensure that there is enough element */
void msg_translate_browse_path_bs__add_BrowsePath_Res_Target_Common(
    const constants__t_BrowsePath_i browsePath,
    const constants__t_ExpandedNodeId_i node,
    const uint32_t remainingIndex,
    constants_statuscodes_bs__t_StatusCode_i* const statusCode)
{
    assert(NULL != browsePaths_results.Results);
    assert(NULL != statusCode);
    assert(NULL != node);

    uint32_t browsePath_index =
        msg_translate_browse_path_bs__get_BrowsePathIndex(browsePath, browsePaths_results.NoOfResults);
    OpcUa_BrowsePathResult* result = &browsePaths_results.Results[browsePath_index];

    assert(NULL != result->Targets);
    *statusCode = constants_statuscodes_bs__e_sc_ok;

    /* Initialize and set. result->NoOfTargets is the current element */

    OpcUa_BrowsePathTarget_Initialize(&result->Targets[result->NoOfTargets]);
    SOPC_ExpandedNodeId_Initialize(&result->Targets[result->NoOfTargets].TargetId);

    const SOPC_ReturnStatus alloc = SOPC_ExpandedNodeId_Copy(&result->Targets[result->NoOfTargets].TargetId, node);
    if (SOPC_STATUS_OK != alloc)
    {
        *statusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        result->Targets[result->NoOfTargets].RemainingPathIndex = remainingIndex;
        result->NoOfTargets++;
    }
}

void msg_translate_browse_path_bs__write_translate_browse_paths_response(
    const constants__t_msg_i msg_translate_browse_path_bs__p_msg_out)
{
    /* Check and get type of message */
    SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) msg_translate_browse_path_bs__p_msg_out;
    assert(encType == &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType);
    OpcUa_TranslateBrowsePathsToNodeIdsResponse* browsePaths_response =
        (OpcUa_TranslateBrowsePathsToNodeIdsResponse*) msg_translate_browse_path_bs__p_msg_out;

    OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize(browsePaths_response);
    browsePaths_response->NoOfResults = browsePaths_results.NoOfResults;
    browsePaths_response->Results = browsePaths_results.Results;

    browsePaths_results.NoOfResults = 0;
    browsePaths_results.Results = NULL;
}
