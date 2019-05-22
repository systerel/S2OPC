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
#define CLEARING_SIGNAL (0x80000000)
#define JOINTURE_SIGNAL (0x40000000)
#define SIGNAL_VALUE (0x20000000)

typedef struct T_CONDITION_VARIABLE tConditionVariable;

typedef enum T_CONDITION_VARIABLE_RESULT
{
    E_COND_VAR_RESULT_OK,
    E_COND_VAR_RESULT_ERROR_OUT_OF_MEM,
    E_COND_VAR_RESULT_TIMEOUT,
    E_COND_VAR_RESULT_ERROR_INCORRECT_PARAMETERS,
    E_COND_VAR_RESULT_ERROR_MAX_WAITERS,
    E_COND_VAR_RESULT_ERROR_NO_WAITERS,
    E_COND_VAR_RESULT_ERROR_NOT_INITIALIZED,
    E_COND_VAR_RESULT_ERROR_ALREADY_INITIALIZED
} eConditionVariableResult;

/*Construction condition variable*/

tConditionVariable* P_SYNCHRO_CreateConditionVariable(void);
void P_SYNCHRO_DestroyConditionVariable(tConditionVariable** pv);

eConditionVariableResult P_SYNCHRO_InitConditionVariable(tConditionVariable* pv, unsigned short int wMaxWaiters);
eConditionVariableResult P_SYNCHRO_ClearConditionVariable(tConditionVariable* pv);

eConditionVariableResult P_SYNCHRO_SignalAllConditionVariable(tConditionVariable* pv, unsigned int signalValue);
eConditionVariableResult P_SYNCHRO_UnlockAndWaitForConditionVariable(tConditionVariable* pv,
                                                                     QueueHandle_t handleMutex,
                                                                     unsigned int uwSignal,
                                                                     unsigned int uwTimeOutMs);

/*****Public s2opc condition variable and mutex api*****/

typedef tConditionVariable Condition; // tConditionVariable*
typedef QueueHandle_t Mutex;          // QueueHandle_t*

#endif
