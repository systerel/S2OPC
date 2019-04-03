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

#include "service_translate_browse_paths_bs.h"

#include "opcua_statuscodes.h"
#include "sopc_logger.h"

#include <assert.h>
#include <limits.h>
#include <string.h>

static const char* objects = "Objects.";

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_translate_browse_paths_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_translate_browse_paths_bs__treat_translate_browse_path_request(
    const constants__t_msg_i service_translate_browse_paths_bs__p_req_msg,
    const constants__t_msg_i service_translate_browse_paths_bs__p_resp_msg,
    constants_statuscodes_bs__t_StatusCode_i* const service_translate_browse_paths_bs__ret)
{
    OpcUa_TranslateBrowsePathsToNodeIdsRequest* req =
        (OpcUa_TranslateBrowsePathsToNodeIdsRequest*) service_translate_browse_paths_bs__p_req_msg;

    if (req->NoOfBrowsePaths == 0)
    {
        *service_translate_browse_paths_bs__ret = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
        return;
    }

    if (req->NoOfBrowsePaths > SOPC_MAX_OPERATIONS_PER_MSG)
    {
        *service_translate_browse_paths_bs__ret = constants_statuscodes_bs__e_sc_bad_too_many_ops;
        return;
    }

    OpcUa_TranslateBrowsePathsToNodeIdsResponse* resp =
        (OpcUa_TranslateBrowsePathsToNodeIdsResponse*) service_translate_browse_paths_bs__p_resp_msg;
    assert((uint64_t) req->NoOfBrowsePaths <= SIZE_MAX / sizeof(*resp->Results));
    resp->Results = malloc(sizeof(*resp->Results) * (size_t) req->NoOfBrowsePaths);

    if (NULL == resp->Results)
    {
        *service_translate_browse_paths_bs__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        return;
    }

    resp->NoOfResults = req->NoOfBrowsePaths;

    /* CAUTION: treatment only valid for FEP */

    for (int32_t i = 0; i < resp->NoOfResults; i++)
    {
        OpcUa_BrowsePath* bp = &req->BrowsePaths[i];
        OpcUa_BrowsePathResult* bpr = &resp->Results[i];
        OpcUa_BrowsePathResult_Initialize(bpr);

        // Check starting nodeId is "Objects" node
        if (bp->StartingNode.Namespace != OPCUA_NAMESPACE_INDEX ||
            bp->StartingNode.IdentifierType != SOPC_IdentifierType_Numeric || bp->StartingNode.Data.Numeric != 85)
        {
            bpr->NoOfTargets = 0;
            bpr->StatusCode = OpcUa_BadNoMatch;
            char* debug = SOPC_NodeId_ToCString(&bp->StartingNode);
            SOPC_Logger_TraceWarning(
                "service_translate_browse_paths_bs__treat_translate_browse_path_request: starting NodeId != 'Objects': "
                "%s",
                debug);
            free(debug);
        }
        else
        {
            char* pathString = NULL;
            size_t pathLength = 0;
            SOPC_StatusCode lengthZeroStatus = OpcUa_BadNothingToDo;
            bool failure = false;

            for (int32_t j = 0; !failure && j < bp->RelativePath.NoOfElements; j++)
            {
                OpcUa_RelativePathElement* rp = &bp->RelativePath.Elements[j];

                if (rp->TargetName.Name.Length > 0)
                {
                    assert((uint64_t) rp->TargetName.Name.Length < SIZE_MAX);
                    pathLength += (size_t) rp->TargetName.Name.Length + 1; //'.' separator or '\0' ending C string
                }
                else
                {
                    // NULL path (invalid) or wildcard (valid only in last path but not managed here)
                    failure = true;
                    pathLength = 0;
                    lengthZeroStatus = OpcUa_BadBrowseNameInvalid;
                }
            }

            if (!failure && pathLength != 0)
            {
                pathLength += (size_t) strlen(objects);
                pathString = malloc(sizeof(*pathString) * pathLength);
                bpr->Targets = malloc(sizeof(*bpr->Targets));
            }

            failure = (0 == pathLength || NULL == pathString || NULL == bpr->Targets);

            if (!failure)
            {
                // Copy "Objects." as path prefix
                char* res = strncpy(pathString, objects, strlen(objects));
                failure = res != pathString;
            }

            int32_t pathIndex = (int32_t) strlen(objects);
            uint16_t nsIndex = 0;

            for (int32_t j = 0; !failure && j < bp->RelativePath.NoOfElements; j++)
            {
                OpcUa_RelativePathElement* rp = &bp->RelativePath.Elements[j];
                assert((uint64_t) rp->TargetName.Name.Length <= SIZE_MAX);
                assert(CHAR_BIT == 8);
                char* res = strncpy(&pathString[pathIndex], (char*) rp->TargetName.Name.Data,
                                    (size_t) rp->TargetName.Name.Length);
                failure = res != &pathString[pathIndex];
                pathIndex += rp->TargetName.Name.Length;
                pathString[pathIndex] = '.';
                pathIndex++;
                nsIndex = rp->TargetName.NamespaceIndex; // Keep last namespace of path (should be the same)
            }

            if (!failure)
            {
                pathString[pathIndex - 1] = '\0';
                bpr->NoOfTargets = 1;
                bpr->StatusCode = SOPC_GoodGenericStatus;
                OpcUa_BrowsePathTarget_Initialize(bpr->Targets);
                bpr->Targets[0].RemainingPathIndex = UINT32_MAX;
                bpr->Targets[0].TargetId.NodeId.Namespace = nsIndex;
                bpr->Targets[0].TargetId.NodeId.IdentifierType = SOPC_IdentifierType_String;
                SOPC_ReturnStatus status =
                    SOPC_String_AttachFromCstring(&bpr->Targets[0].TargetId.NodeId.Data.String, pathString);
                bpr->Targets[0].TargetId.NodeId.Data.String.DoNotClear = false; // Attach and give ownership
                failure = (SOPC_STATUS_OK != status);
            }

            if (failure)
            {
                bpr->NoOfTargets = 0;
                bpr->StatusCode = pathLength == 0 ? lengthZeroStatus : OpcUa_BadOutOfMemory;
                free(pathString);
                free(bpr->Targets);
            }
        }
    }
}
