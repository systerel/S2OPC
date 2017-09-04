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

MACHINE
    msg_browse_response_bs

SEES
    constants,
    message_out_bs

DEFINITIONS
    d_variables ==
        ResponseBrowse_BrowseStatus,
        ResponseBrowse_ContinuationPoint,
        ResponseBrowse_Res_ReferenceTypeId,
        ResponseBrowse_Res_Forwards,
        ResponseBrowse_Res_NodeId,
        ResponseBrowse_Res_BrowseName,
        ResponseBrowse_Res_DisplayName,
        ResponseBrowse_Res_NodeClass,
        ResponseBrowse_Res_TypeDefinition

ABSTRACT_VARIABLES
    d_variables

INVARIANT
        ResponseBrowse_BrowseStatus        : t_BrowseValue +-> BOOL                                  &
        ResponseBrowse_ContinuationPoint   : t_BrowseValue +-> t_Reference                           &
        /* TODOPAB: comment on gère la taille non connue à l'avance de ces tableaux de BrowseResult  ??? */
        ResponseBrowse_Res_ReferenceTypeId : t_BrowseValue +-> (t_BrowseResult +-> t_NodeId)         &
        ResponseBrowse_Res_Forwards        : t_BrowseValue +-> (t_BrowseResult +-> BOOL)             &
        ResponseBrowse_Res_NodeId          : t_BrowseValue +-> (t_BrowseResult +-> t_ExpandedNodeId) &
        ResponseBrowse_Res_BrowseName      : t_BrowseValue +-> (t_BrowseResult +-> t_QualifiedName)  &
        ResponseBrowse_Res_DisplayName     : t_BrowseValue +-> (t_BrowseResult +-> t_LocalizedText)  &
        ResponseBrowse_Res_NodeClass       : t_BrowseValue +-> (t_BrowseResult +-> t_NodeClass)      &
        ResponseBrowse_Res_TypeDefinition  : t_BrowseValue +-> (t_BrowseResult +-> t_ExpandedNodeId)

INITIALISATION
    d_variables
    :(
        ResponseBrowse_BrowseStatus        = {}                   &
        ResponseBrowse_ContinuationPoint   = {}                   &
        ResponseBrowse_Res_ReferenceTypeId = t_BrowseValue * {{}} &
        ResponseBrowse_Res_Forwards        = t_BrowseValue * {{}} &
        ResponseBrowse_Res_NodeId          = t_BrowseValue * {{}} &
        ResponseBrowse_Res_BrowseName      = t_BrowseValue * {{}} &
        ResponseBrowse_Res_DisplayName     = t_BrowseValue * {{}} &
        ResponseBrowse_Res_NodeClass       = t_BrowseValue * {{}} &
        ResponseBrowse_Res_TypeDefinition  = t_BrowseValue * {{}}
     )

OPERATIONS

    set_ResponseBrowse_BrowseStatus(p_bvi, p_bool) =
    PRE
        p_bvi  : t_BrowseValue_i &
        p_bvi  : t_BrowseValue   &
        p_bool : BOOL
    THEN
        ResponseBrowse_BrowseStatus(p_bvi) := p_bool
    END
    ;

    set_ResponseBrowse_ContinuationPoint(p_bvi, p_ref) =
    PRE
        p_bvi : t_BrowseValue_i &
        p_bvi : t_BrowseValue   &
        p_ref : t_Reference_i   &
        p_ref : t_Reference
    THEN
        ResponseBrowse_ContinuationPoint(p_bvi) := p_ref
    END
    ;

    reset_ResponseBrowse_ContinuationPoint(p_bvi) =
    PRE
        p_bvi : t_BrowseValue_i &
        p_bvi : t_BrowseValue
    THEN
        ResponseBrowse_ContinuationPoint := {p_bvi} <<| ResponseBrowse_ContinuationPoint
    END
    ;

    set_ResponseBrowse_Res_ReferenceTypeId(p_bvi, p_bri, p_NodeId) =
    PRE
        p_bvi    : t_BrowseValue_i  &
        p_bvi    : t_BrowseValue    &
        p_bri    : t_BrowseResult_i &
        p_bri    : t_BrowseResult   &
        p_NodeId : t_NodeId_i       &
        p_NodeId : t_NodeId
    THEN
        ResponseBrowse_Res_ReferenceTypeId(p_bvi)(p_bri) := p_NodeId
    END
    ;

    set_ResponseBrowse_Res_Forwards(p_bvi, p_bri, p_bool) =
    PRE
        p_bvi  : t_BrowseValue_i  &
        p_bvi  : t_BrowseValue    &
        p_bri  : t_BrowseResult_i &
        p_bri  : t_BrowseResult   &
        p_bool : BOOL
    THEN
        ResponseBrowse_Res_Forwards(p_bvi)(p_bri) := p_bool
    END
    ;

    set_ResponseBrowse_Res_NodeId(p_bvi, p_bri, p_ExpandedNodeId) =
    PRE
        p_bvi            : t_BrowseValue_i    &
        p_bvi            : t_BrowseValue      &
        p_bri            : t_BrowseResult_i   &
        p_bri            : t_BrowseResult     &
        p_ExpandedNodeId : t_ExpandedNodeId_i &
        p_ExpandedNodeId : t_ExpandedNodeId
    THEN
        ResponseBrowse_Res_NodeId(p_bvi)(p_bri) := p_ExpandedNodeId
    END
    ;

    set_ResponseBrowse_Res_BrowseName(p_bvi, p_bri, p_BrowseName) =
    PRE
        p_bvi        : t_BrowseValue_i   &
        p_bvi        : t_BrowseValue     &
        p_bri        : t_BrowseResult_i  &
        p_bri        : t_BrowseResult    &
        p_BrowseName : t_QualifiedName_i &
        p_BrowseName : t_QualifiedName
    THEN
        ResponseBrowse_Res_BrowseName(p_bvi)(p_bri) := p_BrowseName
    END
    ;

    reset_ResponseBrowse_Res_BrowseName(p_bvi, p_bri) =
    PRE
        p_bvi : t_BrowseValue_i  &
        p_bvi : t_BrowseValue    &
        p_bri : t_BrowseResult_i &
        p_bri : t_BrowseResult
    THEN
        ResponseBrowse_Res_BrowseName(p_bvi) := {p_bri} <<| ResponseBrowse_Res_BrowseName(p_bvi)
    END
    ;

    set_ResponseBrowse_Res_DisplayName(p_bvi, p_bri, p_DisplayName) =
    PRE
        p_bvi         : t_BrowseValue_i   &
        p_bvi         : t_BrowseValue     &
        p_bri         : t_BrowseResult_i  &
        p_bri         : t_BrowseResult    &
        p_DisplayName : t_LocalizedText_i &
        p_DisplayName : t_LocalizedText
    THEN
        ResponseBrowse_Res_DisplayName(p_bvi)(p_bri) := p_DisplayName
    END
    ;

    reset_ResponseBrowse_Res_DisplayName(p_bvi, p_bri) =
    PRE
        p_bvi : t_BrowseValue_i  &
        p_bvi : t_BrowseValue    &
        p_bri : t_BrowseResult_i &
        p_bri : t_BrowseResult
    THEN
        ResponseBrowse_Res_DisplayName(p_bvi) := {p_bri} <<| ResponseBrowse_Res_DisplayName(p_bvi)
    END
    ;

    set_ResponseBrowse_Res_NodeClass(p_bvi, p_bri, p_NodeClass) =
    PRE
        p_bvi       : t_BrowseValue_i  &
        p_bvi       : t_BrowseValue    &
        p_bri       : t_BrowseResult_i &
        p_bri       : t_BrowseResult   &
        p_NodeClass : t_NodeClass_i    &
        p_NodeClass : t_NodeClass
    THEN
        ResponseBrowse_Res_NodeClass(p_bvi)(p_bri) := p_NodeClass
    END
    ;

    reset_ResponseBrowse_Res_NodeClass(p_bvi, p_bri) =
    PRE
        p_bvi : t_BrowseValue_i  &
        p_bvi : t_BrowseValue    &
        p_bri : t_BrowseResult_i &
        p_bri : t_BrowseResult
    THEN
        ResponseBrowse_Res_NodeClass(p_bvi) := {p_bri} <<| ResponseBrowse_Res_NodeClass(p_bvi)
    END
    ;

    set_ResponseBrowse_Res_TypeDefinition(p_bvi, p_bri, p_TypeDefinition) =
    PRE
        p_bvi            : t_BrowseValue_i    &
        p_bvi            : t_BrowseValue      &
        p_bri            : t_BrowseResult_i   &
        p_bri            : t_BrowseResult     &
        p_TypeDefinition : t_ExpandedNodeId_i &
        p_TypeDefinition : t_ExpandedNodeId
    THEN
        ResponseBrowse_Res_TypeDefinition(p_bvi)(p_bri) := p_TypeDefinition
    END
    ;

    reset_ResponseBrowse_Res_TypeDefinition(p_bvi, p_bri) =
    PRE
        p_bvi : t_BrowseValue_i  &
        p_bvi : t_BrowseValue    &
        p_bri : t_BrowseResult_i &
        p_bri : t_BrowseResult
    THEN
        ResponseBrowse_Res_TypeDefinition(p_bvi) := {p_bri} <<| ResponseBrowse_Res_TypeDefinition(p_bvi)
    END
    ;

    p_isallocated <-- malloc_browse_result(p_bvi, p_nb_bri) =
    PRE
        p_bvi    : t_BrowseValue_i &
        p_bvi    : t_BrowseValue   &
        p_nb_bri : NAT
    THEN
        p_isallocated :: BOOL
    END
    ;

    write_BrowseResponse_msg_out(p_msg_out) =
    PRE
        p_msg_out : t_msg_i &
        p_msg_out : t_msg
    THEN
        skip
    END

END
