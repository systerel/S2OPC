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

 File Name            : browse_treatment_result_it.h

 Date                 : 12/11/2019 08:48:58

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _browse_treatment_result_it_h
#define _browse_treatment_result_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 browse_treatment_result_it__currentBrowseResultIdx_i;
extern t_entier4 browse_treatment_result_it__nb_browseResult_max_refs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment_result_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment_result_it__continue_iter_browseResult(
   t_bool * const browse_treatment_result_it__p_continue,
   t_entier4 * const browse_treatment_result_it__p_browseResultIdx);
extern void browse_treatment_result_it__init_iter_browseResult(
   const t_entier4 browse_treatment_result_it__p_nb_browseResult_max_refs,
   t_bool * const browse_treatment_result_it__p_continue);

#endif
