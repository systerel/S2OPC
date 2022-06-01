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
    browse_treatment_1
SEES
    constants,
    address_space_itf

OPERATIONS

    /* Get the node associated to a node ID and the number of references starting from this node.
       p_src_nodeid : current node ID to process (in t_NodeId)
       p_isvalid    : indicates whether the node ID is associated to a node
       p_nb_ref     : number of references starting from this source node
       p_src_node   : source node associated to the node ID (in t_Node if such an association exists)
    */
    p_isvalid, p_nb_ref, p_src_node <-- getall_SourceNode_NbRef(p_src_nodeid) =
    PRE
        p_src_nodeid : t_NodeId_i &
        p_src_nodeid : t_NodeId
    THEN
        IF  p_src_nodeid : dom(a_NodeId~) THEN
            p_isvalid := TRUE ||
            p_nb_ref  :(p_nb_ref : NAT & p_nb_ref = Node_RefIndexEnd(a_NodeId~(p_src_nodeid))) ||
            p_src_node :(p_src_node : t_Node_i &
                         p_src_node = a_NodeId~(p_src_nodeid))
        ELSE
            p_isvalid  := FALSE ||
            p_nb_ref   :: NAT   ||
            p_src_node :: t_Node_i
        END
    END
    ;

    /* Indicates whether a reference type 2 is compatible a reference type 1
       p_is_ref_type1     : indicates whether there is a reference type 1 to check compatibility with.
                            In this case the following 2 parameters define the compatibility
       p_ref_type1        : reference type 1 (in t_Node_Id)
       p_inc_subtypes     : indicates whether subtypes of reference type 1 are considered compatible.
                            Else only a reference type equal to reference type 1 is compatible.
       p_ref_type2        : reference type 2 (in t_Node_Id)
       p_ref_types_compat : indicates whether type 2 is compatible
    */
    p_ref_types_compat <-- Is_RefTypes_Compatible(p_is_ref_type1, p_ref_type1, p_inc_subtypes, p_ref_type2) =
    PRE
        p_is_ref_type1 : BOOL       &
        p_ref_type1    : t_NodeId_i &
        (p_is_ref_type1 = TRUE => p_ref_type1 : t_NodeId) &
        p_inc_subtypes : BOOL       &
        p_ref_type2    : t_NodeId_i &
        p_ref_type2    : t_NodeId
    THEN
        IF  p_is_ref_type1 = TRUE THEN
            IF  p_ref_type1 = p_ref_type2 THEN
                p_ref_types_compat := TRUE
            ELSIF p_inc_subtypes = TRUE & p_ref_type1 /= c_NodeId_indet THEN
                p_ref_types_compat := bool(p_ref_type2 |-> p_ref_type1 : Type_IsTransitiveSubtype)
            ELSE
                p_ref_types_compat := FALSE
            END
        ELSE
            p_ref_types_compat := TRUE
        END
    END
    ;

    p_BrowseName, p_DisplayName, p_NodeClass, p_TypeDefinition <-- get_optional_fields_ReferenceDescription(p_TargetNodeId) =
    PRE
        p_TargetNodeId : t_ExpandedNodeId_i &
        p_TargetNodeId : t_ExpandedNodeId
    THEN
        p_BrowseName, p_DisplayName, p_NodeClass, p_TypeDefinition
        :(
          p_BrowseName : t_QualifiedName_i &
          p_DisplayName : t_LocalizedText_i &
          p_DisplayName : t_LocalizedText &
          p_NodeClass : t_NodeClass_i &
          p_TypeDefinition : t_ExpandedNodeId_i &
          (p_TargetNodeId : dom(conv_ExpandedNodeId_NodeId) &
           conv_ExpandedNodeId_NodeId(p_TargetNodeId) : ran(a_NodeId)
              =>
                p_BrowseName = a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(p_TargetNodeId))) &
                p_DisplayName = a_DisplayName(a_NodeId~(conv_ExpandedNodeId_NodeId(p_TargetNodeId))) &
                p_NodeClass = a_NodeClass(a_NodeId~(conv_ExpandedNodeId_NodeId(p_TargetNodeId))) &
                p_TypeDefinition = Ref_HasTypeDefinition(a_NodeId~(conv_ExpandedNodeId_NodeId(p_TargetNodeId)))
          ) &
          (not (p_TargetNodeId : dom(conv_ExpandedNodeId_NodeId) &
                conv_ExpandedNodeId_NodeId(p_TargetNodeId) : ran(a_NodeId))
              =>
                p_BrowseName = c_QualifiedName_indet &
                p_DisplayName = c_LocalizedText_indet &
                p_NodeClass = c_NodeClass_indet &
                p_TypeDefinition = c_ExpandedNodeId_indet
          )
        )
    END

END
