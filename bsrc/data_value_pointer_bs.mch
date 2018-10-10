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
    data_value_pointer_bs

SEES
    constants

ABSTRACT_VARIABLES
    DataValue_Variant,
    DataValue_StatusCode

INVARIANT
    DataValue_Variant : t_DataValue --> t_Variant &
    DataValue_StatusCode : t_DataValue --> t_StatusCode

INITIALISATION
    DataValue_Variant :: {} ||
    DataValue_StatusCode :: {}

    /*
OPERATIONS

    p_variant <-- get_conv_DataValue_Variant(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue_i
    THEN
        p_variant :(p_variant: t_Variant_i & p_variant: t_Variant)
    END
    ;

    p_sc <-- get_conv_DataValue_Status(p_dataValue) =
    PRE
        p_dataValue : t_DataValue_i &
        p_dataValue : t_DataValue_i
    THEN
        p_sc :(p_sc: t_StatusCode_i & p_sc: t_StatusCode)
    END
    */

END
