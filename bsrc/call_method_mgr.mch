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
    call_method_mgr

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs,
    request_handle_bs

DEFINITIONS
    "address_space.def"

ABSTRACT_VARIABLES
    d_adsp_variables

INVARIANT
    d_adsp_invariant

INITIALISATION
    d_adsp_variables
    :(
        d_adsp_invariant
    )

OPERATIONS

    d_adsp_operations;
    d_adsp_write_operation;

    /* call service operation */
    d_adsp_call_operation;

    /* operation used by service_add_nodes */
    d_adsp_add_node_internal_operation

END
