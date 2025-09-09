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

#include "address_space_namespaces_indexes_bs.h"

#include "sopc_logger.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_namespaces_indexes_bs__INITIALISATION(void)
{
    /*Translated from B but an intialisation is not needed from this module.*/
}

static SOPC_Variant* address_space_namespaces_indexes_bs__check_and_get_variant(
    const constants__t_Variant_i namespace_array_bs__p_variant)
{
    // Same as namespace_array_bs__check_and_get_variant
    SOPC_Variant* variant = namespace_array_bs__p_variant;
    // Check that the variant contains String[].
    if (NULL == variant || SOPC_String_Id != variant->BuiltInTypeId ||
        SOPC_VariantArrayType_Array != variant->ArrayType)
    {
        return NULL;
    }
    return variant;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_namespaces_indexes_bs__read_variant_max_namespaceIndex(
    const constants__t_Variant_i address_space_namespaces_indexes_bs__p_variant,
    t_entier4* const address_space_namespaces_indexes_bs__p_maxNsIdx)
{
    // Same as namespace_array_bs__read_variant_nb_namespaceUris

    const SOPC_Variant* variant =
        address_space_namespaces_indexes_bs__check_and_get_variant(address_space_namespaces_indexes_bs__p_variant);
    if (NULL == variant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Value of server NamespaceArray node have incorrect format in address space");
        *address_space_namespaces_indexes_bs__p_maxNsIdx = 0;
    }
    else
    {
        *address_space_namespaces_indexes_bs__p_maxNsIdx = variant->Value.Array.Length - 1;
    }
}
