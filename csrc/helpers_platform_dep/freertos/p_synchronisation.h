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

#ifndef P_SYNCHRONISATION_H
#define P_SYNCHRONISATION_H

/*****Private condition variable api*****/

#define MAX_SIGNAL (128)
#define SIGNAL_VALUE (0x80000000)

typedef struct T_ELT_TASK_LIST
{
    TaskHandle_t value;
    uint32_t uwWaitedSig;
    uint16_t nxId;
    uint16_t prId;
} tEltTaskList;

typedef struct T_CONDITION_VARIABLE
{
    QueueHandle_t handleLockCounter;
    uint16_t first;
    uint16_t nbWaiters;
    uint16_t maxWaiters;
    tEltTaskList* taskList;
} tConditionVariable;

typedef enum T_CONDITION_VARIABLE_RESULT
{
    E_COND_VAR_RESULT_OK,
    E_COND_VAR_RESULT_TIMEOUT,
    E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS,
    E_COND_VAR_RESULT_ERROR_MAX_WAITERS,
    E_COND_VAR_RESULT_ERROR_NO_WAITERS
} eConditionVariableResult;

/*Construction condition variable*/
tConditionVariable* BuildConditionVariable(unsigned short int wMaxWaiters);
void DestroyConditionVariable(tConditionVariable** ppv);

eConditionVariableResult SignalAllConditionVariable(tConditionVariable* pv, unsigned int signalValue);
eConditionVariableResult UnlockAndWaitForConditionVariable(tConditionVariable* pv,
                                                           QueueHandle_t handleMutex,
                                                           uint32_t uwSignal,
                                                           uint32_t uwTimeOutMs);

/*****Public s2opc condition variable and mutex api*****/

typedef void* Condition; // tConditionVariable*
typedef void* Mutex;     // QueueHandle_t*

#endif
