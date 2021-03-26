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

#ifndef UDP_JSON_GTW_H_
#define UDP_JSON_GTW_H_

/* INCLUDES */
#include <stdbool.h>

#include "sopc_enums.h"


/* Init */

/* Configure */
SOPC_ReturnStatus UDP_Configure(void);

/* Start */
bool UDP_Start(void);

/* Stop and Clean */
void UDP_Stop(void);
void UDP_StopAndClear(void);

#endif /* UDP_JSON_GTW_H_ */
