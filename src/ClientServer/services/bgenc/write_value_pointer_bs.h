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

/******************************************************************************

 File Name            : write_value_pointer_bs.h

 Date                 : 04/08/2022 14:53:49

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _write_value_pointer_bs_h
#define _write_value_pointer_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void write_value_pointer_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void write_value_pointer_bs__copy_write_value_pointer_content(
   const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
   t_bool * const write_value_pointer_bs__bres,
   constants__t_WriteValuePointer_i * const write_value_pointer_bs__p_write_value_copy);
extern void write_value_pointer_bs__free_write_value_pointer(
   const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value);
extern void write_value_pointer_bs__get_write_value_pointer_NodeId_AttributeId(
   const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
   constants__t_NodeId_i * const write_value_pointer_bs__p_nid,
   constants__t_AttributeId_i * const write_value_pointer_bs__p_aid);
extern void write_value_pointer_bs__write_value_pointer_is_valid(
   const constants__t_WriteValuePointer_i write_value_pointer_bs__p_write_value,
   t_bool * const write_value_pointer_bs__is_valid);

#endif
