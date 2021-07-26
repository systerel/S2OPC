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

#include "uam_ns_spduEncoders.h"

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_dict.h"
#include "sopc_pub_source_variable.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_target_variable.h"
#include "sopc_threads.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/
typedef struct
{
    const char* signing_key;
    const char* encryption_key;
    const char* nonce;
} SafetyDemo_interactive_SKS_Params;

typedef struct
{
    bool isProvider;
    SOPC_Dict* pCache;
    SafetyDemo_interactive_SKS_Params sks;
    SOPC_PubSubConfiguration* pConfig;
    volatile sig_atomic_t stopSignal;
    SOPC_PubSourceVariableConfig* sourceConfig;
    SOPC_SubTargetVariableConfig* targetConfig;
    UAM_SpduRequestHandle spduRequestId;
    UAM_SpduResponseHandle spduResponseId;
} SafetyDemo_interactive_Context;

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/
/**
 * \brief initialize the interactive session.
 * \param pContext The applicative context
 */
void SafetyDemo_Interactive_Initialize(SafetyDemo_interactive_Context* pContext);

/**
 * \brief Clean-up the interactive session.
 */
void SafetyDemo_Interactive_Clear(void);

/**
 * \brief Force a full refresh of the display
 */
void SafetyDemo_Interactive_ForceRefresh(void);

/***
 * The user_interactive thread.
 */
void SafetyDemo_Interactive_execute(SafetyDemo_interactive_Context* pContext);

#endif /* SOPC_SAFETY_DEMO_INTERACTIVE_H_ */
