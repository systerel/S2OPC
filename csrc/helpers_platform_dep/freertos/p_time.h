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

#ifndef FREERTOS_P_TIME_H_
#define FREERTOS_P_TIME_H_

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> /* stdlib includes */
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h" /* s2opc includes */
#include "sopc_time.h"

#include "FreeRTOS.h"       /* freeRtos includes */
#include "FreeRTOSConfig.h" /* freeRtos includes */
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

void P_TIME_SetInitialDateToBuildTime(void);

#endif /* FREERTOS_P_TIME_H_ */
