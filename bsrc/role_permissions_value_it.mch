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
    role_permissions_value_it

SEES
    constants

DEFINITIONS
    d_variables ==
    rolePermissionsIdx_to_iterate,
    rolePermissionsIdx_iterated,
    nb_rolePermissions
    ;

    d_inv ==
    (
    nb_rolePermissions : NAT &
    rolePermissionsIdx_to_iterate <: NAT1 &
    rolePermissionsIdx_iterated   <: NAT1 &
    rolePermissionsIdx_to_iterate /\ rolePermissionsIdx_iterated = {} &
    1..nb_rolePermissions = rolePermissionsIdx_to_iterate \/ rolePermissionsIdx_iterated
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_rolePermissions(p_nb_rolePermissions) =
    PRE
        p_nb_rolePermissions : NAT
    THEN
        rolePermissionsIdx_iterated := {}                        ||
        rolePermissionsIdx_to_iterate := 1..p_nb_rolePermissions ||
        nb_rolePermissions := p_nb_rolePermissions               ||
        p_continue := bool(p_nb_rolePermissions > 0)
    END
    ;

    p_continue, p_rolePermissionIdx <-- continue_iter_rolePermissions =
    PRE
        rolePermissionsIdx_to_iterate /= {}
    THEN
        ANY l_rolePermissionIdx WHERE
            l_rolePermissionIdx : NAT &
            l_rolePermissionIdx : rolePermissionsIdx_to_iterate
        THEN
            rolePermissionsIdx_iterated   := rolePermissionsIdx_iterated   \/ {l_rolePermissionIdx}       ||
            rolePermissionsIdx_to_iterate := rolePermissionsIdx_to_iterate -  {l_rolePermissionIdx}       ||
            p_rolePermissionIdx           := l_rolePermissionIdx                                          ||
            p_continue                    := bool(rolePermissionsIdx_to_iterate /= {l_rolePermissionIdx})
        END
    END

END
