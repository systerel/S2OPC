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
#include "data_value_pointer_bs.h"

#include "util_b2c.h"

#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void data_value_pointer_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void data_value_pointer_bs__get_conv_DataValue_LocalDataType(
    const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
    constants__t_NodeId_i* const data_value_pointer_bs__p_dt)
{
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *data_value_pointer_bs__p_dt = (SOPC_NodeId*) SOPC_Variant_Get_DataType(&data_value_pointer_bs__p_dataValue->Value);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

void data_value_pointer_bs__get_conv_DataValue_SourceTimestamp(
    const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
    constants__t_Timestamp* const data_value_pointer_bs__p_st)
{
    *data_value_pointer_bs__p_st = (constants__t_Timestamp){data_value_pointer_bs__p_dataValue->SourceTimestamp,
                                                            data_value_pointer_bs__p_dataValue->SourcePicoSeconds};
}

void data_value_pointer_bs__get_conv_DataValue_Status(const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
                                                      constants__t_RawStatusCode* const data_value_pointer_bs__p_sc)
{
    *data_value_pointer_bs__p_sc = data_value_pointer_bs__p_dataValue->Status;
}

void data_value_pointer_bs__get_conv_DataValue_ValueRank(
    const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
    t_entier4* const data_value_pointer_bs__p_vr)
{
    *data_value_pointer_bs__p_vr = SOPC_Variant_Get_ValueRank(&data_value_pointer_bs__p_dataValue->Value);
}
void data_value_pointer_bs__get_conv_DataValue_Variant(
    const constants__t_DataValue_i data_value_pointer_bs__p_dataValue,
    constants__t_Variant_i* const data_value_pointer_bs__p_variant)
{
    *data_value_pointer_bs__p_variant = &data_value_pointer_bs__p_dataValue->Value;
}
