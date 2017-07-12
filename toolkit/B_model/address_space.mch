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
    address_space

SEES
    constants,
    service_write_decode_bs /* TODO: move to refinment */

DEFINITIONS
    "definitions.def";
    d_variables ==
        a_NodeId,
        a_NodeClass,
        a_Value,
        a_Value_StatusCode,

        /* Service Write */
        ResponseWrite_allocated,
        ResponseWrite_StatusCode
    ;

    d_inv ==
    (
        /* Attributes */
        a_NodeId : t_Node >-> t_NodeId &
        a_NodeClass : t_Node --> t_NodeClass &

        a_Value : t_Node +-> t_Variant &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
        dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        /* Value attribute meta-data */
        a_Value_StatusCode : t_Node +-> t_StatusCode &
        dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable}) &

        /* Service Write */
        ResponseWrite_allocated : BOOL &
        ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
        (ResponseWrite_allocated = FALSE
         =>
         ResponseWrite_StatusCode = {})
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables
    :(
        d_inv
    )

OPERATIONS
    /* a_NodeClass(node) and any_attribute(node) */
    ncl, val <-- read_NodeClass_Attribute(node, aid) =
    PRE
        node : t_Node_i &
        node : t_Node &
        aid : t_AttributeId_i &
        aid : t_AttributeId
    THEN
            /* TODO: add user-access levels */
        ncl,
        val:(ncl : t_NodeClass_i &
             ncl = a_NodeClass(node) &
             val : t_Variant_i & /* TODO: must not be left undeterministic in concretisation, so there must be an _indet */
            (
            node : dom(AttributeID_Node_Variant(aid))
            =>
            val = AttributeID_Node_Variant(aid)(node)
            )
        )
    END
    ;

    /* Promoted */
    /* a_NodeId~ */
    nid_valid, node <-- readall_AddressSpace_Node(nid) =
    PRE
        nid : t_NodeId_i
    THEN
        IF nid : ran(a_NodeId) THEN
            nid_valid := TRUE ||
            node :( node : t_Node_i & node|->nid : a_NodeId )
        ELSE
            nid_valid := FALSE ||
            node := c_Node_indet
        END
    END
    ;

    /* Promoted */
    sc <-- read_Value_StatusCode(node) =
    PRE
        node : t_Node_i &
        node : t_Node &
        a_NodeClass(node) = e_ncl_Variable
    THEN
        sc :(sc : t_StatusCode_i &
             sc = a_Value_StatusCode(node)
            )
    END
    ;

    /* Promoted */
    /* Frees a value previously malloced with read_AddressSpace_Attribute_value */
    read_AddressSpace_free_value(val) =
    PRE
        val : t_Variant_i
    THEN
        skip
    END
    ;

    /* --------------------------------------------------------------------- */
    /* Service Write */

    StatusCode_service <-- treat_write_request_WriteValues(userid) =
    PRE
        userid : t_UserId_i &
        userid : t_UserId &
        ResponseWrite_allocated = TRUE /* TODO & userid /= c_user_local */
    THEN
        StatusCode_service,
        a_Value,
        ResponseWrite_StatusCode
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &
            a_Value : t_Node +-> t_Variant &
            dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
            dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode
        )
    END
    ;

    StatusCode_service <-- alloc_write_request_responses(nb_req) =
    PRE
        nb_req : NAT &
        ResponseWrite_allocated = FALSE
    THEN
        StatusCode_service,
        ResponseWrite_StatusCode,
        ResponseWrite_allocated
        :(
            StatusCode_service : t_StatusCode_i &
            StatusCode_service : t_StatusCode &
            ResponseWrite_StatusCode : t_WriteValue +-> t_StatusCode &
            ResponseWrite_allocated : BOOL &
            (ResponseWrite_allocated = FALSE
             =>
             ResponseWrite_StatusCode = {})
        )
    END
    ;

    dealloc_write_request_responses =
    PRE
        ResponseWrite_allocated = TRUE
    THEN
        ResponseWrite_allocated  := FALSE ||
        ResponseWrite_StatusCode := {}
    END

END
