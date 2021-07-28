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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/

/** \file Provides a simple interactive terminal so as to allow "dynamic" and manual tests.
 * Uses the VT-100 features to create sub-windows, and listens to STDIN for
 * user specific commands.
 */

#ifndef SOPC_SAFETY_DEMO_INTERACTIVE_H_
#define SOPC_SAFETY_DEMO_INTERACTIVE_H_

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_dict.h"
#include "sopc_threads.h"

#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/
typedef bool (*pfUtils_Interactive_Event)(const char* params, void* pUserParam);

/*============================================================================
 * EXTERNAL SERVICES - INTERACTIVE
 *===========================================================================*/
/**
 * \brief initialize the interactive session.
 */
void Utils_Interactive_Initialize(void);

/**
 * \brief Clean-up the interactive session.
 */
void Utils_Interactive_Clear(void);

/**
 * \brief Register an interactive command
 * \param key The command key trigerring a call to pfEvent
 * \param pfEvent the event to call
 * \param pDescr A short human-readable description of the function
 * \param pUserParam A user defined pointer that will be passed to pfUtils_Interactive_Event
 */
void Utils_Interactive_AddCallback(const char key, const char* pDescr, pfUtils_Interactive_Event pfEvent, void* pUserParam);

/**
 * \brief Force a full refresh of the display
 */
void Utils_Interactive_ForceRefresh(void);

/***
 * Show basic helpd.
 */
void Utils_Interactive_printHelp(void);

/***
 * The user_interactive thread.
 */
void Utils_Interactive_execute(void);


/*============================================================================
 * EXTERNAL SERVICES - DISPLAY
 *===========================================================================*/
void Utils_Interactive_displayPushPosition(void);
void Utils_Interactive_displayPopPosition(void);

void Utils_Interactive_displayClearEOL(void);
void Utils_Interactive_displaySetCursorLocation(const UAS_UInt32 x, const UAS_UInt32 y);
void Utils_Interactive_displayClearScreen(void);
#endif /* SOPC_SAFETY_DEMO_INTERACTIVE_H_ */
