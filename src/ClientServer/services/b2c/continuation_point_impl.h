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
 * \brief Declares continuation point structure to store continuation data
 */

#ifndef CONTINUATION_POINT_IMPL_H_
#define CONTINUATION_POINT_IMPL_H_

#include "sopc_types.h"

typedef struct SOPC_ContinuationPointData
{
    uint64_t continuationPointId;

    int32_t nextRefIndexOnNode;
    int32_t maxTargetReferencesToReturn;
    SOPC_NodeId* browseView;
    SOPC_NodeId* nodeId;
    OpcUa_BrowseDirection browseDirection;
    SOPC_NodeId* referenceTypeId;
    bool includeSubtypes;
    uint32_t nodeClassMask;
    uint32_t resultMask;
} SOPC_ContinuationPointData;

static const SOPC_ContinuationPointData sopc_continuationPointData_empty = {0, 0, 0, NULL, NULL, 0, NULL, false, 0, 0};

/**
 * Encode a continuation point identifier into a byte string
 */
SOPC_ReturnStatus SOPC_ContinuationPointId_Encode(uint64_t continuationPointId, SOPC_ByteString* bs);

/**
 * Decode a continuation point identifier from a byte string
 */
SOPC_ReturnStatus SOPC_ContinuationPointId_Decode(const SOPC_ByteString* bs, uint64_t* continuationPointId);

#endif /* CONTINUATION_POINT_IMPL_H_ */
