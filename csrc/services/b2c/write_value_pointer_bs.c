/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*--------------
   SEES Clause
  --------------*/
#include "write_value_pointer_bs.h"

#include "stdbool.h"

#include "sopc_types.h"
#include "util_b2c.h"

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
    OpcUa_WriteValue* writeValueCopy = malloc(sizeof(OpcUa_WriteValue));
    OpcUa_WriteValue_Initialize(&writeValueCopy);

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
            OpcUa_WriteValue_Clear(&writeValueCopy);
            free(writeValueCopy);
        }
    }
}

void write_value_pointer_bs__get_write_value_pointer_NodeId_AttributeId(
    const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
    constants__t_NodeId_i* const write_value_pointer_bs__p_nid,
    constants__t_AttributeId_i* const write_value_pointer_bs__p_aid)
{
    *write_value_pointer_bs__p_nid = &write_value_pointer_bs__p_write_value->NodeId;
    util_AttributeId__C_to_B(write_value_pointer_bs__p_write_value->AttributeId, write_value_pointer_bs__p_aid);
}

void write_value_pointer_bs__write_value_pointer_is_valid(
    const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
    t_bool* const write_value_pointer_bs__is_valid)
{
    *write_value_pointer_bs__is_valid = write_value_pointer_bs__p_write_value != constants__c_WriteValuePointer_indet;
}
