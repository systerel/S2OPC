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

 File Name            : translate_browse_path_source.h

 Date                 : 24/07/2023 14:29:25

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _translate_browse_path_source_h
#define _translate_browse_path_source_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "translate_browse_path_source_1.h"
#include "translate_browse_path_source_1_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "node_id_pointer_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void translate_browse_path_source__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define translate_browse_path_source__get_BrowsePathSource translate_browse_path_source_1__get_BrowsePathSource
#define translate_browse_path_source__get_BrowsePathSourceSize translate_browse_path_source_1__get_BrowsePathSourceSize

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void translate_browse_path_source__free_BrowsePathSource(void);
extern void translate_browse_path_source__update_one_browse_path_source(
   const constants__t_NodeId_i translate_browse_path_source__source,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_source__statusCode_operation);

#endif
