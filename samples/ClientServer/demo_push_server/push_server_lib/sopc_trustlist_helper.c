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

#include <stdio.h>

#include "sopc_common_constants.h"
#include "sopc_encodeable.h"
#include "sopc_mem_alloc.h"
#include "sopc_trustlist_helper.h"

OpcUa_TrustListDataType* SOPC_TrustList_DecodeTrustListData(SOPC_ByteString* trustListData)
{
    SOPC_Buffer* pToDecode = SOPC_Buffer_Attach(trustListData->Data, (uint32_t) trustListData->Length);
    if (NULL == pToDecode)
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(pToDecode, 0);
    OpcUa_TrustListDataType* pTrustList = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pTrustList = SOPC_Calloc(1, sizeof(*pTrustList));
        OpcUa_TrustListDataType_Initialize(pTrustList);
        status = (NULL != pTrustList ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY);
    }
    /* Decode the byteString */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Decode(pTrustList->encodeableType, pTrustList, pToDecode, 0);
        if (SOPC_STATUS_OK != status)
        {
            printf("TrustList:%s:Decode: function failed\n", pTrustList->encodeableType->TypeName);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        if (NULL != pTrustList)
        {
            SOPC_Encodeable_Delete(pTrustList->encodeableType, (void**) &pTrustList);
        }
    }
    SOPC_Free(pToDecode);
    return pTrustList;
}

SOPC_ReturnStatus SOPC_TrustList_EncodeTrustListData(OpcUa_TrustListDataType* trustList, SOPC_ByteString* trustListData)
{
    SOPC_Buffer* pEncoded = SOPC_Buffer_CreateResizable(1024, SOPC_DEFAULT_MAX_STRING_LENGTH);
    if (NULL == pEncoded)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Encode the byteString */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_EncodeableObject_Encode(trustList->encodeableType, trustList, pEncoded, 0);
        if (SOPC_STATUS_OK != status)
        {
            printf("TrustList:%s:Encode: function failed\n", trustList->encodeableType->TypeName);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        trustListData->Data = pEncoded->data;
        trustListData->Length = (int32_t) pEncoded->length;
        // Moved data
        pEncoded->data = NULL;
        pEncoded->length = 0;
    }
    SOPC_Buffer_Delete(pEncoded);
    return status;
}
