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

#ifndef DEMO_SERVER_ALARMS_MGR_H_
#define DEMO_SERVER_ALARMS_MGR_H_

#include "sopc_builtintypes.h"

/**
 * Initialize the test server alarms and conditions.
 * This creates alarm instances and starts periodic updates of condition source values.
 *
 * @return SOPC_STATUS_OK in case of success, SOPC_STATUS_NOK or SOPC_STATUS_OUT_OF_MEMORY otherwise
 */
SOPC_ReturnStatus Demo_Server_InitializeAlarms(void);

/**
 * Stop periodic updates of condition sources values before server shutdown.
 * This cancels timers and stops the update event loop.
 */
void Demo_Server_PreStopAlarms(void);

/**
 * Clear alarm and equipment resources.
 * This frees allocated memory for equipment and alarm instances.
 */
void Demo_Server_ClearAlarms(void);

#endif /* DEMO_SERVER_ALARMS_MGR_H_ */
