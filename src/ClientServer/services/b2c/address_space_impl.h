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

/** \file
 *
 * \brief Declares initialisable members of the AddressSpace
 *        and OPC-UA values of B constants.
 */

#ifndef ADDRESS_SPACE_IMPL_H_
#define ADDRESS_SPACE_IMPL_H_

#include "b2c.h"
#include "constants.h"
#include "sopc_address_space.h"
#include "sopc_types.h"

/* Attributes, and references */
extern SOPC_AddressSpace* address_space_bs__nodes;

/* Address space configured */
extern bool sopc_addressSpace_configured;

void SOPC_AddressSpace_Check_Configured(void);

#endif /* ADDRESS_SPACE_IMPL_H_ */
