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

#include <stdbool.h>

#include "sopc_mem_alloc.h"
#include "sopc_types.h"
#include "util_b2c.h"
#include "write_value_pointer_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void write_value_pointer_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void write_value_pointer_bs__copy_write_value_pointer_content(
    const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
    t_bool* const write_value_pointer_bs__bres,
    constants__t_WriteValuePointer_i* const write_value_pointer_bs__p_write_value_copy)
{
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    OpcUa_WriteValue* wv = write_value_pointer_bs__p_write_value;
    OpcUa_WriteValue* writeValueCopy = SOPC_Malloc(sizeof(OpcUa_WriteValue));
    OpcUa_WriteValue_Initialize((void*) writeValueCopy);

    *write_value_pointer_bs__bres = false;
    *write_value_pointer_bs__p_write_value_copy = constants__c_WriteValuePointer_indet;

    if (NULL != writeValueCopy)
    {
        retStatus = SOPC_NodeId_Copy(&writeValueCopy->NodeId, &wv->NodeId);

        writeValueCopy->AttributeId = wv->AttributeId;

        if (SOPC_STATUS_OK == retStatus)
        {
            retStatus = SOPC_String_Copy(&writeValueCopy->IndexRange, &wv->IndexRange);
        }

        if (SOPC_STATUS_OK == retStatus)
        {
            retStatus = SOPC_DataValue_Copy(&writeValueCopy->Value, &wv->Value);
        }

        if (SOPC_STATUS_OK == retStatus)
        {
            *write_value_pointer_bs__bres = true;
            *write_value_pointer_bs__p_write_value_copy = writeValueCopy;
        }
        else
        {
            OpcUa_WriteValue_Clear(writeValueCopy);
            SOPC_Free(writeValueCopy);
        }
    }
}

void write_value_pointer_bs__get_write_value_pointer_NodeId_AttributeId(
    const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
    constants__t_NodeId_i* const write_value_pointer_bs__p_nid,
    constants__t_AttributeId_i* const write_value_pointer_bs__p_aid)
{
    *write_value_pointer_bs__p_nid = &write_value_pointer_bs__p_write_value->NodeId;
    *write_value_pointer_bs__p_aid = util_AttributeId__C_to_B(write_value_pointer_bs__p_write_value->AttributeId);
}

void write_value_pointer_bs__write_value_pointer_is_valid(
    const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
    t_bool* const write_value_pointer_bs__is_valid)
{
    *write_value_pointer_bs__is_valid = write_value_pointer_bs__p_write_value != constants__c_WriteValuePointer_indet;
}

void write_value_pointer_bs__free_write_value_pointer(
    const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value)
{
    OpcUa_WriteValue_Clear(write_value_pointer_bs__p_write_value);
    SOPC_Free(write_value_pointer_bs__p_write_value);
}
