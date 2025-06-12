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
 *  \file
 *  \brief Security Keys Scheduler: it schedules the periodic update of keys retrieved from provider
 *                                  and set in the manager using the builder.
 */

#ifndef SOPC_SK_SCHEDULER_H_
#define SOPC_SK_SCHEDULER_H_

#include "sopc_sk_scheduler.h"

#include <stdbool.h>
#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"

// minimal period for update (2s). Period used when no keys are available
#define SOPC_SK_SCHEDULER_UPDATE_TIMER_MIN 2000

// maximal period for update (by default no max)
#define SOPC_SK_SCHEDULER_UPDATE_TIMER_MAX UINT32_MAX

typedef struct SOPC_SKscheduler SOPC_SKscheduler;

typedef SOPC_ReturnStatus (*SOPC_SKscheduler_AddTask_Func)(SOPC_SKscheduler* sko,
                                                           SOPC_SKBuilder* skb,
                                                           SOPC_SKProvider* skp,
                                                           SOPC_SKManager* skm,
                                                           uint32_t msPeriod);
typedef SOPC_ReturnStatus (*SOPC_SKscheduler_Start_Func)(SOPC_SKscheduler* sko);
typedef void (*SOPC_SKscheduler_StopAndClear_Func)(SOPC_SKscheduler* sko);

/**
 *  \brief Security Keys Scheduler structure
 *
 */
struct SOPC_SKscheduler
{
    SOPC_SKscheduler_AddTask_Func ptrAddTask;
    SOPC_SKscheduler_Start_Func ptrStart;
    SOPC_SKscheduler_StopAndClear_Func ptrClear;
    void* data;
};

/**
 * \brief  Creates an instance of a default SOPC_SKscheduler.
 *         This scheduler manages only one task in a dedicated Thread
 *
 * \return a SOPC_SKscheduler object or NULL if not enough memory
 */
SOPC_SKscheduler* SOPC_SKscheduler_Create(void);

/**
 * \brief           Creates a task to call builder after \p firstMsPeriod milliseconds and then at half the keys
 *                  lifetime (or default \p SOPC_SK_SCHEDULER_UPDATE_TIMER_MIN value if no keys are available).
 *                  Ownership of SOPC_SKBuilder and SOPC_SKProvider is transferred to the SOPC_SKscheduler
 *
 * \param sko           Pointer to Security Keys Scheduler. Should not be NULL
 * \param skb           Pointer to Security Keys Builder (it might be shared by several tasks). Should not be NULL
 * \param skp           Pointer to Security Keys Provider (it might be shared by several tasks). Should not be NULL
 * \param skm           Pointer to Security Keys Manager. Should not be NULL
 * \param firstMsPeriod  The temporary period in milliseconds, it is used until the first successful update occurs.
 * \return              SOPC_STATUS_OK in case of success, otherwise a bad status (invalid parameters, out of memory).
 */
SOPC_ReturnStatus SOPC_SKscheduler_AddTask(SOPC_SKscheduler* sko,
                                           SOPC_SKBuilder* skb,
                                           SOPC_SKProvider* skp,
                                           SOPC_SKManager* skm,
                                           uint32_t firstMsPeriod);

/**
 *  \brief          Starts a Security Keys Scheduler
 *
 *  \param sko      Pointer to Security Keys Scheduler. Should not be NULL
 *  \return         SOPC_STATUS_OK if the SKscheduler is started otherwise a bad status
 */
SOPC_ReturnStatus SOPC_SKscheduler_Start(SOPC_SKscheduler* sko);

/**
 *  \brief          Stops a Security Keys Scheduler
 *
 *  \param sko      Pointer to Security Keys Scheduler. Should not be NULL
 */
SOPC_ReturnStatus SOPC_SKscheduler_Stop(SOPC_SKscheduler* sko);

/**
 *  \brief          Stops a Security Keys Scheduler and deallocate associated builders and providers,
 *                  the managers are not cleared.
 *                  This object should not be used after a call to this function
 *
 *  \param sko      Pointer to Security Keys Scheduler. Should not be NULL
 */
void SOPC_SKscheduler_StopAndClear(SOPC_SKscheduler* sko);

#endif /* SOPC_SK_SCHEDULER_H_ */
