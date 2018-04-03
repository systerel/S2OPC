/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 * Implements the base machine that stores the BrowseResponse elements.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "msg_browse_response_bs.h"
#include "util_b2c.h"

#include "sopc_logger.h"
#include "sopc_types.h"

/* Message elements */
/* All arrays start at 1, to be directly compatible with B operations */
static int32_t nBrowseResult;
static SOPC_StatusCode* pBrowseStatus;
/*static  *pContinuationPoint;*/
static int32_t* pnAllocReferenceDescription; /** Number of usable RefDescr per BrowseResult (number of allocation is
                                                nAlloc + 1 so that indices are B-compatible) */
static int32_t* pnReferenceDescription;      /** Number of used RefDescr by each BrowseResult */
static SOPC_NodeId** ppResRefTypeId;
static bool** ppResForwards;
static SOPC_ExpandedNodeId** ppResNodeId;
static SOPC_QualifiedName** ppResBrowseName;
static SOPC_LocalizedText** ppResDisplayName;
static OpcUa_NodeClass** ppResNodeClass;
static SOPC_ExpandedNodeId** ppResTypeDefinition;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_browse_response_bs__INITIALISATION(void)
{
    nBrowseResult = 0;
    pBrowseStatus = NULL;
    pnAllocReferenceDescription = NULL;
    pnReferenceDescription = NULL;
    ppResRefTypeId = NULL;
    ppResForwards = NULL;
    ppResNodeId = NULL;
    ppResBrowseName = NULL;
    ppResDisplayName = NULL;
    ppResNodeClass = NULL;
    ppResTypeDefinition = NULL;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_browse_response_bs__set_ResponseBrowse_BrowseStatus(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_StatusCode_i msg_browse_response_bs__p_sc)
{
    assert(NULL != pBrowseStatus);
    util_status_code__B_to_C(msg_browse_response_bs__p_sc, &pBrowseStatus[msg_browse_response_bs__p_bvi]);
}

void msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_Reference_i msg_browse_response_bs__p_ref)
{
    (void) msg_browse_response_bs__p_bvi;
    (void) msg_browse_response_bs__p_ref;
    static bool bWarned = false;
    if (!bWarned)
    {
        SOPC_Logger_TraceWarning("Not implemented: msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint");
        bWarned = true;
    }
}

void msg_browse_response_bs__reset_ResponseBrowse_ContinuationPoint(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi)
{
    (void) msg_browse_response_bs__p_bvi;
    static bool bWarned = false;
    if (!bWarned)
    {
        SOPC_Logger_TraceWarning("Not implemented: msg_browse_response_bs__RESET_ResponseBrowse_ContinuationPoint");
        bWarned = true;
    }
}

void msg_browse_response_bs__set_ResponseBrowse_Res_ReferenceTypeId(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_NodeId_i msg_browse_response_bs__p_NodeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    assert(NULL != ppResRefTypeId);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResRefTypeId[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    status = SOPC_NodeId_Copy(&ppResRefTypeId[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                              (SOPC_NodeId*) msg_browse_response_bs__p_NodeId);
    assert(SOPC_STATUS_OK == status);
}

void msg_browse_response_bs__set_ResponseBrowse_Res_Forwards(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const t_bool msg_browse_response_bs__p_bool)
{
    assert(NULL != ppResForwards);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResForwards[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    ppResForwards[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri] = msg_browse_response_bs__p_bool;
}

void msg_browse_response_bs__set_ResponseBrowse_Res_BrowseName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_QualifiedName_i msg_browse_response_bs__p_BrowseName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    assert(NULL != ppResBrowseName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResBrowseName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    status = SOPC_QualifiedName_Copy(&ppResBrowseName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                     (SOPC_QualifiedName*) msg_browse_response_bs__p_BrowseName);
    assert(SOPC_STATUS_OK == status);
}

void msg_browse_response_bs__reset_ResponseBrowse_Res_BrowseName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResBrowseName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResBrowseName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    SOPC_QualifiedName_Clear(&ppResBrowseName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]);
}

void msg_browse_response_bs__set_ResponseBrowse_Res_DisplayName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_LocalizedText_i msg_browse_response_bs__p_DisplayName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    assert(NULL != ppResDisplayName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResDisplayName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    status = SOPC_LocalizedText_Copy(&ppResDisplayName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                     (SOPC_LocalizedText*) msg_browse_response_bs__p_DisplayName);
    assert(SOPC_STATUS_OK == status);
}

void msg_browse_response_bs__reset_ResponseBrowse_Res_DisplayName(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResDisplayName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResDisplayName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    SOPC_LocalizedText_Clear(&ppResDisplayName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]);
}

void msg_browse_response_bs__set_ResponseBrowse_Res_NodeClass(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_NodeClass_i msg_browse_response_bs__p_NodeClass)
{
    assert(NULL != ppResNodeClass);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResNodeClass[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    assert(true ==
           util_NodeClass__B_to_C(msg_browse_response_bs__p_NodeClass,
                                  &ppResNodeClass[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]));
}

void msg_browse_response_bs__reset_ResponseBrowse_Res_NodeClass(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResNodeClass);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResNodeClass[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    ppResNodeClass[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri] = OpcUa_NodeClass_Unspecified;
}

void msg_browse_response_bs__set_ResponseBrowse_Res_NodeId(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_ExpandedNodeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    assert(NULL != ppResNodeId);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResNodeId[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    status = SOPC_ExpandedNodeId_Copy(&ppResNodeId[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                      (SOPC_ExpandedNodeId*) msg_browse_response_bs__p_ExpandedNodeId);
    assert(SOPC_STATUS_OK == status);
    /* Also update the current number of references recorded for this BrowseResult */
    /* TODO: this implementation counts on the order of the iterator, which must be increasing */
    assert(NULL != pnReferenceDescription);
    pnReferenceDescription[msg_browse_response_bs__p_bvi] = msg_browse_response_bs__p_bri;
}

void msg_browse_response_bs__set_ResponseBrowse_Res_TypeDefinition(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
    const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_TypeDefinition)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    assert(NULL != ppResTypeDefinition);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResTypeDefinition[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    /* TODO: Temporary check. HasTypeDefinition shall not be NULL by PRE here. */
    if (NULL == msg_browse_response_bs__p_TypeDefinition)
    {
        SOPC_ExpandedNodeId_Initialize(
            &ppResTypeDefinition[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]);
    }
    else
    {
        status =
            SOPC_ExpandedNodeId_Copy(&ppResTypeDefinition[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                     (SOPC_ExpandedNodeId*) msg_browse_response_bs__p_TypeDefinition);
        assert(SOPC_STATUS_OK == status);
    }
}

void msg_browse_response_bs__reset_ResponseBrowse_Res_TypeDefinition(
    const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
    const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResTypeDefinition);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResTypeDefinition[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= pnAllocReferenceDescription[msg_browse_response_bs__p_bvi]);
    SOPC_ExpandedNodeId_Clear(&ppResTypeDefinition[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]);
}

void msg_browse_response_bs__malloc_browse_response(const t_entier4 msg_browse_response_bs__p_nb_bvi,
                                                    t_bool* const msg_browse_response_bs__p_isallocated)
{
    /* Initialize first dimension of the arrays required by the BrowseResponse */
    if (msg_browse_response_bs__p_nb_bvi >= 0 &&
        msg_browse_response_bs__p_nb_bvi <
            INT32_MAX) // check msg_browse_response_bs__p_nb_bvi + 1 <= INT32_MAX to avoid overflow
    {
        *msg_browse_response_bs__p_isallocated = true;
        nBrowseResult = msg_browse_response_bs__p_nb_bvi;

        /* 1D arrays */
        /* pBrowseStatus */
        pBrowseStatus = calloc((size_t) nBrowseResult + 1, sizeof(SOPC_StatusCode));
        if (NULL == pBrowseStatus)
            *msg_browse_response_bs__p_isallocated = false;

        /* pnAllocReferenceDescription */
        pnAllocReferenceDescription = calloc((size_t) nBrowseResult + 1, sizeof(int32_t));
        if (NULL == pnAllocReferenceDescription)
            *msg_browse_response_bs__p_isallocated = false;

        /* pnReferenceDescription */
        /* Note: pnReferenceDescription must be initialized to 0 for each response */
        pnReferenceDescription = calloc((size_t) nBrowseResult + 1, sizeof(int32_t));
        if (NULL == pnReferenceDescription)
            *msg_browse_response_bs__p_isallocated = false;

        /* 2D arrays */
        /* 2D arrays are calloced, so that free is doable after partly failed alloc */
        /* ppResRefTypeId */
        ppResRefTypeId = calloc((size_t) nBrowseResult + 1, sizeof(SOPC_NodeId*));
        if (NULL == ppResRefTypeId)
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResForwards */
        ppResForwards = calloc((size_t) nBrowseResult + 1, sizeof(bool*));
        if (NULL == ppResForwards)
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResNodeId */
        ppResNodeId = calloc((size_t) nBrowseResult + 1, sizeof(SOPC_ExpandedNodeId*));
        if (NULL == ppResNodeId)
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResBrowseName */
        ppResBrowseName = calloc((size_t) nBrowseResult + 1, sizeof(SOPC_QualifiedName*));
        if (NULL == ppResBrowseName)
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResDisplayName */
        ppResDisplayName = calloc((size_t) nBrowseResult + 1, sizeof(SOPC_LocalizedText*));
        if (NULL == ppResDisplayName)
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResNodeClass */
        ppResNodeClass = calloc((size_t) nBrowseResult + 1, sizeof(OpcUa_NodeClass*));
        if (NULL == ppResNodeClass)
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResTypeDefinition */
        ppResTypeDefinition = calloc((size_t) nBrowseResult + 1, sizeof(SOPC_ExpandedNodeId*));
        if (NULL == ppResTypeDefinition)
            *msg_browse_response_bs__p_isallocated = false;

        /* Clean half-done mallocation */
        if (msg_browse_response_bs__p_isallocated == false)
            msg_browse_response_bs__free_browse_result();
    }
    else
    {
        *msg_browse_response_bs__p_isallocated = false;
    }
}

void msg_browse_response_bs__malloc_browse_result(const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
                                                  const t_entier4 msg_browse_response_bs__p_nb_bri,
                                                  t_bool* const msg_browse_response_bs__p_isallocated)
{
    int32_t j, bvi, nbri;

    bvi = msg_browse_response_bs__p_bvi;
    nbri = msg_browse_response_bs__p_nb_bri;
    *msg_browse_response_bs__p_isallocated = true;

    /* Initialize the ith BrowseResult of the BrowseResponse */
    /* 1D arrays */
    pnAllocReferenceDescription[bvi] = nbri;
    pnReferenceDescription[bvi] = 0;

    /* 2D arrays */
    /* ppResRefTypeId */
    if (nbri >= 0 && (uint64_t)(nbri + 1) * sizeof(SOPC_NodeId) <= SIZE_MAX &&
        (uint64_t)(nbri + 1) * sizeof(SOPC_ExpandedNodeId) <= SIZE_MAX &&
        (uint64_t)(nbri + 1) * sizeof(SOPC_QualifiedName) <= SIZE_MAX &&
        (uint64_t)(nbri + 1) * sizeof(SOPC_LocalizedText) <= SIZE_MAX &&
        (uint64_t)(nbri + 1) * sizeof(OpcUa_NodeClass) <= SIZE_MAX)
    {
        ppResRefTypeId[bvi] = malloc(sizeof(SOPC_NodeId) * ((size_t) nbri + 1));
        if (NULL == ppResRefTypeId[bvi])
            *msg_browse_response_bs__p_isallocated = false;
        for (j = 0; NULL != ppResRefTypeId[bvi] && j <= nbri; ++j)
            SOPC_NodeId_Initialize(&ppResRefTypeId[bvi][j]);

        /* ppResForwards */

        ppResForwards[bvi] = calloc((size_t) nbri + 1, sizeof(bool));
        if (NULL == ppResForwards[bvi])
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResNodeId */
        ppResNodeId[bvi] = malloc(sizeof(SOPC_ExpandedNodeId) * (size_t)(nbri + 1));
        if (NULL == ppResNodeId[bvi])
            *msg_browse_response_bs__p_isallocated = false;
        for (j = 0; NULL != ppResNodeId[bvi] && j <= nbri; ++j)
            SOPC_ExpandedNodeId_Initialize(&ppResNodeId[bvi][j]);

        /* ppResBrowseName */
        ppResBrowseName[bvi] = malloc(sizeof(SOPC_QualifiedName) * (size_t)(nbri + 1));
        if (NULL == ppResBrowseName[bvi])
            *msg_browse_response_bs__p_isallocated = false;
        for (j = 0; NULL != ppResBrowseName[bvi] && j <= nbri; ++j)
            SOPC_QualifiedName_Initialize(&ppResBrowseName[bvi][j]);

        /* ppResDisplayName */
        ppResDisplayName[bvi] = malloc(sizeof(SOPC_LocalizedText) * (size_t)(nbri + 1));
        if (NULL == ppResDisplayName[bvi])
            *msg_browse_response_bs__p_isallocated = false;
        for (j = 0; NULL != ppResDisplayName[bvi] && j <= nbri; ++j)
            SOPC_LocalizedText_Initialize(&ppResDisplayName[bvi][j]);

        /* ppResNodeClass */
        ppResNodeClass[bvi] = calloc((size_t) nbri + 1, sizeof(OpcUa_NodeClass));
        if (NULL == ppResNodeClass[bvi])
            *msg_browse_response_bs__p_isallocated = false;

        /* ppResTypeDefinition */
        ppResTypeDefinition[bvi] = malloc(sizeof(SOPC_ExpandedNodeId) * (size_t)(nbri + 1));
        if (NULL == ppResTypeDefinition[bvi])
            *msg_browse_response_bs__p_isallocated = false;
        for (j = 0; NULL != ppResTypeDefinition[bvi] && j <= nbri; ++j)
            SOPC_ExpandedNodeId_Initialize(&ppResTypeDefinition[bvi][j]);
    }
    else
    {
        // nbri + 1 not compatible with size_t type necessary for allocation
        *msg_browse_response_bs__p_isallocated = false;
    }

    /* Clean half-done mallocation */
    if (!msg_browse_response_bs__p_isallocated)
        msg_browse_response_bs__free_browse_result();
}

void msg_browse_response_bs__free_browse_result(void)
{
    int32_t i, j;

    if (nBrowseResult <= 0)
        return;

    /* 2D arrays */
    for (i = 0; i <= nBrowseResult; ++i)
    {
        for (j = 0; NULL != pnAllocReferenceDescription && j <= pnAllocReferenceDescription[i]; ++j)
        {
            if (NULL != ppResRefTypeId && NULL != ppResRefTypeId[i])
                SOPC_NodeId_Clear(&ppResRefTypeId[i][j]);
            if (NULL != ppResNodeId && NULL != ppResNodeId[i])
                SOPC_ExpandedNodeId_Clear(&ppResNodeId[i][j]);
            if (NULL != ppResBrowseName && NULL != ppResBrowseName[i])
                SOPC_QualifiedName_Clear(&ppResBrowseName[i][j]);
            if (NULL != ppResDisplayName && NULL != ppResDisplayName[i])
                SOPC_LocalizedText_Clear(&ppResDisplayName[i][j]);
            if (NULL != ppResTypeDefinition && NULL != ppResTypeDefinition[i])
                SOPC_ExpandedNodeId_Clear(&ppResTypeDefinition[i][j]);
        }
        if (NULL != ppResRefTypeId)
            free(ppResRefTypeId[i]);
        if (NULL != ppResForwards)
            free(ppResForwards[i]);
        if (NULL != ppResNodeId)
            free(ppResNodeId[i]);
        if (NULL != ppResBrowseName)
            free(ppResBrowseName[i]);
        if (NULL != ppResDisplayName)
            free(ppResDisplayName[i]);
        if (NULL != ppResNodeClass)
            free(ppResNodeClass[i]);
        if (NULL != ppResTypeDefinition)
            free(ppResTypeDefinition[i]);
    }
    free(ppResRefTypeId);
    free(ppResForwards);
    free(ppResNodeId);
    free(ppResBrowseName);
    free(ppResDisplayName);
    free(ppResNodeClass);
    free(ppResTypeDefinition);

    /* 1D arrays */
    free(pBrowseStatus);
    free(pnAllocReferenceDescription);
    free(pnReferenceDescription);

    /* Clear the pointers */
    msg_browse_response_bs__INITIALISATION();
}

void msg_browse_response_bs__write_BrowseResponse_msg_out(const constants__t_msg_i msg_browse_response_bs__p_msg_out,
                                                          t_bool* const msg_browse_response_bs__p_isvalid)
{
    OpcUa_BrowseResponse* pResp = (OpcUa_BrowseResponse*) msg_browse_response_bs__p_msg_out;
    OpcUa_BrowseResult* lbr;
    OpcUa_ReferenceDescription* lrd;
    int32_t i, j, nRefs;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* TODO: Verify (in previous code) that msg_browse_response_bs__p_msg_out is initialized correctly */
    *msg_browse_response_bs__p_isvalid = false;

    /* You must copy the local results, because of the recursive free of BrowseResponse */
    pResp->NoOfResults = nBrowseResult;
    if (nBrowseResult >= 0 && (uint64_t) SIZE_MAX / sizeof(OpcUa_BrowseResult) >= (uint64_t) nBrowseResult)
    {
        lbr = (OpcUa_BrowseResult*) malloc(sizeof(OpcUa_BrowseResult) * (size_t) nBrowseResult);
    }
    else
    {
        lbr = NULL;
    }
    if (NULL == lbr)
        return;
    for (i = 0; i < nBrowseResult; ++i)
        OpcUa_BrowseResult_Initialize(&lbr[i]);
    pResp->Results = lbr; /* So that the half-allocated BrowseResults are freed */

    /* Fills the BrowseResults with local data */
    for (i = 0; i < nBrowseResult && i <= INT32_MAX; ++i)
    {
        lbr[i].StatusCode = pBrowseStatus[i + 1];
        SOPC_ByteString_Initialize(&lbr[i].ContinuationPoint);
        /* Also a copy, because of the recursive clear */
        nRefs = pnReferenceDescription[i + 1];
        lbr[i].NoOfReferences = nRefs;
        if (nRefs >= 0 && (uint64_t) SIZE_MAX / sizeof(OpcUa_ReferenceDescription) >= (uint64_t) nRefs)
        {
            lrd = (OpcUa_ReferenceDescription*) malloc(sizeof(OpcUa_ReferenceDescription) * (size_t) nRefs);
        }
        else
        {
            lrd = NULL;
        }
        if (NULL == lrd)
            return;
        for (j = 0; j < nRefs && j <= INT32_MAX; ++j)
        {
            OpcUa_ReferenceDescription_Initialize(&lrd[j]);
        }
        lbr[i].References = lrd;

        for (j = 0; j < nRefs && j < INT32_MAX; ++j)
        {
            status = SOPC_NodeId_Copy(&lrd[j].ReferenceTypeId, &ppResRefTypeId[i + 1][j + 1]);
            if (SOPC_STATUS_OK != status)
                return;

            lrd[j].IsForward = ppResForwards[i + 1][j + 1];
            status = SOPC_ExpandedNodeId_Copy(&lrd[j].NodeId, &ppResNodeId[i + 1][j + 1]);
            if (SOPC_STATUS_OK != status)
                return;

            status = SOPC_QualifiedName_Copy(&lrd[j].BrowseName, &ppResBrowseName[i + 1][j + 1]);
            if (SOPC_STATUS_OK != status)
                return;

            status = SOPC_LocalizedText_Copy(&lrd[j].DisplayName, &ppResDisplayName[i + 1][j + 1]);
            if (SOPC_STATUS_OK != status)
                return;
            lrd[j].NodeClass = ppResNodeClass[i + 1][j + 1];

            status = SOPC_ExpandedNodeId_Copy(&lrd[j].TypeDefinition, &ppResTypeDefinition[i + 1][j + 1]);
            if (SOPC_STATUS_OK != status)
                return;
        }
    }

    pResp->NoOfDiagnosticInfos = 0;
    pResp->DiagnosticInfos = NULL;

    *msg_browse_response_bs__p_isvalid = true;
}
