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

/// @file sopc_interrupttimer.h
///
/// @brief Interrupt timer, called from interrupt, used by ::SOPC_RT_Publisher

#ifndef SOPC_INTERRUPT_TIMER_H_
#define SOPC_INTERRUPT_TIMER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_doublebuffer.h"

/// @brief Status of a timer instance
typedef enum
{
    SOPC_INTERRUPT_TIMER_STATUS_DISABLED, ///< Timer instance is stopped
    SOPC_INTERRUPT_TIMER_STATUS_ENABLED,  ///< Timer instance is started
    SOPC_INTERRUPT_TIMER_STATUS_INVALID = INT32_MAX
} SOPC_IrqTimer_InstanceStatus;

/// @brief Handle of an interrupt timer workspace.
/// For one interrupt timer workspace, several timer instances can be instantiated,
/// Created by SOPC_InterruptTimer_Create, then used as following steps:
/// * Initialize an instance by SOPC_InterruptTimer_Initialize (period, callback called on elapsed period...)
/// * Publish new data to the instance by SOPC_InterruptTimer_SetData
///
/// The function SOPC_InterruptTimer_Update shall be called from periodic process or interrupt.
typedef struct SOPC_InterruptTimer SOPC_InterruptTimer;

/// @brief Handle of a interrupt timer data container.
/// This handle is used if you want work directly with buffer of an interrupt timer instance.
/// The buffer is externally exposed by this handle, linked to a SOPC_InterruptTimer + timer instance identifier.
/// Created by SOPC_InterruptTimer_DataHandle_Create, which links it to one workspace and one instance, then those steps
/// shall be followed to publish data:
/// * Initialized by SOPC_InterruptTimer_DataHandle_Initialize.
/// * Used by SOPC_InterruptTimer_DataHandle_GetBufferInfo and SOPC_InterruptTimer_DataHandle_SetNewSize
/// * Modification committed by SOPC_InterruptTimer_DataHandle_Finalize.
typedef struct SOPC_InterruptTimer_DataHandle SOPC_InterruptTimer_DataHandle;

// Callback prototypes definitions used by SetCallback and InitAndStart

/// @brief Start callback, called when timer switch from DISABLED to ENABLED
/// @param [in] timerId Timer instance identifier, Between 0 and number of possible instance configured by
/// SOPC_IniterruptTimer_Initialize
/// @param [in] pUserContext User context
typedef void (*sopc_irq_timer_cb_start)(uint32_t timerId,    //
                                        void* pUserContext); //

/// @brief Stop callback, called when timer switch from ENABLED to DISABLED
/// @param [in] timerId Timer instance identifier. Between 0 and number of possible instance configured by
/// SOPC_IniterruptTimer_Initialize.
/// @param [in] pUserContext User context
typedef void (*sopc_irq_timer_cb_stop)(uint32_t timerId,    //
                                       void* pUserContext); //

/// @brief Elapsed callback, called when timer reach its configured period
/// @param [in] timerId Timer instance identifier
/// @param [in] pUserContext User context
/// @param [in] pData Data published by SOPC_InterruptTimer_Instance_SetData
/// @param [in] Data size in bytes
typedef void (*sopc_irq_timer_cb_period_elapsed)(uint32_t timerId,   //
                                                 void* pUserContext, //
                                                 void* pData,        //
                                                 uint32_t size);     //

/// @brief Create interrupt timer. This function shall be followed by initialize function to use the timer.
/// @warning Not thread safe!
/// @return Returns pointer on SOPC_InterruptTimer.
SOPC_InterruptTimer* SOPC_InterruptTimer_Create(void);

/// @brief Destroy interrupt timer. This function shall be called only if SOPC_InterruptTimer_DeInitialize returns
/// SOPC_STATUS_OK.
/// @warning Not thread safe!
/// @param [inout] ppTimer Address from where SOPC_InterruptTimer object will be deleted. Set to NULL.
void SOPC_InterruptTimer_Destroy(SOPC_InterruptTimer** ppTimer);

/// @brief Initialize interrupt timer workspace
/// @param [in] pTimer Interrupt timer object
/// @param [in] nbInstances Maximum of timer instances (timer identifiers will be between 0 and nbInstances - 1)
/// @param [in] maxInstanceDataSize Maximum of data in bytes hold by each timer
/// @return SOPC_STATUS_INVALID_STATE if interrupt timer is in use, initializing or resetting state.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Initialize(SOPC_InterruptTimer* pTimer,   //
                                                 uint32_t nbInstances,          //
                                                 uint32_t maxInstanceDataSize); //

/// @brief DeInitialize interrupt timer workspace
/// @param [in] pTimer Interrupt timer object
/// @return SOPC_STATUS_INVALID_STATE if interrupt timer is in other state that initialized (in use, resetting,
/// @return initializing). SOPC_STATUS_NOK for others error or already not initialized. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_DeInitialize(SOPC_InterruptTimer* pTimer);

/// @brief Reset timer instance and initialize it on next SOPC_InterruptTimer_Update call
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @param [in] period Timer period (tick)
/// @param [in] offset Timer offset (tick)
/// @param [in] pUserContext Use context
/// @param [in] cbStart Start event callback
/// @param [in] cbElapsed Elapsed event callback
/// @param [in] cbStop Stop event callback
/// @param [in] initStatus Initial status, set STARTED or STOPPED status
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Init(SOPC_InterruptTimer* pTimer,                //
                                                    uint32_t idInstanceTimer,                   //
                                                    uint32_t period,                            //
                                                    uint32_t offset,                            //
                                                    void* pUserContext,                         //
                                                    sopc_irq_timer_cb_start cbStart,            //
                                                    sopc_irq_timer_cb_period_elapsed cbElapsed, //
                                                    sopc_irq_timer_cb_stop cbStop,              //
                                                    SOPC_IrqTimer_InstanceStatus initStatus);   //

/// @brief Force next update of timer to stop without calling any intermediate callback (stop/start/elapsed)
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DeInit(SOPC_InterruptTimer* pTimer, //
                                                      uint32_t idInstanceTimer);   //

/// @brief Next update of timer instance will start it
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Start(SOPC_InterruptTimer* pTimer, //
                                                     uint32_t idInstanceTimer);   //

/// @brief Next update of timer will stop it
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_Stop(SOPC_InterruptTimer* pTimer, //
                                                    uint32_t idInstanceTimer);   //

/// @brief Next update of timer will set new period
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @param [in] period Period in ticks
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetPeriod(SOPC_InterruptTimer* pTimer, //
                                                         uint32_t idInstanceTimer,    //
                                                         uint32_t period);            //

/// @brief Next update of timer will set new offset
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @param [in] offset Offset in ticks
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetOffset(SOPC_InterruptTimer* pTimer, //
                                                         uint32_t idInstanceTimer,    //
                                                         uint32_t offset);            //

/// @brief Next update of timer will set callback linked to start, stop, elapsed event
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @param [in] pUserContext Use context
/// @param [in] cbStart Callback called when timer is started
/// @param [in] cbElapsed Callback called when timer is elapsed
/// @param [in] cbStop Callback called when timer is stopped
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetCallback(SOPC_InterruptTimer* pTimer,                //
                                                           uint32_t idInstanceTimer,                   //
                                                           void* pUserContext,                         //
                                                           sopc_irq_timer_cb_start cbStart,            //
                                                           sopc_irq_timer_cb_period_elapsed cbElapsed, //
                                                           sopc_irq_timer_cb_stop cbStop);             //

/// @brief Publish new data to the timer instance, take into account by next update call.
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Timer instance identifier
/// @param [in] pData Data to publish
/// @param [in] sizeToWrite Data size
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_SetData(SOPC_InterruptTimer* pTimer, //
                                                       uint32_t idInstanceTimer,    //
                                                       uint8_t* pData,              //
                                                       uint32_t sizeToWrite);       //

/// @brief Create data handle for an interrupt timer workspace and one of its instances
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer
/// @return SOPC_InterruptTimer_DataHandle object
SOPC_InterruptTimer_DataHandle* SOPC_InterruptTimer_Instance_DataHandle_Create(SOPC_InterruptTimer* pTimer, //
                                                                               uint32_t idInstanceTimer);   //

/// @brief Destroy data handle
/// @param [inout] ppDataContainer Address where Interrupt timer data handle shall be destroyed. Set to NULL.
void SOPC_InterruptTimer_DestroyDataContainer(SOPC_InterruptTimer_DataHandle** ppDataContainer);

/// @brief Initialize data handle used to publish data.
/// @warning This function shall be called before GetBufferInfo and SetNewSize
/// @warning After modification, SOPC_InterruptTimer_DataHandle_Finalize shall be called in order to take into account
/// new data with new size
/// @param [in] pDataContainer Interrupt timer data handle
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Initialize(SOPC_InterruptTimer_DataHandle* pDataContainer //
);

/// @brief Expose data handle buffer informations
/// @warning This function shall be used between SOPC_InterruptTimer_Instance_DataHandle_Initialize  and
/// SOPC_InterruptTimer_Instance_DataHandle_Finalize
/// @warning User becomes the "owner" of the exposed data. He doesn't shall write above *pMaxAllowedSize.
/// @param [in] pContainer Interrupt timer instance data handle
/// @param [out] pMaxAllowedSize Function returns max allowed size of returned exposed buffer
/// @param [out] pCurrentSize Function returns current significant bytes present in the returned exposed buffer
/// @param [out] ppData Address where is returned pointer on exposed buffer. Do not free this value.
/// @return SOPC_STATUS_OK in case of success.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo(SOPC_InterruptTimer_DataHandle* pContainer,
                                                                        uint32_t* pMaxAllowedSize,
                                                                        size_t* pCurrentSize,
                                                                        uint8_t** ppData);

/// @brief Update data handle size with newSize parameter.
/// @warning This function shall be used between SOPC_InterruptTimer_Instance_DataHandle_Initialize  and
/// SOPC_InterruptTimer_Instance_DataHandle_Finalize
/// @param [in] pContainer Interrupt timer instance data handle
/// @param [in] newSize Significant bytes to take into account.
/// @return SOPC_STATUS_OK in case of success.
/// @return SOPC_INVALID_PARAMETERS if newSize > max allowed size returned by
/// SOPC_InterruptTimer_Instance_DataHandle_GetBufferInfo
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_SetNewSize(SOPC_InterruptTimer_DataHandle* pContainer,
                                                                     size_t newSize);

/// @brief Commit data container with new data and its new size
/// @param [in] pDataContainer Interrupt timer instance data handle
/// @param [in] bCancel Modification canceled
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_DataHandle_Finalize(SOPC_InterruptTimer_DataHandle* pDataContainer, //
                                                                   bool bCancel);                                  //

/// @brief Get a timer instance status : started or stopped.
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] idInstanceTimer Interrupt timer instance
/// @param [out] status Address where is returned timer instance status
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Instance_LastStatus(SOPC_InterruptTimer* pTimer,           //
                                                          uint32_t idInstanceTimer,              //
                                                          SOPC_IrqTimer_InstanceStatus* status); //

/// @brief Update timer tick with external tick value. Invoke callback if necessary.
/// @param [in] pTimer Interrupt timer workspace
/// @param [in] externalTickValue value. Shall be always incremented. Internally, tick value is declared as uint64_t
/// @return SOPC_STATUS_INVALID_STATE if workspace is not initialized or API for this instance is currently in use.
/// @return SOPC_STATUS_NOK for others error. Else SOPC_STATUS_OK.
SOPC_ReturnStatus SOPC_InterruptTimer_Update(SOPC_InterruptTimer* pTimer, //
                                             uint32_t externalTickValue); //

#endif
