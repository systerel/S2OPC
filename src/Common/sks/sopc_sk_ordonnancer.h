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

/**
 *  \file sopc_sk_ordonnancer.h
 *
 *  \brief Object to build/fill a Security Keys Manager.
 */

#ifndef SOPC_SK_ORDONNANCER_H_
#define SOPC_SK_ORDONNANCER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_ordonnancer.h"
#include "sopc_sk_provider.h"

// minimal periode for update 2s. Time use when no keys are available
#define SOPC_SK_ORDONNACER_UPDATE_TIMER_MIN 2000

// maximal periode for update ( by default no max )
#define SOPC_SK_ORDONNACER_UPDATE_TIMER_MAX UINT32_MAX

typedef struct SOPC_SKOrdonnancer SOPC_SKOrdonnancer;

typedef SOPC_ReturnStatus (*SOPC_SKOrdonnancer_AddTask_Func)(SOPC_SKOrdonnancer* sko,
                                                             SOPC_SKBuilder* skb,
                                                             SOPC_SKProvider* skp,
                                                             SOPC_SKManager* skm,
                                                             uint32_t msPeriod);
typedef SOPC_ReturnStatus (*SOPC_SKOrdonnancer_Start_Func)(SOPC_SKOrdonnancer* sko);
typedef void (*SOPC_SKOrdonnancer_StopAndClear_Func)(SOPC_SKOrdonnancer* sko);

/**
 *  \brief Bytes Security Keys Ordonnancer structure
 *
 */
struct SOPC_SKOrdonnancer
{
    SOPC_SKOrdonnancer_AddTask_Func ptrAddTask;
    SOPC_SKOrdonnancer_Start_Func ptrStart;
    SOPC_SKOrdonnancer_StopAndClear_Func ptrClear;
    void* data;
};

/**
 * \brief  Create an instance of a default SOPC_SKOrdonnancer.
 *         This ordonnancer manage only one task in a dedicated Thread
 *
 * \return a SOPC_SKOrdonnancer object or NULL if not enough memory
 */
SOPC_SKOrdonnancer* SOPC_SKOrdonnancer_Create(void);

/**
 * \brief           Creates a periodic task to call builder every msPeriod milliseconds.
 *                  Ownership of SOPC_SKBuilder and SOPC_SKProvider is moved to the SOPC_SKOrdonnancer
 *                  Note : If Builder or Provider instance should be shared, these could be wrapped in an instance with
 * a specific Clear function.
 *
 * \param sko       Pointer to Security Keys Ordonnancer. Should not be NULL
 * \param skb       Pointer to Security Keys Builder. Should not be NULL
 * \param skp       Pointer to Security Keys Provider. Should not be NULL
 * \param skm       Pointer to Security Keys Manager. Should not be NULL
 * \param msPeriod  The period in milliseconds
 * \return          SOPC_STATUS_OK if keys are set otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKOrdonnancer_AddTask(SOPC_SKOrdonnancer* sko,
                                             SOPC_SKBuilder* skb,
                                             SOPC_SKProvider* skp,
                                             SOPC_SKManager* skm,
                                             uint32_t msPeriod);

/**
 *  \brief          Start a Security KeysOrdonnancer
 *
 *  \param sko      Pointer to Security Keys Ordonnancer. Should not be NULL
 *  \return         SOPC_STATUS_OK if the SKOrdonnancer is started otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKOrdonnancer_Start(SOPC_SKOrdonnancer* sko);

/**
 *
 *  \param sko      Pointer to Security Keys Ordonnancer. Should not be NULL
 */
SOPC_ReturnStatus SOPC_SKOrdonnancer_Stop(SOPC_SKOrdonnancer* sko);

/**
 *  \brief          Stop a Security Keys Ordonnancer and deallocate data bytes content.
 *                  This object should not be used after a call to this function
 *
 *  \param sko      Pointer to Security Keys Ordonnancer. Should not be NULL
 */
void SOPC_SKOrdonnancer_StopAndClear(SOPC_SKOrdonnancer* sko);

#endif /* SOPC_SK_ORDONNANCER_H_ */
