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

/*--------------
   SEES Clause
  --------------*/
#include "argument_pointer_bs.h"

#include "util_b2c.h"

#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void argument_pointer_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

// Variant should be an array of ExtensionObject
static SOPC_VariantValue* argument_pointer_bs__check_and_get_variant_value(
    const constants__t_Variant_i argument_pointer_bs__p_variant);

static SOPC_VariantValue* argument_pointer_bs__check_and_get_variant_value(
    const constants__t_Variant_i argument_pointer_bs__p_variant)
{
    SOPC_Variant* variant = argument_pointer_bs__p_variant;
    if (NULL == variant)
    {
        return NULL;
    }
    // TODO: These assertions shall be checked by B Model or changed for If statement
    assert(SOPC_VariantArrayType_Array == variant->ArrayType); // variant is an array
    assert(SOPC_ExtensionObject_Id == variant->BuiltInTypeId); // variant value is an extension object..
    return &variant->Value;
}

void argument_pointer_bs__read_argument_type(const constants__t_Argument_i argument_pointer_bs__p_arg,
                                             constants__t_NodeId_i* const argument_pointer_bs__p_type)
{
    assert(NULL != argument_pointer_bs__p_type);
    *argument_pointer_bs__p_type = &argument_pointer_bs__p_arg->DataType;
}

void argument_pointer_bs__read_argument_valueRank(const constants__t_Argument_i argument_pointer_bs__p_arg,
                                                  t_entier4* const argument_pointer_bs__p_vr)
{
    assert(NULL != argument_pointer_bs__p_vr);
    *argument_pointer_bs__p_vr = argument_pointer_bs__p_arg->ValueRank;
}

void argument_pointer_bs__read_variant_argument(const constants__t_Variant_i argument_pointer_bs__p_variant,
                                                const t_entier4 argument_pointer_bs__p_index,
                                                constants__t_Argument_i* const argument_pointer_bs__p_arg)
{
    assert(NULL != argument_pointer_bs__p_arg);

    SOPC_VariantValue* value = argument_pointer_bs__check_and_get_variant_value(argument_pointer_bs__p_variant);
    assert(NULL != value);
    int32_t length = value->Array.Length;
    assert(0 < argument_pointer_bs__p_index && argument_pointer_bs__p_index <= length);
    SOPC_ExtensionObject* extObjectArr = &value->Array.Content.ExtObjectArr[argument_pointer_bs__p_index - 1];

    assert(SOPC_ExtObjBodyEncoding_Object == extObjectArr->Encoding);            // .. encoded in object ..
    assert(&OpcUa_Argument_EncodeableType == extObjectArr->Body.Object.ObjType); // .. of type argument.
    *argument_pointer_bs__p_arg = (OpcUa_Argument*) extObjectArr->Body.Object.Value;
    assert(NULL != *argument_pointer_bs__p_arg);
    assert(&OpcUa_Argument_EncodeableType == (*argument_pointer_bs__p_arg)->encodeableType);
}

void argument_pointer_bs__read_variant_nb_argument(const constants__t_Variant_i argument_pointer_bs__p_variant,
                                                   t_entier4* const argument_pointer_bs__p_nb)
{
    assert(NULL != argument_pointer_bs__p_nb);
    SOPC_VariantValue* value = argument_pointer_bs__check_and_get_variant_value(argument_pointer_bs__p_variant);
    if (NULL == value)
    {
        *argument_pointer_bs__p_nb = 0;
    }
    else
    {
        *argument_pointer_bs__p_nb = value->Array.Length;
    }
}
