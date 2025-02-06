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

#include "namespace_array_bs.h"
#include "sopc_assert.h"
#include "sopc_logger.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespace_array_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
static SOPC_Variant* namespace_array_bs__check_and_get_variant(
    const constants__t_Variant_i namespace_array_bs__p_variant)
{
    SOPC_Variant* variant = namespace_array_bs__p_variant;
    // Check that the variant contains String[].
    if (NULL == variant || SOPC_String_Id != variant->BuiltInTypeId ||
        SOPC_VariantArrayType_Array != variant->ArrayType)
    {
        return NULL;
    }
    return variant;
}

void namespace_array_bs__read_variant_namespaceUri(const constants__t_Variant_i namespace_array_bs__p_variant,
                                                   const t_entier4 namespace_array_bs__p_index,
                                                   constants__t_NamespaceUri* const namespace_array_bs__p_namespaceUri)
{
    SOPC_Variant* variant = namespace_array_bs__check_and_get_variant(namespace_array_bs__p_variant);
    SOPC_ASSERT(NULL != variant);
    int32_t length = variant->Value.Array.Length;
    // Check we try to access at a valid index
    SOPC_ASSERT(0 < namespace_array_bs__p_index && namespace_array_bs__p_index <= length);
    *namespace_array_bs__p_namespaceUri = &variant->Value.Array.Content.StringArr[namespace_array_bs__p_index - 1];
}

void namespace_array_bs__read_variant_nb_namespaceUris(const constants__t_Variant_i namespace_array_bs__p_variant,
                                                       t_entier4* const namespace_array_bs__p_nb)
{
    SOPC_ASSERT(NULL != namespace_array_bs__p_nb);

    SOPC_Variant* variant = namespace_array_bs__check_and_get_variant(namespace_array_bs__p_variant);
    if (NULL == variant)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Value of server NamespaceArray node have incorrect format in address space");
        *namespace_array_bs__p_nb = 0;
    }
    else
    {
        *namespace_array_bs__p_nb = variant->Value.Array.Length;
    }
}
