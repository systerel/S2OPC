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
    address_space_typing_bs

SEES
    constants

DEFINITIONS

    d_variables ==

        Type_IsTransitiveSubtype,
        Type_IsCompatSimpleTypeOrEnumeration,
        Type_IsReferenceTypeIdValid,
        Object_HasMethod
    ;

    d_inv ==
    (
        Type_IsTransitiveSubtype : t_NodeId <-> t_NodeId &
        Type_IsCompatSimpleTypeOrEnumeration : t_NodeId <-> t_NodeId &
        Type_IsReferenceTypeIdValid <: t_NodeId &
        Object_HasMethod : t_NodeId <-> t_NodeId &

        c_ByteString_Type_NodeId |->  c_Byte_Type_NodeId /: Type_IsTransitiveSubtype &
        c_Byte_Type_NodeId |-> c_ByteString_Type_NodeId  /: Type_IsTransitiveSubtype
    );

    d_init ==
        d_inv

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_init )

OPERATIONS

    bres <-- is_transitive_subtype(p_subtype, p_parent_type) =
    PRE
        p_subtype : t_NodeId_i &
        p_subtype : t_NodeId &
        p_parent_type : t_NodeId_i &
        p_parent_type : t_NodeId
    THEN
        bres := bool(p_subtype |-> p_parent_type : Type_IsTransitiveSubtype)
    END
    ;

    bres <-- is_compatible_simple_type_or_enumeration(p_value_type, p_data_type) =
    PRE
        p_value_type : t_NodeId_i &
        p_value_type : t_NodeId &
        p_data_type : t_NodeId_i &
        p_data_type : t_NodeId
    THEN
        bres := bool(p_value_type |-> p_data_type : Type_IsCompatSimpleTypeOrEnumeration)
    END
    ;

    bres <-- is_valid_ReferenceTypeId(p_nodeId) =
    PRE
        p_nodeId : t_NodeId_i &
        p_nodeId : t_NodeId
    THEN
        bres := bool(p_nodeId : Type_IsReferenceTypeIdValid)
    END
    ;

    p_bool <-- check_object_has_method(p_object, p_method) =
    PRE
        p_method : t_NodeId_i &
        p_method : t_NodeId   &
        p_object : t_NodeId_i &
        p_object : t_NodeId
    THEN
        p_bool := bool(p_object |-> p_method : Object_HasMethod)
    END

END
