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

MACHINE
    browse_treatment_result_bs

SEES
    constants,
    constants_statuscodes_bs

DEFINITIONS

    d_variables_out ==
        out_BrowseResult_BrowseStatus,
        out_BrowseResult_nb_References,
        out_BrowseResultReferences,
        out_BrowseResult_ReferenceTypeId,
        out_BrowseResult_isForward,
        out_BrowseResult_NodeId,
        out_BrowseResult_BrowseName,
        out_BrowseResult_DisplayName,
        out_BrowseResult_NodeClass,
        out_BrowseResult_TypeDefinition,
        max_nb_references
    ;

    d_variables_out_status ==
        isBrowseResultAllocated
    ;

    d_variables_mask ==
        mask_hasReferenceType,
        mask_hasIsForward,
        mask_hasNodeClass,
        mask_hasBrowseName,
        mask_hasDisplayName,
        mask_hasTypeDefinition
    ;

    d_inv_out ==
        out_BrowseResult_BrowseStatus    : t_StatusCode_i             &
        out_BrowseResult_nb_References   : NAT                        &
        out_BrowseResultReferences       : t_BrowseResultReferences_i &
        out_BrowseResult_ReferenceTypeId : seq(t_NodeId_i)         &
        out_BrowseResult_isForward       : seq(BOOL)               &
        out_BrowseResult_NodeId          : seq(t_ExpandedNodeId)   &
        out_BrowseResult_BrowseName      : seq(t_QualifiedName_i)  &
        out_BrowseResult_DisplayName     : seq(t_LocalizedText_i)  &
        out_BrowseResult_NodeClass       : seq(t_NodeClass_i)      &
        out_BrowseResult_TypeDefinition  : seq(t_ExpandedNodeId_i) &
        size(out_BrowseResult_ReferenceTypeId) = out_BrowseResult_nb_References &
        size(out_BrowseResult_isForward)       = out_BrowseResult_nb_References &
        size(out_BrowseResult_NodeId)          = out_BrowseResult_nb_References &
        size(out_BrowseResult_BrowseName)      = out_BrowseResult_nb_References &
        size(out_BrowseResult_DisplayName)     = out_BrowseResult_nb_References &
        size(out_BrowseResult_NodeClass)       = out_BrowseResult_nb_References &
        size(out_BrowseResult_TypeDefinition)  = out_BrowseResult_nb_References &

        max_nb_references                : NAT
    ;

    d_inv_out_status ==
        isBrowseResultAllocated : BOOL
    ;

    d_inv_mask ==
        mask_hasReferenceType <: t_BrowseResultMask_i &
        mask_hasIsForward <: t_BrowseResultMask_i &
        mask_hasNodeClass <: t_BrowseResultMask_i &
        mask_hasBrowseName <: t_BrowseResultMask_i &
        mask_hasDisplayName <: t_BrowseResultMask_i &
        mask_hasTypeDefinition <: t_BrowseResultMask_i
    ;

    d_init_out ==
        d_variables_out
        :(
            out_BrowseResult_BrowseStatus    = c_StatusCode_indet             &
            out_BrowseResult_nb_References   = 0                              &
            out_BrowseResultReferences       = c_BrowseResultReferences_indet &
            out_BrowseResult_ReferenceTypeId = {}                         &
            out_BrowseResult_isForward       = {}                         &
            out_BrowseResult_NodeId          = {}                         &
            out_BrowseResult_BrowseName      = {}                         &
            out_BrowseResult_DisplayName     = {}                         &
            out_BrowseResult_NodeClass       = {}                         &
            out_BrowseResult_TypeDefinition  = {}                         &
            max_nb_references                = 0
        )
    ;

    d_init_out_status ==
        isBrowseResultAllocated :(isBrowseResultAllocated = FALSE)
    ;

    d_init_mask ==
        d_variables_mask :( d_inv_mask)

ABSTRACT_VARIABLES
    d_variables_out,
    d_variables_out_status,
    d_variables_mask

INVARIANT
    d_inv_out &
    d_inv_out_status &
    d_inv_mask

INITIALISATION
    d_init_out ||
    d_init_out_status ||
    d_init_mask

OPERATIONS

    p_alloc_bres <-- alloc_browse_result(p_maxResultRefs) =
    PRE
        isBrowseResultAllocated = FALSE         &
        p_maxResultRefs : NAT                   &
        p_maxResultRefs <= k_n_BrowseTarget_max
    THEN
        CHOICE
            /* Out of memory */
            p_alloc_bres := FALSE
        OR
            p_alloc_bres := TRUE ||
            d_variables_out,
            d_variables_out_status
            :(  d_inv_out &
                d_inv_out_status &

                isBrowseResultAllocated            = TRUE                       &
                max_nb_references                  = p_maxResultRefs            &
                out_BrowseResult_BrowseStatus      = e_sc_ok                    &
                out_BrowseResult_nb_References     = 0                          &
                out_BrowseResult_ReferenceTypeId   = {}                         &
                out_BrowseResult_isForward         = {}                         &
                out_BrowseResult_NodeId            = {}                         &
                out_BrowseResult_BrowseName        = {}                         &
                out_BrowseResult_DisplayName       = {}                         &
                out_BrowseResult_NodeClass         = {}                         &
                out_BrowseResult_TypeDefinition    = {}                         &
                out_BrowseResultReferences         : t_BrowseResultReferences
            )
        END
    END
    ;

    p_alloc_failed <-- setall_browse_result_reference_at (p_refIndex, p_refTypeId, p_isForward,
                                                          p_NodeId, p_BrowseName, p_DisplayName,
                                                          p_NodeClass, p_TypeDefinition) =
    PRE
        isBrowseResultAllocated = TRUE &
        p_refIndex : NAT1 &
        p_refIndex = out_BrowseResult_nb_References + 1 & /* Next unused index */
        p_refIndex <= max_nb_references &
        p_refTypeId : t_NodeId_i &
        p_isForward : BOOL &
        p_NodeId : t_ExpandedNodeId_i &
        p_NodeId : t_ExpandedNodeId &
        p_BrowseName : t_QualifiedName_i &
        p_DisplayName : t_LocalizedText_i &
        p_NodeClass : t_NodeClass_i &
        p_TypeDefinition : t_ExpandedNodeId_i
    THEN
        CHOICE
            p_alloc_failed := FALSE ||
            out_BrowseResult_nb_References,
            out_BrowseResult_ReferenceTypeId,
            out_BrowseResult_isForward,
            out_BrowseResult_NodeId,
            out_BrowseResult_BrowseName,
            out_BrowseResult_DisplayName,
            out_BrowseResult_NodeClass,
            out_BrowseResult_TypeDefinition
            :(  out_BrowseResult_nb_References   = p_refIndex &
                out_BrowseResult_ReferenceTypeId = out_BrowseResult_ReferenceTypeId$0 <- p_refTypeId &
                out_BrowseResult_isForward       = out_BrowseResult_isForward$0 <- p_isForward &
                out_BrowseResult_NodeId          = out_BrowseResult_NodeId$0 <- p_NodeId &
                out_BrowseResult_BrowseName      = out_BrowseResult_BrowseName$0 <- p_BrowseName &
                out_BrowseResult_DisplayName     = out_BrowseResult_DisplayName$0 <- p_DisplayName &
                out_BrowseResult_NodeClass       = out_BrowseResult_NodeClass$0 <- p_NodeClass &
                out_BrowseResult_TypeDefinition  = out_BrowseResult_TypeDefinition$0 <- p_TypeDefinition
            )
        OR
            p_alloc_failed := TRUE
        END
    END
    ;

    p_nb_references <-- get_browse_result_nb_references =
    PRE
        isBrowseResultAllocated = TRUE
    THEN
        p_nb_references := out_BrowseResult_nb_References
    END
    ;

    p_refTypeId, p_isForward, p_NodeId, p_BrowseName, p_DisplayName, p_NodeClass, p_TypeDefinition
      <-- getall_browse_result_reference_at (p_refIndex) =
    PRE
        isBrowseResultAllocated = TRUE &
        p_refIndex : NAT &
        p_refIndex : 1 .. out_BrowseResult_nb_References
    THEN
        p_refTypeId : ( p_refTypeId : t_NodeId_i & p_refTypeId = out_BrowseResult_ReferenceTypeId ( p_refIndex ) ) ||
        p_isForward := out_BrowseResult_isForward ( p_refIndex ) ||
        p_NodeId : ( p_NodeId : t_ExpandedNodeId_i & p_NodeId = out_BrowseResult_NodeId ( p_refIndex ) ) ||
        p_BrowseName := out_BrowseResult_BrowseName ( p_refIndex ) ||
        p_DisplayName := out_BrowseResult_DisplayName ( p_refIndex ) ||
        p_NodeClass := out_BrowseResult_NodeClass ( p_refIndex ) ||
        p_TypeDefinition := out_BrowseResult_TypeDefinition ( p_refIndex )
    END
    ;

    p_nb_references, p_browseResult <-- getall_and_move_browse_result =
    PRE
        isBrowseResultAllocated = TRUE
    THEN
        p_browseResult := out_BrowseResultReferences ||
        p_nb_references := out_BrowseResult_nb_References ||
        d_init_out ||
        d_init_out_status
    END
    ;

    clear_browse_result =
    BEGIN
        d_init_out ||
        d_init_out_status
    END
    ;

    // Note: avoid unused parameter warning in browse_treatment_i
    unused_browse_view (p_browseView) =
    PRE
        p_browseView : t_NodeId_i
    THEN
        skip
    END
    ;

    bres <-- is_ReferenceType_in_mask (p_resultMask) =
    PRE
        p_resultMask : t_BrowseResultMask_i
    THEN
        bres := bool(p_resultMask : mask_hasReferenceType)
    END
    ;

    bres <-- is_IsForward_in_mask (p_resultMask) =
    PRE
        p_resultMask : t_BrowseResultMask_i
    THEN
        bres := bool(p_resultMask : mask_hasIsForward)
    END
    ;

    bres <-- is_NodeClass_in_mask (p_resultMask) =
    PRE
        p_resultMask : t_BrowseResultMask_i
    THEN
        bres := bool(p_resultMask : mask_hasNodeClass)
    END
    ;

    bres <-- is_BrowseName_in_mask (p_resultMask) =
    PRE
        p_resultMask : t_BrowseResultMask_i
    THEN
        bres := bool(p_resultMask : mask_hasBrowseName)
    END
    ;

    bres <-- is_DisplayName_in_mask (p_resultMask) =
    PRE
        p_resultMask : t_BrowseResultMask_i
    THEN
        bres := bool(p_resultMask : mask_hasDisplayName)
    END
    ;

    bres <-- is_TypeDefinition_in_mask (p_resultMask) =
    PRE
        p_resultMask : t_BrowseResultMask_i
    THEN
        bres := bool(p_resultMask : mask_hasTypeDefinition)
    END
END
