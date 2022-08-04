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

 File Name            : argument_pointer_bs.h

 Date                 : 04/08/2022 14:53:28

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _argument_pointer_bs_h
#define _argument_pointer_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void argument_pointer_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void argument_pointer_bs__read_argument_type(
   const constants__t_Argument_i argument_pointer_bs__p_arg,
   constants__t_NodeId_i * const argument_pointer_bs__p_type);
extern void argument_pointer_bs__read_argument_valueRank(
   const constants__t_Argument_i argument_pointer_bs__p_arg,
   t_entier4 * const argument_pointer_bs__p_vr);
extern void argument_pointer_bs__read_variant_argument(
   const constants__t_Variant_i argument_pointer_bs__p_variant,
   const t_entier4 argument_pointer_bs__p_index,
   constants__t_Argument_i * const argument_pointer_bs__p_arg);
extern void argument_pointer_bs__read_variant_nb_argument(
   const constants__t_Variant_i argument_pointer_bs__p_variant,
   const constants__t_Node_i argument_pointer_bs__p_node,
   t_entier4 * const argument_pointer_bs__p_nb,
   t_bool * const argument_pointer_bs__p_bres);

#endif
