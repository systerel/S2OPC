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

#include <stdint.h>

#include "sopc_mem_alloc.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_types.h"

#include "helpers.h"

SOPC_ReturnStatus Helpers_AsyncLocalWrite(uint32_t endpointConfigIdx,
                                          SOPC_NodeId** lpNid,
                                          uint32_t* lAttrId,
                                          SOPC_DataValue** lpDv,
                                          size_t nItems,
                                          Helpers_WriteValue_Callback* wvNotifier)
{
    if (NULL == lpNid || NULL == lAttrId || NULL == lpDv || INT32_MAX < nItems)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    for (size_t i = 0; i < nItems; ++i)
    {
        /* Does not protect against invalid attribute ids */
        if (NULL == lpNid[i] || NULL == lpDv[i])
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    OpcUa_WriteRequest* request = SOPC_Calloc(1, sizeof(OpcUa_WriteRequest));
    OpcUa_WriteValue* lwv = SOPC_Calloc(nItems, sizeof(OpcUa_WriteValue));

    if (NULL == request || NULL == lwv)
    {
        SOPC_Free(request);
        SOPC_Free(lwv);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    OpcUa_WriteRequest_Initialize(request);
    request->NoOfNodesToWrite = (int32_t) nItems;

    /* Now fills the WriteValues */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (size_t i = 0; i < nItems && SOPC_STATUS_OK == status; ++i)
    {
        OpcUa_WriteValue* wv = &lwv[i];

        status = SOPC_NodeId_Copy(&wv->NodeId, lpNid[i]);
        wv->AttributeId = lAttrId[i];
        SOPC_String_Initialize(&wv->IndexRange);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_DataValue_Copy(&wv->Value, lpDv[i]);
        }
    }

    /* Optionnal before-hand write notification system */
    for (size_t i = 0; i < nItems && SOPC_STATUS_OK == status && NULL != wvNotifier; ++i)
    {
        wvNotifier(&lwv[i]);
    }

    /* Finish the WriteRequest and send it */
    if (SOPC_STATUS_OK == status)
    {
        request->NodesToWrite = lwv;
        SOPC_ToolkitServer_AsyncLocalServiceRequest(endpointConfigIdx, request, 0);
    }
    else
    {
        for (size_t i = 0; i < nItems; ++i)
        {
            OpcUa_WriteValue_Clear(&lwv[i]);
        }
        SOPC_Free(lwv);
        lwv = NULL;
        SOPC_Free(request);
        request = NULL;
    }

    return status;
}
