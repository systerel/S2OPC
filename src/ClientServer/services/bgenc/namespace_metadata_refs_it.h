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

 File Name            : namespace_metadata_refs_it.h

 Date                 : 14/08/2024 16:16:28

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _namespace_metadata_refs_it_h
#define _namespace_metadata_refs_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_Node_i namespace_metadata_refs_it__Node;
extern t_entier4 namespace_metadata_refs_it__RefIndex;
extern t_entier4 namespace_metadata_refs_it__RefIndexEnd;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void namespace_metadata_refs_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void namespace_metadata_refs_it__continue_iter_namespacemetadata_refs(
   t_bool * const namespace_metadata_refs_it__p_continue,
   constants__t_Reference_i * const namespace_metadata_refs_it__p_ref);
extern void namespace_metadata_refs_it__init_iter_namespacemetadata_refs(
   const constants__t_Node_i namespace_metadata_refs_it__p_node,
   t_bool * const namespace_metadata_refs_it__p_continue);

#endif
