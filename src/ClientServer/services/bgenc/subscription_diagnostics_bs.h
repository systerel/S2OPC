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

 File Name            : subscription_diagnostics_bs.h

 Date                 : 28/09/2026 12:41:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _subscription_diagnostics_bs_h
#define _subscription_diagnostics_bs_h

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
extern void subscription_diagnostics_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_diagnostics_bs__subscription_created(
   const constants__t_opcua_duration_i subscription_diagnostics_bs__p_publishInterval);
extern void subscription_diagnostics_bs__subscription_deleted(
   const constants__t_opcua_duration_i subscription_diagnostics_bs__p_publishInterval);
extern void subscription_diagnostics_bs__subscription_diagnostics_bs_UNINITIALISATION(void);
extern void subscription_diagnostics_bs__subscription_modified(
   const constants__t_opcua_duration_i subscription_diagnostics_bs__p_oldPublishInterval,
   const constants__t_opcua_duration_i subscription_diagnostics_bs__p_newPublishInterval);

#endif
