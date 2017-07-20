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
    address_space_bs

SEES
    constants

DEFINITIONS
    d_inv ==
    (
        /* Attributes */
        a_NodeId : t_Node +-> t_NodeId &
        a_NodeClass : t_Node +-> t_NodeClass &

        a_Value : t_Node +-> t_Variant &

        /* Value attribute meta-data */
        a_Value_StatusCode : t_Node +-> t_StatusCode
    )

ABSTRACT_VARIABLES
    a_NodeId,
    a_NodeClass,
    a_Value,
    a_Value_StatusCode

INVARIANT
    d_inv

INITIALISATION
    /* This is where the Address Space is filled */
    a_NodeId,
    a_NodeClass,
    a_Value,
    a_Value_StatusCode
    :(
        a_NodeId : t_Node >-> t_NodeId &
        a_NodeClass : t_Node --> t_NodeClass &

        a_Value : t_Node +-> t_Variant &
        dom(a_NodeClass|>{e_ncl_Variable}) <: dom(a_Value) &
        dom(a_Value) <: dom(a_NodeClass|>{e_ncl_Variable, e_ncl_VariableType}) &

        a_Value_StatusCode : t_Node +-> t_StatusCode &
        dom(a_Value_StatusCode) = dom(a_NodeClass|>{e_ncl_Variable})
    )

OPERATIONS
    /* As INITIALISATION may use mallocs, needs an UNINIT */
    UNINITIALISATION =
    BEGIN
        skip
    END
    ;

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

    /* Reads any attribute and outputs a variant (valid or not)
     * As this function uses the util_variant__new_Variant_from_* functions, the value must be freed once used
     */
    variant <-- read_AddressSpace_Attribute_value(node, aid) =
    PRE
        node : t_Node_i         &
        node : t_Node           &
        node : dom(a_NodeId)    &
        node : dom(a_NodeClass) &
        aid  : t_AttributeId_i  &
        aid  : t_AttributeId
    THEN
        SELECT aid = e_aid_NodeId THEN
            variant
            :(
                variant : t_Variant_i &
                variant = conv_NodeId_Variant(a_NodeId(node))
            )
        WHEN aid = e_aid_NodeClass THEN
            variant := conv_NodeClass_Variant(a_NodeClass(node))
        WHEN aid = e_aid_Value THEN
            variant := a_Value(node)
        ELSE
            variant := c_Variant_indet
        END
    END
    ;

    /* a_Value, setter */
    set_Value(node, value) =
    PRE
        node  : t_Node_i    &
        node  : t_Node      &
        value : t_Variant_i &
        value : t_Variant
    THEN
        a_Value(node) := value
    END
    ;

    /* a_Value_StatusCode */
    sc <-- get_Value_StatusCode(node) =
    PRE
        node : t_Node_i &
        node : t_Node   &
        node : dom(a_Value_StatusCode)
    THEN
        sc :(sc : t_StatusCode_i &
             sc = a_Value_StatusCode(node))
    END
    ;

    /* a_NodeClass */
    ncl <-- get_NodeClass(node) =
    PRE
        node : t_Node_i &
        node : t_Node   &
        node : dom(a_NodeClass)
    THEN
        ncl
        :(
            ncl : t_NodeClass_i &
            ncl = a_NodeClass(node)
        )
    END
    ;

    /* Frees a value previously malloced with read_AddressSpace_Attribute_value */
    read_AddressSpace_free_value(val) =
    PRE
        val : t_Variant_i
    THEN
        skip
    END

END
