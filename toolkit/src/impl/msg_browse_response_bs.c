/*
 *  Copyright (C) 2017 Systerel and others.
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

#include "sopc_types.h"


/* Message elements */
/* All arrays start at 1, to be directly compatible with B operations */
static int32_t               nBrowseResult;
static SOPC_StatusCode      *pBrowseStatus;
/*static  *pContinuationPoint;*/
static int32_t               nAllocReferenceDescription; /** Number of usable RefDescr per BrowseResult (number of allocation is nAlloc + 1 so that indices are B-compatible) */
static int32_t               *pnReferenceDescription; /** Number of used RefDescr by each BrowseResult */
static SOPC_NodeId          **ppResRefTypeId;
static bool                 **ppResForwards;
static SOPC_ExpandedNodeId  **ppResNodeId;
static SOPC_QualifiedName   **ppResBrowseName;
static SOPC_LocalizedText   **ppResDisplayName;
static OpcUa_NodeClass      **ppResNodeClass;
static SOPC_ExpandedNodeId  **ppResTypeDefinition;


/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_browse_response_bs__INITIALISATION(void)
{
    nBrowseResult               = 0;
    pBrowseStatus               = NULL;
    nAllocReferenceDescription  = 0;
    pnReferenceDescription      = NULL;
    ppResRefTypeId              = NULL;
    ppResForwards               = NULL;
    ppResNodeId                 = NULL;
    ppResBrowseName             = NULL;
    ppResDisplayName            = NULL;
    ppResNodeClass              = NULL;
    ppResTypeDefinition         = NULL;
}


/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_browse_response_bs__set_ResponseBrowse_BrowseStatus(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const t_bool msg_browse_response_bs__p_bool)
{
    assert(NULL != pBrowseStatus);
    /* TODO: this should raise a warning until type is changed */
    pBrowseStatus[msg_browse_response_bs__p_bvi] = !msg_browse_response_bs__p_bool;
}


void msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_Reference_i msg_browse_response_bs__p_ref)
{
    printf("Not implemented: msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint\n");
}


void msg_browse_response_bs__reset_ResponseBrowse_ContinuationPoint(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi)
{
    printf("Not implemented: msg_browse_response_bs__reset_ResponseBrowse_ContinuationPoint\n");
}


void msg_browse_response_bs__set_ResponseBrowse_Res_ReferenceTypeId(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_NodeId_i msg_browse_response_bs__p_NodeId)
{
    assert(NULL != ppResRefTypeId);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResRefTypeId[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    assert(STATUS_OK ==
           SOPC_NodeId_Copy(&ppResRefTypeId[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                            (SOPC_NodeId *)msg_browse_response_bs__p_NodeId));
}


void msg_browse_response_bs__set_ResponseBrowse_Res_Forwards(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const t_bool msg_browse_response_bs__p_bool)
{
    assert(NULL != ppResForwards);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResForwards[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    ppResForwards[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri] = msg_browse_response_bs__p_bool;
}


void msg_browse_response_bs__set_ResponseBrowse_Res_BrowseName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_QualifiedName_i msg_browse_response_bs__p_BrowseName)
{
    assert(NULL != ppResBrowseName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResBrowseName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    assert(STATUS_OK ==
           SOPC_QualifiedName_Copy(&ppResBrowseName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                   (SOPC_QualifiedName *)msg_browse_response_bs__p_BrowseName));
}


void msg_browse_response_bs__reset_ResponseBrowse_Res_BrowseName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResBrowseName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResBrowseName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    SOPC_QualifiedName_Clear(&ppResBrowseName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]);
}


void msg_browse_response_bs__set_ResponseBrowse_Res_DisplayName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_LocalizedText_i msg_browse_response_bs__p_DisplayName)
{
    assert(NULL != ppResDisplayName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResDisplayName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    assert(STATUS_OK ==
           SOPC_LocalizedText_Copy(&ppResDisplayName[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                   (SOPC_LocalizedText *)msg_browse_response_bs__p_DisplayName));
}


void msg_browse_response_bs__reset_ResponseBrowse_Res_DisplayName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResDisplayName);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResDisplayName[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
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
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
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
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    ppResNodeClass[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri] = OpcUa_NodeClass_Unspecified;
}


void msg_browse_response_bs__set_ResponseBrowse_Res_NodeId(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_ExpandedNodeId)
{
    assert(NULL != ppResNodeId);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResNodeId[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    assert(STATUS_OK ==
           SOPC_ExpandedNodeId_Copy(&ppResNodeId[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                    (SOPC_ExpandedNodeId *)msg_browse_response_bs__p_ExpandedNodeId));
}


void msg_browse_response_bs__set_ResponseBrowse_Res_TypeDefinition(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_TypeDefinition)
{
    assert(NULL != ppResTypeDefinition);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResTypeDefinition[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    assert(STATUS_OK ==
           SOPC_ExpandedNodeId_Copy(&ppResTypeDefinition[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri],
                                    (SOPC_ExpandedNodeId *)msg_browse_response_bs__p_TypeDefinition));
}


void msg_browse_response_bs__reset_ResponseBrowse_Res_TypeDefinition(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri)
{
    assert(NULL != ppResTypeDefinition);
    assert(msg_browse_response_bs__p_bvi <= nBrowseResult);
    assert(NULL != ppResTypeDefinition[msg_browse_response_bs__p_bvi]);
    assert(msg_browse_response_bs__p_bri <= nAllocReferenceDescription);
    SOPC_ExpandedNodeId_Clear(&ppResTypeDefinition[msg_browse_response_bs__p_bvi][msg_browse_response_bs__p_bri]);
}


void msg_browse_response_bs__malloc_browse_result(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const t_entier4 msg_browse_response_bs__p_nb_bri,
   t_bool * const msg_browse_response_bs__p_isallocated)
{
    int32_t i, j;

    *msg_browse_response_bs__p_isallocated = true;
    nBrowseResult               = msg_browse_response_bs__p_bvi;
    nAllocReferenceDescription  = msg_browse_response_bs__p_nb_bri;

    /* 1D arrays */
    /* pBrowseStatus */
    pBrowseStatus               = malloc(sizeof(SOPC_StatusCode)*(nBrowseResult+1));
    if(NULL == pBrowseStatus)
        *msg_browse_response_bs__p_isallocated = false;

    /* pnReferenceDescription */
    pnReferenceDescription      = malloc(sizeof(int32_t)*(nBrowseResult+1));
    if(NULL == pnReferenceDescription)
        *msg_browse_response_bs__p_isallocated = false;

    /* 2D arrays */
    /* ppResRefTypeId */
    ppResRefTypeId              = malloc(sizeof(SOPC_NodeId *)*(nBrowseResult+1));
    if(NULL == ppResRefTypeId)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResRefTypeId[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResRefTypeId[i] = malloc(sizeof(SOPC_NodeId)*(nAllocReferenceDescription+1));
            if(NULL == ppResRefTypeId[i])
                *msg_browse_response_bs__p_isallocated = false;
            for(j=0; NULL != ppResRefTypeId[i] && j <= nAllocReferenceDescription; ++j)
                SOPC_NodeId_Initialize(&ppResRefTypeId[i][j]);
        }
    }

    /* ppResForwards */
    ppResForwards               = malloc(sizeof(bool *)*(nBrowseResult+1));
    if(NULL == ppResForwards)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResForwards[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResForwards[i] = malloc(sizeof(bool)*(nAllocReferenceDescription+1));
            if(NULL == ppResForwards[i])
                *msg_browse_response_bs__p_isallocated = false;
        }
    }

    /* ppResNodeId */
    ppResNodeId                 = malloc(sizeof(SOPC_ExpandedNodeId *)*(nBrowseResult+1));
    if(NULL == ppResNodeId)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResNodeId[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResNodeId[i] = malloc(sizeof(SOPC_ExpandedNodeId)*(nAllocReferenceDescription+1));
            if(NULL == ppResNodeId[i])
                *msg_browse_response_bs__p_isallocated = false;
            for(j=0; NULL != ppResNodeId[i] && j <= nAllocReferenceDescription; ++j)
                SOPC_ExpandedNodeId_Initialize(&ppResNodeId[i][j]);
        }
    }

    /* ppResBrowseName */
    ppResBrowseName             = malloc(sizeof(SOPC_QualifiedName *)*(nBrowseResult+1));
    if(NULL == ppResBrowseName)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResBrowseName[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResBrowseName[i] = malloc(sizeof(SOPC_QualifiedName)*(nAllocReferenceDescription+1));
            if(NULL == ppResBrowseName[i])
                *msg_browse_response_bs__p_isallocated = false;
            for(j=0; NULL != ppResBrowseName[i] && j <= nAllocReferenceDescription; ++j)
                SOPC_QualifiedName_Initialize(&ppResBrowseName[i][j]);
        }
    }

    /* ppResDisplayName */
    ppResDisplayName            = malloc(sizeof(SOPC_LocalizedText *)*(nBrowseResult+1));
    if(NULL == ppResDisplayName)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResDisplayName[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResDisplayName[i] = malloc(sizeof(SOPC_LocalizedText)*(nAllocReferenceDescription+1));
            if(NULL == ppResDisplayName[i])
                *msg_browse_response_bs__p_isallocated = false;
            for(j=0; NULL != ppResDisplayName[i] && j <= nAllocReferenceDescription; ++j)
                SOPC_LocalizedText_Initialize(&ppResDisplayName[i][j]);
        }
    }

    /* ppResNodeClass */
    ppResNodeClass              = malloc(sizeof(OpcUa_NodeClass *)*(nBrowseResult+1));
    if(NULL == ppResNodeClass)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResNodeClass[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResNodeClass[i] = malloc(sizeof(OpcUa_NodeClass)*(nAllocReferenceDescription+1));
            if(NULL == ppResNodeClass[i])
                *msg_browse_response_bs__p_isallocated = false;
        }
    }

    /* ppResTypeDefinition */
    ppResTypeDefinition         = malloc(sizeof(SOPC_ExpandedNodeId *)*(nBrowseResult+1));
    if(NULL == ppResTypeDefinition)
        *msg_browse_response_bs__p_isallocated = false;
    else {
        ppResTypeDefinition[0] = NULL;
        for(i=1; i<=nAllocReferenceDescription; ++i) {
            ppResTypeDefinition[i] = malloc(sizeof(SOPC_ExpandedNodeId)*(nAllocReferenceDescription+1));
            if(NULL == ppResTypeDefinition[i])
                *msg_browse_response_bs__p_isallocated = false;
            for(j=0; NULL != ppResTypeDefinition[i] && j <= nAllocReferenceDescription; ++j)
                SOPC_ExpandedNodeId_Initialize(&ppResTypeDefinition[i][j]);
        }
    }

    /* Clean half-done mallocation */
    if(! msg_browse_response_bs__p_isallocated)
        msg_browse_response_bs__free_browse_result();
}


void msg_browse_response_bs__free_browse_result(void)
{
    int32_t i;

    /* 1D arrays */
    free(pBrowseStatus);
    free(pnReferenceDescription);

    /* 2D arrays */
    for(i=0; NULL != ppResRefTypeId && i<=nAllocReferenceDescription; ++i)
        free(ppResRefTypeId[i]);
    free(ppResRefTypeId);
    for(i=0; NULL != ppResForwards && i<=nAllocReferenceDescription; ++i)
        free(ppResForwards[i]);
    free(ppResForwards);
    for(i=0; NULL != ppResNodeId && i<=nAllocReferenceDescription; ++i)
        free(ppResNodeId[i]);
    free(ppResNodeId);
    for(i=0; NULL != ppResBrowseName && i<=nAllocReferenceDescription; ++i)
        free(ppResBrowseName[i]);
    free(ppResBrowseName);
    for(i=0; NULL != ppResDisplayName && i<=nAllocReferenceDescription; ++i)
        free(ppResDisplayName[i]);
    free(ppResDisplayName);
    for(i=0; NULL != ppResNodeClass && i<=nAllocReferenceDescription; ++i)
        free(ppResNodeClass[i]);
    free(ppResNodeClass);
    for(i=0; NULL != ppResTypeDefinition && i<=nAllocReferenceDescription; ++i)
        free(ppResTypeDefinition[i]);
    free(ppResTypeDefinition);

    /* Clear the pointers */
    msg_browse_response_bs__INITIALISATION();
}


void msg_browse_response_bs__write_BrowseResponse_msg_out(
   const constants__t_msg_i msg_browse_response_bs__p_msg_out,
   t_bool * const msg_browse_response_bs__p_isvalid)
{
    OpcUa_BrowseResponse *pResp = (OpcUa_BrowseResponse *)msg_browse_response_bs__p_msg_out;
    OpcUa_BrowseResult *lbr;
    OpcUa_ReferenceDescription *lrd;
    int32_t i, j, nRefs;

    /* TODO: Verify (in previous code) that msg_browse_response_bs__p_msg_out is initialized correctly */
    *msg_browse_response_bs__p_isvalid = false;

    /* You must copy the local results, because of the recursive free of BrowseResponse */
    pResp->NoOfResults = nBrowseResult;
    lbr = (OpcUa_BrowseResult *)malloc(sizeof(OpcUa_BrowseResult)*nBrowseResult);
    if(NULL == lbr)
        return;
    for(i=0; i<nBrowseResult; ++i)
        OpcUa_BrowseResult_Initialize(&lbr[i]);
    pResp->Results = lbr; /* So that the half-allocated BrowseResults are freed */

    /* Fills the BrowseResults with local data */
    for(i=0; i<nBrowseResult; ++i)
    {
        lbr[i].StatusCode = pBrowseStatus[i+1];
        SOPC_ByteString_Initialize(&lbr[i].ContinuationPoint);
        /* Also a copy, because of the recursive clear */
        nRefs = pnReferenceDescription[i+1];
        lbr[i].NoOfReferences = nRefs;
        lrd = (OpcUa_ReferenceDescription *)malloc(sizeof(OpcUa_ReferenceDescription)*nRefs);
        if(NULL == lrd)
            return;
        for(j=0; j<nRefs; ++j)
            OpcUa_ReferenceDescription_Initialize(&lrd[j]);
        lbr[i].References = lrd;

        for(j=0; j<nRefs; ++j)
        {
            if(STATUS_OK != SOPC_NodeId_Copy(&lrd[j].ReferenceTypeId, &ppResRefTypeId[i+1][j+1]))
                return;
            lrd[j].IsForward = ppResForwards[i+1][j+1];
            if(STATUS_OK != SOPC_ExpandedNodeId_Copy(&lrd[j].NodeId, &ppResNodeId[i+1][j+1]))
                return;
            if(STATUS_OK != SOPC_QualifiedName_Copy(&lrd[j].BrowseName, &ppResBrowseName[i+1][j+1]))
                return;
            if(STATUS_OK != SOPC_LocalizedText_Copy(&lrd[j].DisplayName, &ppResDisplayName[i+1][j+1]))
                return;
            lrd[j].NodeClass = ppResNodeClass[i+1][j+1];
            if(STATUS_OK != SOPC_ExpandedNodeId_Copy(&lrd[j].TypeDefinition, &ppResTypeDefinition[i+1][j+1]))
                return;
        }

    }

    pResp->NoOfDiagnosticInfos = 0;
    pResp->DiagnosticInfos = NULL;

    *msg_browse_response_bs__p_isvalid = true;
}

