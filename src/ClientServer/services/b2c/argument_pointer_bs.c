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

#include "address_space_impl.h"
#include "util_b2c.h"

#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void argument_pointer_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

// Variant should be an array of ExtensionObject
static SOPC_VariantValue* argument_pointer_bs__check_and_get_variant_value(
    const constants__t_Variant_i argument_pointer_bs__p_variant)
{
    SOPC_Variant* variant = argument_pointer_bs__p_variant;
    if (NULL == variant || SOPC_VariantArrayType_Array != variant->ArrayType ||
        SOPC_ExtensionObject_Id != variant->BuiltInTypeId)
    {
        return NULL;
    }
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
    if (NULL != value)
    {
        int32_t length = value->Array.Length;
        assert(0 < argument_pointer_bs__p_index && argument_pointer_bs__p_index <= length);
        SOPC_ExtensionObject* extObjectArr = &value->Array.Content.ExtObjectArr[argument_pointer_bs__p_index - 1];

        assert(SOPC_ExtObjBodyEncoding_Object == extObjectArr->Encoding);            // .. encoded in object ..
        assert(&OpcUa_Argument_EncodeableType == extObjectArr->Body.Object.ObjType); // .. of type argument.
        *argument_pointer_bs__p_arg = (OpcUa_Argument*) extObjectArr->Body.Object.Value;
        assert(NULL != *argument_pointer_bs__p_arg);
        assert(&OpcUa_Argument_EncodeableType == (*argument_pointer_bs__p_arg)->encodeableType);
    }
}

void argument_pointer_bs__read_variant_nb_argument(const constants__t_Variant_i argument_pointer_bs__p_variant,
                                                   const constants__t_Node_i argument_pointer_bs__p_node,
                                                   t_entier4* const argument_pointer_bs__p_nb,
                                                   t_bool* const argument_pointer_bs__p_bres)
{
    assert(NULL != argument_pointer_bs__p_nb);

    if (NULL == argument_pointer_bs__p_variant)
    {
        // Special case, if input variant is null it means there is no arguments !
        // (<=> sequence of 0 arguments)
        *argument_pointer_bs__p_nb = 0;
        *argument_pointer_bs__p_bres = true;
        return;
    }

    SOPC_VariantValue* value = argument_pointer_bs__check_and_get_variant_value(argument_pointer_bs__p_variant);
    if (NULL == value)
    {
        char* nodeId =
            SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, argument_pointer_bs__p_node));
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "input arguments of method %s cannot be found or have incorrect format in address space",
                               nodeId);
        SOPC_Free(nodeId);
        *argument_pointer_bs__p_nb = 0;
        *argument_pointer_bs__p_bres = false;
    }
    else
    {
        *argument_pointer_bs__p_nb = value->Array.Length;
        *argument_pointer_bs__p_bres = true;
    }
}
