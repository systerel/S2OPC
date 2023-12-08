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
    translate_browse_path_result

SEES
    address_space_itf,
    constants,
    constants_statuscodes_bs,
    msg_translate_browse_path_bs,
    translate_browse_path_source

DEFINITIONS
    d_variables ==
        BrowsePathResult,
        BrowsePathRemainingNodeId,
        BrowsePathRemainingIndex
        ;

    d_var_browse ==
        BrowseContext_MaxNbTargets,
        isBrowseValueContextDefined,
        isBrowseResultComputed
        ;

    d_invariant ==
        BrowsePathResult : seq(t_ExpandedNodeId_i) &
        BrowsePathRemainingNodeId : seq(t_ExpandedNodeId_i) &
        BrowsePathRemainingIndex  : seq(NAT1)
        ;

    d_inv_browse ==
        BrowseContext_MaxNbTargets : NAT &
        isBrowseValueContextDefined   : BOOL &
        isBrowseResultComputed        : BOOL
        ;

    d_init_browse ==
        d_var_browse:(
            BrowseContext_MaxNbTargets = 0 &
            isBrowseValueContextDefined = FALSE &
            isBrowseResultComputed      = FALSE
        )

ABSTRACT_VARIABLES
    d_variables,
    d_var_browse

INVARIANT
    d_invariant &
    d_inv_browse

INITIALISATION
    BrowsePathResult := {} ||
    BrowsePathRemainingNodeId := {} ||
    BrowsePathRemainingIndex  := {} ||

    d_init_browse

OPERATIONS

    /* Fill BrowsePathResult with filtered browse result node
       Node in BrowsePathResult are a copy an should be freed
    */
    statusCode_operation <-- treat_one_relative_path_element(relativePathElt, path_index) =
    PRE
        IsDecoded = TRUE &
        isBrowseValueContextDefined = FALSE   &
        relativePathElt : t_RelativePathElt_i &
        relativePathElt : t_RelativePathElt   &
        relativePathElt : s_RelativePathElt   &
        path_index : NAT1
    THEN
        statusCode_operation,
        BrowsePathResult,
        BrowsePathRemainingNodeId,
        BrowsePathRemainingIndex
        :(

            BrowsePathResult : seq(t_ExpandedNodeId_i) &
            c_ExpandedNodeId_indet /: ran(BrowsePathResult) &
            BrowsePathRemainingNodeId : seq(t_ExpandedNodeId_i) &
            BrowsePathRemainingIndex  : seq(NAT1) &

            size(BrowsePathResult) <= k_n_BrowsePathResPerElt_max &
            size(BrowsePathRemainingNodeId) <= k_n_BrowsePathResPerElt_max  &
            size(BrowsePathRemainingNodeId)  = size(BrowsePathRemainingIndex) &

            statusCode_operation : t_StatusCode_i &
            statusCode_operation : {e_sc_ok,
                e_sc_bad_browse_name_invalid,
                e_sc_bad_no_match,
                e_sc_bad_node_id_unknown,
                e_sc_bad_nothing_to_do,
                e_sc_bad_out_of_memory,
                e_sc_bad_query_too_complex,
                e_sc_uncertain_reference_out_of_server} &

            ( statusCode_operation : { e_sc_ok, e_sc_uncertain_reference_out_of_server }
                =>
                /* all target in BrowsePathResult has browse name equal to TargetName and
                   there is a relation between this target and at least one source through Reference_TargetNode/RefIndex_Reference */
                !(target).( target :ran(BrowsePathResult) =>
                    /* 1. TargetName in relativePathElt is equals to Browse name of target */
                    RelativePathElt_TargetName(relativePathElt) = a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(target))) &
                    /* 2. there is a source which is related to target */
                    #(source).(source : ran(BrowsePathSource) & card ( { lnode, lind | lnode = a_NodeId~(source) &
                        lind : 1..Node_RefIndexEnd(a_NodeId~(source)) &
                        target = Reference_TargetNode(RefIndex_Reference(lnode |-> lind)) &
                        ( RelativePathElt_ReferenceTypeId(relativePathElt) /= c_NodeId_indet
                          =>
                          /* TODO: use isReverse */
                          RelativePathElt_ReferenceTypeId(relativePathElt) = Reference_ReferenceType(RefIndex_Reference(lnode |-> lind))
                        ) &
                        RelativePathElt_IsInverse(relativePathElt) /= Reference_IsForward(RefIndex_Reference(lnode |-> lind))
                    } ) > 0 )
                )
            ) &
            ( statusCode_operation = e_sc_uncertain_reference_out_of_server
                =>
                size(BrowsePathRemainingNodeId) > 0
            )
        )
    END
    ;

    /* Operations manipulating BrowsePathResult and BrowsePathRemaining */

    free_BrowsePathResult =
    BEGIN
        BrowsePathResult := {}
    END
    ;

    free_BrowsePathRemaining =
    BEGIN
        BrowsePathRemainingNodeId := {} ||
        BrowsePathRemainingIndex  := {}
    END
    ;


    /* PROMOTED OPERATIONS */
    /* Promoted from browse_path_result */
    res <-- get_BrowsePathResultSize =
    BEGIN
        res := size(BrowsePathResult)
    END
    ;

    nodeId <-- get_BrowsePathResult (index) =
    PRE
        index : NAT1 &
        index <= size(BrowsePathResult)
    THEN
        nodeId := BrowsePathResult (index)
    END
    ;

    res <-- get_BrowsePathRemainingSize =
    BEGIN
        res := size(BrowsePathRemainingNodeId)
    END
    ;


    nodeId, remainingIndex <-- get_BrowsePathRemaining (index) =
    PRE
        index : NAT1 &
        index <= size(BrowsePathRemainingNodeId)
    THEN
        nodeId := BrowsePathRemainingNodeId (index) ||
        remainingIndex := BrowsePathRemainingIndex(index)
    END
    ;

    /* Promoted from browse_treatment */
    p_service_StatusCode <-- set_browse_value_context(p_session, p_maxTargetRef, p_browseView,
                                                      p_nodeId, p_browseDirection, p_referenceType,
                                                      p_includeSubtypes, p_nodeClassMask, p_resultMask,
                                                      p_autoReleaseCP) =
    PRE
        isBrowseValueContextDefined = FALSE         &
        p_session : t_session_i                     &
        p_maxTargetRef : NAT                        &
        p_browseView : t_NodeId_i                   &
        p_nodeId : t_NodeId_i                       &
        p_nodeId : t_NodeId                         &
        p_browseDirection : t_BrowseDirection_i     &
        p_browseDirection : t_BrowseDirection       &
        p_referenceType : t_NodeId_i                &
        p_nodeClassMask : t_BrowseNodeClassMask_i   &
        p_resultMask    : t_BrowseResultMask_i      &
        p_autoReleaseCP : BOOL                      &
        p_includeSubtypes : BOOL
    THEN
        BrowseContext_MaxNbTargets,
        p_service_StatusCode,
        isBrowseValueContextDefined
        :(
                p_service_StatusCode : {e_sc_ok, e_sc_bad_out_of_memory} &
                (p_service_StatusCode = e_sc_ok =>
                 BrowseContext_MaxNbTargets = p_maxTargetRef &
                 isBrowseValueContextDefined = TRUE) &
                (p_service_StatusCode /= e_sc_ok =>
                 isBrowseValueContextDefined = FALSE)
        )
    END
    ;

    p_service_StatusCode <-- set_browse_value_context_from_continuation_point(p_session, p_continuationPointId) =
    PRE
        isBrowseValueContextDefined = FALSE &
        p_session : t_session_i &
        p_session : t_session &
        p_continuationPointId : t_ContinuationPointId_i
    THEN
        p_service_StatusCode,
        BrowseContext_MaxNbTargets,
        isBrowseValueContextDefined
        :(  p_service_StatusCode : {e_sc_ok, e_sc_bad_continuation_point_invalid,
                                    e_sc_bad_out_of_memory} &
            BrowseContext_MaxNbTargets : NAT &
            isBrowseValueContextDefined = bool(p_service_StatusCode = e_sc_ok)
        )
    END
    ;

    clear_browse_value_context =
    BEGIN
        BrowseContext_MaxNbTargets := 0 ||
        isBrowseValueContextDefined := FALSE
    END
    ;

    p_serviceStatusCode, p_continuationPointId, p_nbReferences <-- compute_browse_result =
    PRE
        isBrowseValueContextDefined = TRUE &
        isBrowseResultComputed = FALSE
    THEN
        p_serviceStatusCode,
        p_continuationPointId,
        p_nbReferences
        :(
          p_serviceStatusCode : t_StatusCode_i &
          p_serviceStatusCode : t_StatusCode   &
          p_serviceStatusCode : {e_sc_ok, e_sc_bad_out_of_memory, e_sc_bad_node_id_unknown,
                                   e_sc_bad_reference_type_id_invalid, e_sc_bad_no_continuation_points,
                                   e_sc_bad_view_id_unknown} & /* Add other bad view statuses when treated */
          p_nbReferences : NAT &
          p_nbReferences <= k_n_BrowseTarget_max &
          p_continuationPointId : t_ContinuationPointId_i &
          (p_serviceStatusCode /= e_sc_ok => p_continuationPointId = c_ContinuationPointId_indet) &
          isBrowseResultComputed = bool(p_serviceStatusCode = e_sc_ok or p_serviceStatusCode = e_sc_bad_no_continuation_points)
        )
    END
    ;

    bres <-- release_continuation_point (p_session, p_continuationPointId) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_continuationPointId : t_ContinuationPointId_i
    THEN
        bres :: BOOL
    END
    ;

    set_session_closed(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        skip
    END
    ;

    continuation_points_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    p_nb_references, p_browseResult <-- getall_and_move_browse_result =
    PRE
        isBrowseResultComputed = TRUE
    THEN
        isBrowseResultComputed,
        p_nb_references
        :(  isBrowseResultComputed = FALSE &
            p_nb_references : NAT &
            p_nb_references <= k_n_BrowseTarget_max) ||
        p_browseResult :: t_BrowseResultReferences_i
    END

END
