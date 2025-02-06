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

#include "namespaces_uri_eval_bs.h"
#include "sopc_assert.h"
#include "sopc_logger.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespaces_uri_eval_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void namespaces_uri_eval_bs__compare_namespaceUri_and_value_uri(
    const constants__t_NamespaceUri namespaces_uri_eval_bs__p_namespaceUri,
    const constants__t_Variant_i namespaces_uri_eval_bs__p_variant,
    t_bool* const namespaces_uri_eval_bs__p_bres)
{
    const SOPC_Variant* variant = namespaces_uri_eval_bs__p_variant;
    int32_t comparison = -1;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    *namespaces_uri_eval_bs__p_bres = false;
    // Variant must be single opc ua string
    if (SOPC_String_Id == variant->BuiltInTypeId && SOPC_VariantArrayType_SingleValue == variant->ArrayType)
    {
        const SOPC_String namespaceUri = variant->Value.String;
        status = SOPC_String_Compare(&namespaceUri, namespaces_uri_eval_bs__p_namespaceUri, false, &comparison);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 == comparison)
        {
            *namespaces_uri_eval_bs__p_bres = true;
        }
    }
}
