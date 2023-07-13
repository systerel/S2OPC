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

#ifndef P_UTILS_H
#define P_UTILS_H

#include "sopc_enums.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define MAX_P_UTILS_LIST (16)

typedef struct T_TASK_LIST_ELT
{
    /** NULL value is reserved for empty list elements */
    TaskHandle_t value;
    void* pContext;
    uint32_t infosField1;
    uint32_t infosField2;
    uint16_t nxId;
    uint16_t prId;
} tUtilsListElt;

typedef struct T_TASK_LIST
{
    tUtilsListElt* list;
    uint16_t firstValid;
    uint16_t firstFreeNextOQP;
    uint16_t firstFreePreviousOQP;
    uint16_t firstFree;
    uint16_t wMaxWaitingTasks;
    uint16_t wNbRegisteredTasks;
    SemaphoreHandle_t lockHandle;
} tUtilsList;

/* Non Thread safe private list api */

uint16_t P_UTILS_LIST_GetEltIndex(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);
uint16_t P_UTILS_LIST_GetNbEltMT(tUtilsList* ptr);

/* \brief Adds an element to the list, if possible. \p handleTask can't be NULL. */
SOPC_ReturnStatus P_UTILS_LIST_AddElt(tUtilsList* ptr,
                                      TaskHandle_t handleTask,
                                      void* pContext,
                                      uint32_t infos1,
                                      uint32_t infos2);

/** \brief Enumerate and parse elements of the list.
 *
 * Continue the enumeration from \p pCurrentSlotId and updates it to the index of the next non empty element.
 * \p pCurrentSlotId can be set to UINT16_MAX to initiate the enumerator.
 * Returns NULL and sets \p pCurrentSlotId to UINT16_MAX when enumeration is finished.
 */
TaskHandle_t P_UTILS_LIST_ParseValueElt(tUtilsList* ptr,
                                        uint32_t* pOutValue,
                                        uint32_t* pOutValue2,
                                        void** ppOutContext,
                                        uint16_t* pCurrentSlotId);

void* P_UTILS_LIST_ParseContextElt(tUtilsList* ptr, uint16_t* pCurrentSlotId);

uint16_t P_UTILS_LIST_RemoveElt(tUtilsList* pv,
                                TaskHandle_t taskNotified,
                                uint32_t infos1,
                                uint32_t infos2,
                                uint16_t* pOutNextOQPSlot);

void P_UTILS_LIST_DeInit(tUtilsList* ptr);

SOPC_ReturnStatus P_UTILS_LIST_Init(tUtilsList* ptr, uint16_t wMaxRDV);

/* Thread safe private list api */

uint16_t P_UTILS_LIST_GetEltIndexMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

void* P_UTILS_LIST_GetContextFromHandleMT(tUtilsList* ptr, TaskHandle_t taskNotified, uint32_t infos1, uint32_t infos2);

TaskHandle_t P_UTILS_LIST_ParseValueEltMT(tUtilsList* ptr,
                                          uint32_t* pOutValue,
                                          uint32_t* pOutValue2,
                                          void** ppOutContext,
                                          uint16_t* pCurrentSlotId);

uint16_t P_UTILS_LIST_GetNbEltMT(tUtilsList* ptr);

SOPC_ReturnStatus P_UTILS_LIST_AddEltMT(tUtilsList* ptr,
                                        TaskHandle_t handleTask,
                                        void* pContext,
                                        uint32_t infos,
                                        uint32_t info2);

void* P_UTILS_LIST_ParseContextEltMT(tUtilsList* ptr, uint16_t* pCurrentSlotId);

uint16_t P_UTILS_LIST_RemoveEltMT(tUtilsList* pv,
                                  TaskHandle_t taskNotified,
                                  uint32_t infos1,
                                  uint32_t infos2,
                                  uint16_t* pOutNextOQPSlot);

void P_UTILS_LIST_DeInitMT(tUtilsList* ptr);

SOPC_ReturnStatus P_UTILS_LIST_InitMT(tUtilsList* ptr, uint16_t wMaxRDV);

void P_UTILS_LIST_DeInitMT(tUtilsList* ptr);

#ifdef FOLLOW_ALLOC
extern uint32_t cptAlloc;
extern uint32_t cptFree;
extern QueueHandle_t cptMutex;
void DEBUG_incrementCpt(void);
void DEBUG_decrementCpt(void);
#else
#define DEBUG_incrementCpt()
#define DEBUG_decrementCpt()
#endif

#endif
