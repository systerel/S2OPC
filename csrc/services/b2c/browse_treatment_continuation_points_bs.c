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

#include "browse_treatment_continuation_points_bs.h"

#include <string.h>

#include "sopc_mem_alloc.h"
#include "util_b2c.h"

static uint64_t continuationPointIdSeed = constants__c_ContinuationPointId_indet;

void browse_treatment_continuation_points_bs__INITIALISATION(void)
{
    continuationPointIdSeed = SOPC_TimeReference_GetCurrent();
}

static uint64_t get_freshContinuationPointId(void)
{
    continuationPointIdSeed++;
    if (continuationPointIdSeed == constants__c_ContinuationPointId_indet)
    {
        continuationPointIdSeed++;
    }
    return continuationPointIdSeed;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_continuation_points_bs__clear_continuation_point(
    const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_continuationPoint)
{
    SOPC_NodeId_Clear(browse_treatment_continuation_points_bs__p_continuationPoint.browseView);
    SOPC_Free(browse_treatment_continuation_points_bs__p_continuationPoint.browseView);
    SOPC_NodeId_Clear(browse_treatment_continuation_points_bs__p_continuationPoint.nodeId);
    SOPC_Free(browse_treatment_continuation_points_bs__p_continuationPoint.nodeId);
    SOPC_NodeId_Clear(browse_treatment_continuation_points_bs__p_continuationPoint.referenceTypeId);
    SOPC_Free(browse_treatment_continuation_points_bs__p_continuationPoint.referenceTypeId);
}

void browse_treatment_continuation_points_bs__create_continuation_point_bs(
    const t_entier4 browse_treatment_continuation_points_bs__p_nextIndex,
    const t_entier4 browse_treatment_continuation_points_bs__p_maxTargetRef,
    const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_browseView,
    const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_nodeId,
    const constants__t_BrowseDirection_i browse_treatment_continuation_points_bs__p_browseDirection,
    const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_referenceType,
    const t_bool browse_treatment_continuation_points_bs__p_includeSubtypes,
    const constants__t_BrowseNodeClassMask_i browse_treatment_continuation_points_bs__p_nodeClassMask,
    const constants__t_BrowseResultMask_i browse_treatment_continuation_points_bs__p_resultMask,
    t_bool* const browse_treatment_continuation_points_bs__bres,
    constants__t_ContinuationPoint_i* const browse_treatment_continuation_points_bs__p_ContinuationPoint)
{
    *browse_treatment_continuation_points_bs__bres = false;
    *browse_treatment_continuation_points_bs__p_ContinuationPoint = constants__c_ContinuationPoint_indet;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool allocSuccess = true;
    SOPC_ContinuationPointData data;
    memset(&data, 0, sizeof(data));

    if (allocSuccess && browse_treatment_continuation_points_bs__p_browseView != constants__c_NodeId_indet)
    {
        data.browseView = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        allocSuccess = NULL != data.browseView;
        if (allocSuccess)
        {
            status = SOPC_NodeId_Copy(data.browseView, browse_treatment_continuation_points_bs__p_browseView);
            allocSuccess = SOPC_STATUS_OK == status;
        }
    }

    if (allocSuccess && browse_treatment_continuation_points_bs__p_nodeId != constants__c_NodeId_indet)
    {
        data.nodeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        allocSuccess = NULL != data.nodeId;
        if (allocSuccess)
        {
            status = SOPC_NodeId_Copy(data.nodeId, browse_treatment_continuation_points_bs__p_nodeId);
            allocSuccess = SOPC_STATUS_OK == status;
        }
    }

    if (allocSuccess && browse_treatment_continuation_points_bs__p_referenceType != constants__c_NodeId_indet)
    {
        data.referenceTypeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        allocSuccess = NULL != data.referenceTypeId;
        if (allocSuccess)
        {
            status = SOPC_NodeId_Copy(data.referenceTypeId, browse_treatment_continuation_points_bs__p_referenceType);
            allocSuccess = SOPC_STATUS_OK == status;
        }
    }

    if (allocSuccess)
    {
        data.continuationPointId = get_freshContinuationPointId();
        data.nextRefIndexOnNode = browse_treatment_continuation_points_bs__p_nextIndex;
        data.maxTargetReferencesToReturn = browse_treatment_continuation_points_bs__p_maxTargetRef;
        data.browseDirection = util_BrowseDirection__B_to_C(browse_treatment_continuation_points_bs__p_browseDirection);
        data.includeSubtypes = browse_treatment_continuation_points_bs__p_includeSubtypes;
        data.nodeClassMask = browse_treatment_continuation_points_bs__p_nodeClassMask;
        data.resultMask = browse_treatment_continuation_points_bs__p_resultMask;

        *browse_treatment_continuation_points_bs__bres = true;
        *browse_treatment_continuation_points_bs__p_ContinuationPoint = data;
    }
    else
    {
        SOPC_Free(data.browseView);
        SOPC_Free(data.nodeId);
        SOPC_Free(data.referenceTypeId);
    }
}

void browse_treatment_continuation_points_bs__get_continuation_point_id(
    const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_continuationPoint,
    constants__t_ContinuationPointId_i* const browse_treatment_continuation_points_bs__p_continuationPointId)
{
    assert(browse_treatment_continuation_points_bs__p_continuationPoint.continuationPointId != 0);
    *browse_treatment_continuation_points_bs__p_continuationPointId =
        browse_treatment_continuation_points_bs__p_continuationPoint.continuationPointId;
}

void browse_treatment_continuation_points_bs__getall_continuation_point_bs(
    const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_ContinuationPoint,
    t_entier4* const browse_treatment_continuation_points_bs__p_nextIndex,
    t_entier4* const browse_treatment_continuation_points_bs__p_maxTargetRef,
    constants__t_NodeId_i* const browse_treatment_continuation_points_bs__p_browseView,
    constants__t_NodeId_i* const browse_treatment_continuation_points_bs__p_nodeId,
    constants__t_BrowseDirection_i* const browse_treatment_continuation_points_bs__p_browseDirection,
    constants__t_NodeId_i* const browse_treatment_continuation_points_bs__p_referenceType,
    t_bool* const browse_treatment_continuation_points_bs__p_includeSubtypes,
    constants__t_BrowseNodeClassMask_i* const browse_treatment_continuation_points_bs__p_nodeClassMask,
    constants__t_BrowseResultMask_i* const browse_treatment_continuation_points_bs__p_resultMask)
{
    SOPC_ContinuationPointData data = browse_treatment_continuation_points_bs__p_ContinuationPoint;
    assert(data.continuationPointId != 0);
    *browse_treatment_continuation_points_bs__p_nextIndex = data.nextRefIndexOnNode;
    *browse_treatment_continuation_points_bs__p_maxTargetRef = data.maxTargetReferencesToReturn;
    *browse_treatment_continuation_points_bs__p_browseView = data.browseView;
    *browse_treatment_continuation_points_bs__p_nodeId = data.nodeId;
    *browse_treatment_continuation_points_bs__p_browseDirection = util_BrowseDirection__C_to_B(data.browseDirection);
    *browse_treatment_continuation_points_bs__p_referenceType = data.referenceTypeId;
    *browse_treatment_continuation_points_bs__p_includeSubtypes = data.includeSubtypes;
    *browse_treatment_continuation_points_bs__p_nodeClassMask = data.nodeClassMask;
    *browse_treatment_continuation_points_bs__p_resultMask = data.resultMask;
}
