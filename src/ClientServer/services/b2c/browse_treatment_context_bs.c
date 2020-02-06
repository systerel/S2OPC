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

#include "browse_treatment_context_bs.h"

#define NODECLASS_MASK_OBJECT 1          // bit 0
#define NODECLASS_MASK_VARIABLE 2        // bit 1
#define NODECLASS_MASK_METHOD 4          // bit 2
#define NODECLASS_MASK_OBJECT_TYPE 8     // bit 3
#define NODECLASS_MASK_VARIABLE_TYPE 16  // bit 4
#define NODECLASS_MASK_REFERENCE_TYPE 32 // bit 5
#define NODECLASS_MASK_DATA_TYPE 64      // bit 6
#define NODECLASS_MASK_VIEW 128          // bit 7

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_context_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_context_bs__is_NodeClass_in_NodeClassMask_bs(
    const constants__t_NodeClass_i browse_treatment_context_bs__p_nodeClass,
    const constants__t_BrowseNodeClassMask_i browse_treatment_context_bs__p_nodeClassMask,
    t_bool* const browse_treatment_context_bs__bres)
{
    const constants__t_BrowseNodeClassMask_i mask = browse_treatment_context_bs__p_nodeClassMask;

    if (0 == mask)
    {
        // It means no mask, do not evaluate the actual NodeClass value:
        *browse_treatment_context_bs__bres = true;
        return;
    }

    // NodeClassMask is not '*', evaluate the NodeClass value:
    switch (browse_treatment_context_bs__p_nodeClass)
    {
    case constants__e_ncl_Object:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_OBJECT);
        break;
    case constants__e_ncl_Variable:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_VARIABLE);
        break;
    case constants__e_ncl_Method:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_METHOD);
        break;
    case constants__e_ncl_ObjectType:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_OBJECT_TYPE);
        break;
    case constants__e_ncl_VariableType:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_VARIABLE_TYPE);
        break;
    case constants__e_ncl_ReferenceType:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_REFERENCE_TYPE);
        break;
    case constants__e_ncl_DataType:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_DATA_TYPE);
        break;
    case constants__e_ncl_View:
        *browse_treatment_context_bs__bres = 0 != (mask & NODECLASS_MASK_VIEW);
        break;
    default:
        *browse_treatment_context_bs__bres = false;
    }
}
