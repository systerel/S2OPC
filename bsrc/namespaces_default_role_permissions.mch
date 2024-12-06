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
    namespaces_default_role_permissions

SEES
    constants

OPERATIONS
    p_val_DefaultRolePermissions <-- get_DefaultRolePermissions(p_namespaceUri) =
    PRE
        p_namespaceUri : t_NamespaceUri
    THEN
        p_val_DefaultRolePermissions :(p_val_DefaultRolePermissions : t_RolePermissionTypes_i)
    END
    ;

    delete_rolePermissions(p_rolePermissions) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes
    THEN
        skip
    END
END
