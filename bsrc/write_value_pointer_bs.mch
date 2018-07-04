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
    write_value_pointer_bs

SEES
    constants

OPERATIONS

    is_valid <-- write_value_pointer_is_valid (p_write_value) =
    PRE
        p_write_value : t_WriteValuePointer_i
    THEN
        is_valid := bool(p_write_value : t_WriteValuePointer)
    END
    ;

    p_nid, p_aid <-- get_write_value_pointer_NodeId_AttributeId (p_write_value) =
    PRE
        p_write_value : t_WriteValuePointer_i &
        p_write_value : t_WriteValuePointer
    THEN
        p_nid, p_aid
        :(p_nid : t_NodeId_i & p_nid : t_NodeId &
            p_aid : t_AttributeId_i & p_aid : t_AttributeId)
    END
    ;

    bres, p_write_value_copy <-- copy_write_value_pointer_content (p_write_value) =
    PRE
        p_write_value : t_WriteValuePointer_i &
        p_write_value : t_WriteValuePointer
    THEN
        bres,
        p_write_value_copy
        :(bres : BOOL &
          p_write_value_copy : t_WriteValuePointer_i &
          (bres = FALSE => p_write_value_copy = c_WriteValuePointer_indet) &
          (bres = TRUE => p_write_value_copy : t_WriteValuePointer))
    END
    ;

    free_write_value_pointer (p_write_value) =
    PRE
        p_write_value : t_WriteValuePointer_i &
        p_write_value : t_WriteValuePointer
    THEN
        skip
    END
END
