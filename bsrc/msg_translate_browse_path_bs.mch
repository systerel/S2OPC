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
    msg_translate_browse_path_bs

SEES
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
    d_inv_in ==
        IsDecoded                      : BOOL                                 &
           /* BrowsePath */
        BrowsePaths                    <: t_BrowsePath                        &
        BrowsePaths                    = cast_t_BrowsePath[1..card(BrowsePaths)] &
        BrowsePath_StartingNode        : BrowsePaths --> t_NodeId            &
        BrowsePath_RelativePath        : BrowsePaths --> t_RelativePath      &

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
    d_init ==
    d_variables_in,
    d_variables_out
        :(
            IsDecoded : BOOL &
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

    d_add_BrowsePath_Res_Target(remainingIndex) ==
        LET target_index BE
            target_index = card(BrowsePath_Res_Target_Node(browsePath))
        IN

        StatusCode,
        BrowsePath_Res_Target_Node,
        BrowsePath_Res_Target_Index
        :(
            StatusCode : t_StatusCode_i &
            StatusCode : { e_sc_ok, e_sc_bad_out_of_memory } &

            (StatusCode = e_sc_ok =>
                {browsePath} <<| BrowsePath_Res_Target_Node = {browsePath} <<| BrowsePath_Res_Target_Node$0 &
                BrowsePath_Res_Target_Node(browsePath) = BrowsePath_Res_Target_Node$0(browsePath) <+ {target_index |-> node} &
                {browsePath} <<| BrowsePath_Res_Target_Index = {browsePath} <<| BrowsePath_Res_Target_Index$0 &
                BrowsePath_Res_Target_Index(browsePath) = BrowsePath_Res_Target_Index$0(browsePath) <+ {target_index |-> remainingIndex}
            ) &
            (StatusCode /= e_sc_ok =>
                BrowsePath_Res_Target_Node = BrowsePath_Res_Target_Node$0 &
                BrowsePath_Res_Target_Index = BrowsePath_Res_Target_Index$0
            )
        )
    END

ABSTRACT_VARIABLES
    d_variables_in,
    d_variables_out

INVARIANT
    d_inv_in &
    d_inv_out

INITIALISATION
    d_init

OPERATIONS

    StatusCode_service <-- decode_translate_browse_paths_request(req_msg) =
    PRE
        req_msg : t_msg_i &
        req_msg : t_msg   &
        req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_translate_browse_paths_to_node_ids_req
    THEN
        StatusCode_service,
        d_variables_in
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : {e_sc_ok, e_sc_bad_too_many_ops} /* to do in C */   &
            ( StatusCode_service = e_sc_ok
              =>
              IsDecoded = TRUE
            ) &
            ( StatusCode_service = e_sc_bad_too_many_ops
              =>
              IsDecoded = FALSE
            ) &
            d_inv_in
        )
    END
    ;

    p_nb_BrowsePaths <-- read_nb_BrowsePaths =
    PRE
        IsDecoded = TRUE
    THEN
        p_nb_BrowsePaths :(p_nb_BrowsePaths : NAT & p_nb_BrowsePaths = card(BrowsePaths))
    END
    ;

    nodeId <-- read_BrowsePath_StartingNode(browsePath) =
    PRE
        browsePath : t_BrowsePath_i &
        browsePath : BrowsePaths
    THEN
        nodeId :(
            nodeId : t_NodeId_i &
            nodeId = BrowsePath_StartingNode(browsePath))
    END
    ;

    relativePath <-- read_BrowsePath_RelativePath(browsePath) =
    PRE
        browsePath : t_BrowsePath_i     &
        browsePath : BrowsePaths
    THEN
        relativePath :(
            relativePath : t_RelativePath_i &
            relativePath = BrowsePath_RelativePath(browsePath))
    END
    ;

    nb_relativePathElt <-- read_RelativePath_Nb_RelativePathElt(relativePath) =
    PRE
        relativePath : t_RelativePath_i &
        relativePath : t_RelativePath   &
        relativePath : dom(RelativePath_RelativePathElt)
    THEN
        nb_relativePathElt := card(RelativePath_RelativePathElt(relativePath))
    END
    ;

    relativePathElt <-- read_RelativePath_RelativePathElt(relativePath, index) =
    PRE
        relativePath : t_RelativePath_i &
        relativePath : t_RelativePath   &
        relativePath : dom(RelativePath_RelativePathElt) &
        index : NAT &
        index : dom(RelativePath_RelativePathElt(relativePath))
    THEN
        relativePathElt :(
            relativePathElt : t_RelativePathElt_i &
            relativePathElt = RelativePath_RelativePathElt(relativePath)(index))
    END
    ;

    referenceTypeId <-- read_RelativePathElt_ReferenceTypeId(relativePathElt) =
    PRE
        relativePathElt : t_RelativePathElt_i &
        relativePathElt : t_RelativePathElt   &
        relativePathElt : dom(RelativePathElt_ReferenceTypeId)
    THEN
        referenceTypeId
        :(referenceTypeId : t_NodeId_i &
          referenceTypeId : t_NodeId   &
          referenceTypeId = RelativePathElt_ReferenceTypeId(relativePathElt)
        )
    END
    ;

    isInverse <-- read_RelativePathElt_IsInverse(relativePathElt) =
    PRE
        relativePathElt: t_RelativePathElt_i &
        relativePathElt: t_RelativePathElt   &
        relativePathElt : dom(RelativePathElt_IsInverse)
    THEN
        isInverse
        :(isInverse : BOOL &
           isInverse = RelativePathElt_IsInverse(relativePathElt)
           )
    END
    ;

    includedSubtypes <-- read_RelativePathElt_IncludedSubtypes(relativePathElt) =
    PRE
        relativePathElt: t_RelativePathElt_i &
        relativePathElt: t_RelativePathElt   &
        relativePathElt : dom(RelativePathElt_IncludedSubtypes)
    THEN
        includedSubtypes
        :(includedSubtypes : BOOL &
           includedSubtypes = RelativePathElt_IncludedSubtypes(relativePathElt)
           )
    END
    ;

    targetName <-- read_RelativePathElt_TargetName(relativePathElt) =
    PRE
        relativePathElt: t_RelativePathElt_i &
        relativePathElt: t_RelativePathElt   &
        relativePathElt : dom(RelativePathElt_TargetName)
    THEN
        targetName
        :(targetName : t_QualifiedName_i &
           targetName : t_QualifiedName   &
           targetName = RelativePathElt_TargetName(relativePathElt)
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

    write_BrowsePath_Res_StatusCode(browsePath, statusCode) =
    PRE
        browsePath : t_BrowsePath_i &
        browsePath : BrowsePaths    &
        statusCode : t_StatusCode_i &
        statusCode : t_StatusCode   &
        dom(BrowsePath_Res_StatusCode) = BrowsePaths
    THEN
        BrowsePath_Res_StatusCode(browsePath) := statusCode
    END
    ;

    statusCode <-- alloc_BrowsePath_Res_Target(browsePath, nbTargetMax) =
    PRE
        browsePath : t_BrowsePath_i &
        browsePath : BrowsePaths    &
        nbTargetMax : NAT1 &
        IsDecoded = TRUE   &
        dom(BrowsePath_Res_NbTargetMax) = BrowsePaths
    THEN
        BrowsePath_Res_NbTargetMax,
        BrowsePath_Res_Target_Node,
        BrowsePath_Res_Target_Index,
        statusCode
        :(
            statusCode : t_StatusCode_i &
            statusCode : t_StatusCode   &
            (statusCode  = e_sc_ok => BrowsePath_Res_NbTargetMax = BrowsePath_Res_NbTargetMax$0 <+ {browsePath |-> nbTargetMax} ) &
            (statusCode /= e_sc_ok => BrowsePath_Res_NbTargetMax = BrowsePath_Res_NbTargetMax$0 <+ {browsePath |-> 0} ) &
                BrowsePath_Res_Target_Node  = BrowsePath_Res_Target_Node$0 <+ {browsePath |-> {}} &
            BrowsePath_Res_Target_Index = BrowsePath_Res_Target_Index$0 <+ {browsePath |-> {}}
        )
    END
    ;

    StatusCode <-- add_BrowsePath_Res_Target(browsePath, node) =
    PRE
        browsePath : t_BrowsePath_i  &
        browsePath : BrowsePaths     &
        node: t_ExpandedNodeId_i     &
        node: t_ExpandedNodeId       &
        card(BrowsePath_Res_Target_Node(browsePath)) < BrowsePath_Res_NbTargetMax(browsePath)
    THEN
        d_add_BrowsePath_Res_Target(k_n_noRemainingPathIndex)
    END
    ;

    StatusCode <-- add_BrowsePath_Res_Target_withRemainingPath(browsePath, node, remainingIndex) =
    PRE
        browsePath: t_BrowsePath_i   &
        browsePath : BrowsePaths     &
        node: t_ExpandedNodeId_i     &
        node: t_ExpandedNodeId       &
        remainingIndex : NAT         &
        card(BrowsePath_Res_Target_Node(browsePath)) < BrowsePath_Res_NbTargetMax(browsePath)
    THEN
        d_add_BrowsePath_Res_Target(remainingIndex)
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

END


