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

/** \file
 *
 * \brief Internal module to manage a list of EventIds with limited size (FIFO behavior)
 */

#ifndef LIBS2OPC_INTERNAL_EVENTID_LIST_H_
#define LIBS2OPC_INTERNAL_EVENTID_LIST_H_

#include <stdbool.h>
#include <stddef.h>

#include "sopc_builtintypes.h"

/**
 * \brief Structure representing an EventId list with limited size (FIFO behavior)
 */
typedef struct SOPC_SLinkedList SOPC_EventIdList;

/**
 * \brief Create a new EventId list with given maximum capacity
 *
 * \param maxSize   Maximum number of EventIds that can be stored in the list
 *
 * \return         Pointer to the newly created EventId list, NULL in case of allocation error
 */
SOPC_EventIdList* SOPC_EventIdList_Create(size_t maxSize);

/**
 * \brief Add a new EventId to the given list
 *        If the list is full, the oldest EventId is discarded (FIFO behavior)
 *
 * \param list     Pointer to the EventId list
 * \param eventId  EventId to add to the list (copied)
 *
 * \return         SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_PARAMETERS if list or eventId is NULL,
 *                 SOPC_STATUS_OUT_OF_MEMORY in case of allocation error
 */
SOPC_ReturnStatus SOPC_EventIdList_Add(SOPC_EventIdList* list, const SOPC_ByteString* eventId);

/**
 * \brief Check if the given EventId exists in the list
 *
 * \param list     Pointer to the EventId list
 * \param eventId  EventId to search in the list
 *
 * \return         true if EventId exists in the list, false otherwise or if list or eventId is NULL
 */
bool SOPC_EventIdList_Contains(SOPC_EventIdList* list, const SOPC_ByteString* eventId);

/**
 * \brief Clear all EventIds from the given list
 *
 * \param list     Pointer to the EventId list to clear
 */
void SOPC_EventIdList_Clear(SOPC_EventIdList* list);

/**
 * \brief Delete the EventId list content and free it
 *
 * \param list     Pointer to the EventId list to delete
 */
void SOPC_EventIdList_Delete(SOPC_EventIdList** list);

#endif /* LIBS2OPC_INTERNAL_EVENTID_LIST_H_ */
