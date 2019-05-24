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

#ifndef SOPC_P_THREADS_H_
#define SOPC_P_THREADS_H_

// typedef struct T_THREAD_WKS tThreadWks;

typedef void (*tPtrFct)(void*);

/*****Private thread api*****/

typedef struct T_THREAD_ARGS
{
    tPtrFct cbExternalCallback;
    void* ptrStartArgs;
    tPtrFct cbWaitingForJoin;
    tPtrFct cbReadyToSignal;
    TaskHandle_t handleTask;
    QueueHandle_t lockRecHandle;
    QueueHandle_t signalReadyToWait;
    tConditionVariable* pJointure;
} tThreadArgs;

typedef struct T_THREAD_WKS
{
    tUtilsList taskList;
    tThreadArgs args;
} tThreadWks;

typedef enum E_THREAD_RESULT
{
    E_THREAD_RESULT_OK,
    E_THREAD_RESULT_ERROR_NOK,
    E_THREAD_RESULT_ERROR_NOT_INITIALIZED,
    E_THREAD_RESULT_ERROR_SELF_JOIN_THREAD,
    E_THREAD_RESULT_ERROR_ALREADY_INITIALIZED
} eThreadResult;

tThreadWks* P_THREAD_Create(tPtrFct fct, void* args, tPtrFct fctWatingForJoin, tPtrFct fctReadyToSignal);
eThreadResult P_THREAD_Init(tThreadWks* p, unsigned short wMaxJoin);
eThreadResult P_THREAD_Join(tThreadWks* p);

/*****Public s2opc thread api*****/

typedef tThreadWks Thread;

#endif /* SOPC_P_THREADS_H_ */
