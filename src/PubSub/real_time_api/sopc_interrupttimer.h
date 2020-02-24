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

#ifndef SOPC_INTERRUPT_TIMER_H_
#define SOPC_INTERRUPT_TIMER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_doublebuffer.h"

// Status of a timer instance
typedef enum
{
    SOPC_INTERRUPT_TIMER_STATUS_DISABLED, // Timer instance is started
    SOPC_INTERRUPT_TIMER_STATUS_ENABLED,  // Timer instance is stopped
    SOPC_INTERRUPT_TIMER_STATUS_INVALID = INT32_MAX
} SOPC_IrqTimer_InstanceStatus;

typedef struct SOPC_InterruptTimer SOPC_InterruptTimer;
typedef struct SOPC_InterruptTimer_DataHandle SOPC_InterruptTimer_DataHandle;

// Callback prototypes definitions used by SetCallback and InitAndStart

// Start callback, called when timer switch from DISABLED to ENABLED
typedef void (*sopc_irq_timer_cb_start)(uint32_t timerId,    // Timer instance identifier
                                        void* pUserContext); // User context

// Stop callback, called when timer switch from ENABLED to DISABLED
typedef void (*sopc_irq_timer_cb_stop)(uint32_t timerId,    // Timer instance identifier
                                       void* pUserContext); // User context

// Elapsed callback, called when timer reach its configured period
typedef void (*sopc_irq_timer_cb_period_elapsed)(uint32_t timerId,   // Timer instance identifier
                                                 void* pUserContext, // User context
                                                 void* pData,        // Data published by set data API
                                                 uint32_t size);     // Data size in bytes

// Create interrupt timer. This function shall be followed by initialize function to use the timer.
SOPC_InterruptTimer* SOPC_InterruptTimer_Create(void);

// Destroy interrupt timer. This function shall be called only if resetting ended with OK result.
void SOPC_InterruptTimer_Destroy(SOPC_InterruptTimer** ppTimer);

// Initialize interrupt timer
// Returns : SOPC_STATUS_INVALID_STATE if interrupt timer is in use, initializing or resetting state.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Initialize(
    SOPC_InterruptTimer* pTimer,   // Interrupt timer object
    uint32_t nbInstances,          // Maximum of timer instances (timer identifiers between 0 and nbInstances - 1)
    uint32_t maxInstanceDataSize); // Maximum of data in bytes hold by each timer

// DeInitialize interrupt timer
// Returns : SOPC_STATUS_INVALID_STATE if interrupt timer is in other state that initialized (in use, resetting,
// initializing). SOPC_STATUS_NOK for others error or already not initialized. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_DeInitialize(SOPC_InterruptTimer* pTimer);

// Reset timer and initialize it on next update
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Init(
    SOPC_InterruptTimer* pTimer,                // Interrupt timer object
    uint32_t idInstanceTimer,                   // Id of timer instance
    uint32_t period,                            // Timer Period
    uint32_t offset,                            // Timer offset
    void* pUserContext,                         // Free user context
    sopc_irq_timer_cb_start cbStart,            // Start event callback
    sopc_irq_timer_cb_period_elapsed cbElapsed, // Elapsed event callback
    sopc_irq_timer_cb_stop cbStop,
    SOPC_IrqTimer_InstanceStatus initStatus); // Stop event callback

// Force next update of timer to stop without calling any intermediate callback (stop/start/elapsed)
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DeInit(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                      uint32_t idInstanceTimer);   // Timer instance identifier

// Next update of timer will start it
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Start(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                     uint32_t idInstanceTimer);   // Timer instance identifier

// Next update of timer will stop it
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Stop(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                    uint32_t idInstanceTimer);   // Timer instance identifier

// Next update of timer will set new period
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetPeriod(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                         uint32_t idInstanceTimer,    // Timer instance identifier
                                                         uint32_t period);            // Period in ticks

// Next update of timer will set new offset
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetOffset(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                         uint32_t idInstanceTimer,    // Timer instance identifier
                                                         uint32_t offset);            // offset in ticks

// Next update of timer will set callback linked to start, stop, elapsed event
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetCallback(
    SOPC_InterruptTimer* pTimer,                // Interrupt timer object
    uint32_t idInstanceTimer,                   // Timer instance identifier
    void* pUserContext,                         // User context
    sopc_irq_timer_cb_start cbStart,            // Callback called when timer is started
    sopc_irq_timer_cb_period_elapsed cbElapsed, // Callback called when timer is elapsed
    sopc_irq_timer_cb_stop cbStop);             // Callback called when timer is stopped

// Publish new data to the timer instance, take into account by next update call.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetData(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                                       uint32_t idInstanceTimer,    // Timer instance identifier
                                                       uint8_t* pData,              // Data to publish
                                                       uint32_t sizeToWrite);       // Data size

// Create data handle for an interrupt timer object and one of its instances
SOPC_InterruptTimer_DataHandle* SOPC_InterruptTimer_Instance_DataHandle_Create(SOPC_InterruptTimer* pTimer, //
                                                                               uint32_t idInstanceTimer);   //

// Destroy data handle
void SOPC_InterruptTimer_DestroyDataContainer(SOPC_InterruptTimer_DataHandle** ppDataContainer);

// Initialize data handle used to publish data.
// This function shall be followed by Commit in order to take into account new data size
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Initialize(
    SOPC_InterruptTimer_DataHandle* pDataContainer // Data container
);

// Expose data handle via SOPC_Buffer structure
// This function shall be used between Initialize  and Finalize
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo(SOPC_InterruptTimer_DataHandle* pContainer, //
                                                                        uint32_t* pMaxAllowedSize,                  //
                                                                        uint32_t* pCurrentSize,                     //
                                                                        uint8_t** ppData);                          //

// Update data handle size via SOPC_Buffer structure currentSize field.
// This function shall be used between Initialize  and Finalize
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_SetNewSize(SOPC_InterruptTimer_DataHandle* pContainer, //
                                                                     uint32_t newSize);                          //

// Commit data container with new data
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Finalize(
    SOPC_InterruptTimer_DataHandle* pDataContainer, // Data handle object
    bool bCancel);                                  // Do not commit modification if true

// Get a timer instance status : started or stopped.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_LastStatus(
    SOPC_InterruptTimer* pTimer,           // Interrupt timer object
    uint32_t idInstanceTimer,              // Timer instance identifier
    SOPC_IrqTimer_InstanceStatus* status); // Status ENABLED or DISABLED

// Update timer tick with external tick value. Invoke callback if necessary.
// Returns : SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
// SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Update(SOPC_InterruptTimer* pTimer, // Interrupt timer object
                                             uint32_t externalTickValue); // Tick value. Shall be always incremented.

#endif
