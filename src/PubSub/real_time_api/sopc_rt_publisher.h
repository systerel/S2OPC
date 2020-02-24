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

#ifndef SOPC_RT_PUBLISHER_H_
#define SOPC_RT_PUBLISHER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_buffer.h"
#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_interrupttimer.h"

// Status of a pub instance. Mapped on IRQ TIMER STATUS
typedef enum
{
    SOPC_RT_PUBLISHER_MSG_PUB_STATUS_DISABLED = SOPC_INTERRUPT_TIMER_STATUS_DISABLED, // Timer instance is started
    SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED = SOPC_INTERRUPT_TIMER_STATUS_ENABLED,   // Timer instance is stopped
    SOPC_RT_PUBLISHER_MSG_PUB_STATUS_SIZE = SOPC_INTERRUPT_TIMER_STATUS_INVALID
} SOPC_RT_Publisher_MessageStatus;

// Definition of callback used by message publishing configuration.
// Those users callback should not be blocking.
// Start callback, called when timer switch from DISABLED to ENABLED
typedef void (*ptrCallbackStart)(uint32_t msgId,      // Message instance identifier
                                 void* pUserContext); // User context

// Stop callback, called when timer switch from ENABLED to DISABLED
typedef void (*ptrCallbackStop)(uint32_t msgId,      // Message instance identifier
                                void* pUserContext); // User context

// Elapsed callback, called when timer reach its configured period
typedef void (*ptrCallbackSend)(uint32_t msgId,     // Message instance identifier
                                void* pUserContext, // User context
                                void* pData,        // Data published by set data API
                                uint32_t size);     // Data size in bytes

// RT Publisher object
typedef struct SOPC_RT_Publisher SOPC_RT_Publisher;

// RT Publisher initializer object
typedef struct SOPC_RT_Publisher_Initializer SOPC_RT_Publisher_Initializer;

// Creation of initializer. It will be used by a RT publisher initialization. After used by RT publisher function, this
// one can be destroyed. Configuration will be initialized with Initializer_AddMessage function.
// Returns : NULL if invalid  parameters or out of memory.
SOPC_RT_Publisher_Initializer* SOPC_RT_Publisher_Initializer_Create(
    uint32_t maxSizeOfMessage); // Max size of a message, common to all message

// Destroy initializer.
void SOPC_RT_Publisher_Initializer_Destroy(SOPC_RT_Publisher_Initializer** ppInitializer);

// Adding message publishing configuration to publisher initializer
// Returns : SOPC_STATUS_OK if valid message identifier is returned.
SOPC_ReturnStatus SOPC_RT_Publisher_Initializer_AddMessage(
    SOPC_RT_Publisher_Initializer* pConfig,        // RT publisher configuration
    uint32_t period,                               // Period in heart beats
    uint32_t offset,                               // Offset in heart beats
    void* pContext,                                // User context
    ptrCallbackStart cbStart,                      // Start callback
    ptrCallbackSend cbSend,                        // Send callback
    ptrCallbackStop cbStop,                        // Stop callback
    SOPC_RT_Publisher_MessageStatus initialStatus, // initial status
    uint32_t* pOutMsgId);                          // ===> Message Identifier

// RT Publisher object creation.
SOPC_RT_Publisher* SOPC_RT_Publisher_Create(void);

// RT Publisher object destruction.
void SOPC_RT_Publisher_Destroy(SOPC_RT_Publisher** ppPubRt);

// Start RT publisher with initializer in parameter
// Returns : SOPC_STATUS_OK if switches to initialized from not initialized status.
// NOK if already initialized.
// INVALID STATE in the other status
SOPC_ReturnStatus SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub,                      // RT Publisher object
                                               SOPC_RT_Publisher_Initializer* pInitializer); // RT Publisher initializer

// Beat heart, this function invoke update function of interrupt timer. It should be called from IRQ or high priority
// task. This function invoke user callback. So, this function should not take more time that period set and don't be
// blocking.
SOPC_ReturnStatus SOPC_RT_Publisher_BeatHeart(SOPC_RT_Publisher* pPub, // RT Publisher object
                                              uint32_t tickValue);     // Cumulative tick value

// Stop RT publisher
// Returns : SOPC_STATUS_OK if switches to no initialized from initialized status.
// NOK if already stopped.
// SOPC_INVALID_STATE could be returned if API is in use or if already stopped(BeatHeart, MessageSetValue... )
// In that case, retry to stop.
SOPC_ReturnStatus SOPC_RT_Publisher_DeInitialize(SOPC_RT_Publisher* pPub);

// Start message publishing for a given message identifier. RT Publisher should be started.
SOPC_ReturnStatus SOPC_RT_Publisher_StartMessagePublishing(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                           uint32_t msgIdentifier); // Message identifier

// Stop message publishing for a given message identifier. RT Publisher should be started
SOPC_ReturnStatus SOPC_RT_Publisher_StopMessagePublishing(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                          uint32_t msgIdentifier); // Message identifier

// Configure a message in the case of already started publisher.
SOPC_ReturnStatus SOPC_RT_Publisher_ConfigureMessage(SOPC_RT_Publisher* pPub,    // RT Publisher object
                                                     uint32_t messageIdentifier, // Message identifier
                                                     uint32_t period,            // Period in heart beats
                                                     uint32_t offset,            // Offset in heart beats
                                                     void* pContext,             // User context
                                                     ptrCallbackStart cbStart,   // Start callback
                                                     ptrCallbackSend cbSend,     // Send callback
                                                     ptrCallbackStart cbStop,    // Stop callback)
                                                     SOPC_RT_Publisher_MessageStatus initialStatus); // initial status

// Get the next message publishing status, which will be taken into account by the next heart beat.
SOPC_ReturnStatus SOPC_RT_Publisher_GetMessagePubStatus(
    SOPC_RT_Publisher* pPub,                   // Publisher object
    uint32_t msgIdentifier,                    // Message identifier
    SOPC_RT_Publisher_MessageStatus* pStatus); // Next status will be taken into account by next heart beat

// Set message publishing value for a given message identifier.
// This function can be replace by GetBuffer and ReleaseBuffer if you want externally
// update data via SOPC_Buffer API.
// After start, if message value is not set, if valid period and timer started, elapsed callback will
// called with size = 0. Event if size = 0, value point always on valid space, but bytes are not significant !!!
SOPC_ReturnStatus SOPC_RT_Publisher_SetMessageValue(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                    uint32_t msgIdentifier,  // Message identifier
                                                    uint8_t* value,          // Value to publish
                                                    uint32_t size);          // Size of value

// Get buffer handle to write directly to DBO.
// Returns : SOPC_STATUS_OK : data pointer and maximum size are updated
// with pointer where write and max size allowed
SOPC_ReturnStatus SOPC_RT_Publisher_GetBuffer(SOPC_RT_Publisher* pPub, // RT Publisher object
                                              uint32_t msgIdentifier,  // message identifier
                                              SOPC_Buffer* pBuffer);   // SOPC Buffer with unallocated data pointer

// Commit data to publish and release buffer
// Returns : SOPC_STATUS_OK if data well commit.
// SOPC_Buffer contains current size to take into account.
SOPC_ReturnStatus SOPC_RT_Publisher_ReleaseBuffer(SOPC_RT_Publisher* pPub, // RT Publisher object
                                                  uint32_t msgIdentifier,  // Message identifier
                                                  SOPC_Buffer* pBuffer);   // Buffer with current size to commit

#endif /* SOPC_RT_PUBLISHER_H_ */
