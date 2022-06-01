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
    translate_browse_path

SEES
    address_space_itf,
    constants,
    constants_statuscodes_bs,
    message_in_bs


DEFINITIONS
    d_variables_in ==
        IsDecoded,
        BrowsePaths,
        BrowsePath_StartingNode,
        BrowsePath_RelativePath,

        RelativePath_RelativePathElt,

        RelativePathElt_ReferenceTypeId,
        RelativePathElt_IsInverse,
        RelativePathElt_IncludedSubtypes,
        RelativePathElt_TargetName
        ;

    d_variables_out==
        BrowsePath_Res_NbTargetMax,
        BrowsePath_Res_StatusCode,
        BrowsePath_Res_Target_Node,
        BrowsePath_Res_Target_Index
        ;

    d_variables_browse ==
    BrowseContext_MaxNbTargets,
    isBrowseValueContextDefined,
    isBrowseResultComputed
    ;

    d_inv_in ==
        IsDecoded                      : BOOL                                 &
           /* BrowsePath */
        BrowsePaths                    <: t_BrowsePath                        &
        BrowsePaths                    = cast_t_BrowsePath[1..card(BrowsePaths)] &
        BrowsePath_StartingNode        : t_BrowsePath +-> t_NodeId            &
        BrowsePath_RelativePath        : t_BrowsePath +-> t_RelativePath      &
        dom(BrowsePath_StartingNode)   = BrowsePaths &
        dom(BrowsePath_RelativePath)   = BrowsePaths &

        /* RelativePath */
        RelativePath_RelativePathElt   :  t_RelativePath +-> seq(t_RelativePathElt) &

        /* RelativePathElt */
        RelativePathElt_ReferenceTypeId   : t_RelativePathElt +-> t_NodeId &
        RelativePathElt_IsInverse         : t_RelativePathElt +-> BOOL     &
        RelativePathElt_IncludedSubtypes  : t_RelativePathElt +-> BOOL     &
        RelativePathElt_TargetName        : t_RelativePathElt +-> t_QualifiedName
    ;

    d_inv_out ==
        BrowsePath_Res_NbTargetMax       : t_BrowsePath +-> NAT                 &
        BrowsePath_Res_StatusCode        : t_BrowsePath +-> t_StatusCode           &
        BrowsePath_Res_Target_Node       : t_BrowsePath +-> seq(t_ExpandedNodeId)  &
        BrowsePath_Res_Target_Index      : t_BrowsePath +-> seq(NAT) &

        /* Same dom for all result arrays */
        dom(BrowsePath_Res_NbTargetMax) = dom(BrowsePath_Res_StatusCode)   &
        dom(BrowsePath_Res_NbTargetMax) = dom(BrowsePath_Res_Target_Node)  &
        dom(BrowsePath_Res_NbTargetMax) = dom(BrowsePath_Res_Target_Index) &
        /* The target arrays of each Path have same dom
           and max size
        */
        !(browsePath).(browsePath : dom(BrowsePath_Res_Target_Node)
            =>
            size(BrowsePath_Res_Target_Node(browsePath)) = size(BrowsePath_Res_Target_Index(browsePath)) &
            size(BrowsePath_Res_Target_Node(browsePath)) <= BrowsePath_Res_NbTargetMax(browsePath)
         )
         ;


    d_inv_browse ==
    BrowseContext_MaxNbTargets : NAT &
    isBrowseValueContextDefined   : BOOL &
    isBrowseResultComputed        : BOOL
    ;

    d_init ==
    d_variables_in,
    d_variables_out
        :(
            BrowsePaths                   = {} &
            BrowsePath_StartingNode       = {} &
            BrowsePath_RelativePath       = {} &

            /* RelativePath */
            RelativePath_RelativePathElt     = {} &

            /* RelativePathElt */
            RelativePathElt_ReferenceTypeId  = {} &
            RelativePathElt_IsInverse        = {} &
            RelativePathElt_IncludedSubtypes = {} &
            RelativePathElt_TargetName       = {} &

            /* Result */
            BrowsePath_Res_NbTargetMax  = {} &
            BrowsePath_Res_StatusCode   = {} &
            BrowsePath_Res_Target_Node  = {} &
            BrowsePath_Res_Target_Index = {}

    )
    ;

    d_init_browse ==
    d_variables_browse
    :(
        BrowseContext_MaxNbTargets = 0 &
        isBrowseValueContextDefined = FALSE &
        isBrowseResultComputed      = FALSE
    )

ABSTRACT_VARIABLES
    d_variables_in,
    d_variables_out,
    d_variables_browse

INVARIANT
    d_inv_in &
    d_inv_out &
    d_inv_browse

INITIALISATION
    d_init ||
    d_init_browse

OPERATIONS

    StatusCode_service <-- init_translate_browse_paths_request(req_msg) =
    PRE
        req_msg : t_msg_i &
        req_msg : t_msg   &
        req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_translate_browse_paths_to_node_ids_req
    THEN
        StatusCode_service,
        d_variables_in,
        d_variables_out
        :(
            StatusCode_service : {e_sc_ok, e_sc_bad_too_many_ops, e_sc_bad_nothing_to_do} &
            ( StatusCode_service = e_sc_ok
                =>
                IsDecoded = TRUE &
                card(BrowsePaths) > 0 &
                card(BrowsePath_Res_NbTargetMax) = card(BrowsePaths)
            ) &
            ( StatusCode_service = e_sc_bad_nothing_to_do
                =>
                IsDecoded = TRUE &
                card(BrowsePaths) = 0
            ) &
            ( StatusCode_service = e_sc_bad_too_many_ops
                =>
                IsDecoded = FALSE
            ) &
            d_inv_in
        )
    END
    ;

    statusCode <-- alloc_browse_path_result =
    PRE
        card(BrowsePaths) > 0
    THEN
        d_variables_out,
        statusCode
        :(
          statusCode : t_StatusCode_i &
          statusCode : t_StatusCode   &
          d_inv_out &

          (statusCode = e_sc_ok =>
              (dom(BrowsePath_Res_StatusCode)   = BrowsePaths &
               dom(BrowsePath_Res_Target_Node)  = BrowsePaths &
               dom(BrowsePath_Res_Target_Index) = BrowsePaths)
           )
        )
    END
    ;

    treat_one_translate_browse_path(browsePath) =
    PRE
        browsePath : t_BrowsePath_i &
        browsePath : BrowsePaths
    THEN
        d_variables_out
        :(
            BrowsePath_Res_StatusCode(browsePath) : {e_sc_ok, e_sc_uncertain_reference_out_of_server,
                e_sc_bad_nothing_to_do, e_sc_bad_node_id_unknown,
                e_sc_bad_query_too_complex, e_sc_bad_no_match, e_sc_bad_too_many_matches} &
            (BrowsePath_Res_StatusCode(browsePath) = e_sc_ok =>
                ! (ind).(ind : dom(BrowsePath_Res_Target_Node(browsePath))
                    =>
                    /* Target is local and its browse name is equal to the last target name in the request */
                    BrowsePath_Res_Target_Node(browsePath)(ind) : dom(conv_ExpandedNodeId_NodeId) &
                    a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(BrowsePath_Res_Target_Node(browsePath)(ind)))) =
                    RelativePathElt_TargetName(last(RelativePath_RelativePathElt(BrowsePath_RelativePath(browsePath)))) &
                    BrowsePath_Res_Target_Index(browsePath)(ind) = k_n_noRemainingPathIndex)
            ) &

            /* e_sc_uncertain_reference_out_of_server is returned if there is at least one match which is not local.
               Other matches are locals. */
            (BrowsePath_Res_StatusCode(browsePath) = e_sc_uncertain_reference_out_of_server =>
                ! (ind).(ind : dom(BrowsePath_Res_Target_Node(browsePath))
                    =>
                    (
                        /* If target is local, its browse name is equal to the last target name in the request */
                        BrowsePath_Res_Target_Node(browsePath)(ind) : dom(conv_ExpandedNodeId_NodeId)
                        =>
                        a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(BrowsePath_Res_Target_Node(browsePath)(ind)))) =
                        RelativePathElt_TargetName(last(RelativePath_RelativePathElt(BrowsePath_RelativePath(browsePath)))) &
                        BrowsePath_Res_Target_Index(browsePath)(ind) = k_n_noRemainingPathIndex
                    ) &
                    (
                        /* If Node is not local */
                        BrowsePath_Res_Target_Node(browsePath)(ind) /: dom(conv_ExpandedNodeId_NodeId)
                        =>
                        BrowsePath_Res_Target_Node(browsePath)(ind) /= c_ExpandedNodeId_indet &
                        BrowsePath_Res_Target_Index(browsePath)(ind) < k_n_noRemainingPathIndex
                    )
                )
            ) &


            (BrowsePath_Res_StatusCode(browsePath) : {e_sc_bad_nothing_to_do, e_sc_bad_node_id_unknown,
                    e_sc_bad_query_too_complex, e_sc_bad_no_match}
                =>
                BrowsePath_Res_Target_Node(browsePath) = {} &
                BrowsePath_Res_Target_Index(browsePath) = {}
            )
        )
    END
    ;

    write_translate_browse_paths_response(p_msg_out) =
    PRE
        p_msg_out : t_msg_i &
        p_msg_out : t_msg
    THEN
        d_init
    END
    ;

    free_translate_browse_paths_response =
    BEGIN
        skip
    END
    ;


    /* Promoted from msg_translate_browse_path_bs */
    p_nb_BrowsePaths <-- read_nb_BrowsePaths =
    PRE
        p_nb_BrowsePaths : NAT
    THEN
        p_nb_BrowsePaths := card(BrowsePaths)
    END
    ;

    /* Promoted from Browse Treatment */

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
        :(  p_service_StatusCode : {e_sc_ok, e_sc_bad_continuation_point_invalid} &
            BrowseContext_MaxNbTargets : NAT & BrowseContext_MaxNbTargets <= k_n_BrowseTarget_max &
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
          p_serviceStatusCode : {e_sc_ok, e_sc_bad_out_of_memory, e_sc_bad_node_id_unknown,
                                   e_sc_bad_reference_type_id_invalid, e_sc_bad_no_continuation_points,
                                   e_sc_bad_view_id_unknown} & /* Add other bad view statuses when treated */
          p_nbReferences : NAT &
          p_nbReferences <= k_n_BrowseTarget_max &
          p_nbReferences = BrowseContext_MaxNbTargets &
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
            p_nb_references <= k_n_BrowseTarget_max &
            p_nb_references = BrowseContext_MaxNbTargets) ||
        p_browseResult :: t_BrowseResultReferences_i
    END

END
