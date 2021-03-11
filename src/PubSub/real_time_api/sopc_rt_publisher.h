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

/// @file sopc_rt_publisher.h
/// @brief Real time publisher, updated by function called from IRQ, call send callback for each configured message
///
/// RT Publisher object is used to declare several messages. It calls callbacks periodically on those messages.
/// 3 callbacks are added to each message:
/// * Start callback
/// * Stop callback
/// * Elapsed period callback
///
/// For each message, a period is defined.
/// Follow those steps to create a RT Publisher:
/// * 1) Create an initializer with SOPC_RT_Publisher_Initializer_Create
/// * 2) Add messages specifications to the initializer (period, callbacks...) with
/// SOPC_RT_Publisher_Initializer_AddMessage
/// * 3) Create an RT Publisher with SOPC_RT_Publisher_Create
/// * 4) Create and initialize RT Publisher with the previous initializer with the function SOPC_RT_Publisher_Initialize
///
/// To publish data to a message, you can use SOPC_RT_Publisher_SetMessageValue.
///
/// You can also follow those steps to publish a message value via a SOPC_Buffer.
/// * 1) Initialize a standard SOPC_Buffer structure for a particular message with SOPC_RT_Publisher_GetBuffer
/// * 2) Work with your SOPC_Buffer. Do not call SOPC_Buffer_Delete on this buffer.
/// * 3) Commit modification with SOPC_RT_Publisher_ReleaseBuffer.
///
/// Publisher heart beat function SOPC_RT_Publisher_HeartBeat shall be called from an interrupt periodically.
/// Period of the interrupt shall be considered as the common granularity for each message.

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

/// @brief Status of a pub instance. Mapped on IRQ TIMER STATUS
typedef enum
{
    SOPC_RT_PUBLISHER_MSG_PUB_STATUS_DISABLED =
        SOPC_INTERRUPT_TIMER_STATUS_DISABLED,                                       ///< Message publication is started
    SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED = SOPC_INTERRUPT_TIMER_STATUS_ENABLED, ///< Message publication is stopped
    SOPC_RT_PUBLISHER_MSG_PUB_STATUS_SIZE = SOPC_INTERRUPT_TIMER_STATUS_INVALID     ///< Invalid status
} SOPC_RT_Publisher_MessageStatus;

/// @brief Definition of callback used by message publishing configuration.
/// @warning This user callback should not be blocking, and should be as short as possible.
/// Start callback, called when timer switch from DISABLED to ENABLED  after a SOPC_RT_Publisher_StartMessagePublishing
/// called.
/// @param [in] msgId Message instance identifier, returned by initializer creation steps.
/// @param [in] pUserContext User context, configured by initializer creation steps.
typedef void (*ptrCallbackStart)(uint32_t msgId,      //
                                 void* pUserContext); //

/// @brief Definition of callback used by message publishing configuration.
/// @warning This user callback should not be blocking, and should be as short as possible.
/// Stop callback, called when timer switch from ENABLED to DISABLED after a SOPC_RT_Publisher_StopMessagePublishing
/// called.
/// @param [in] msgId Message instance identifier, returned by initializer creation steps.
/// @param [in] pUserContext User context, configured by initializer creation steps.
typedef void (*ptrCallbackStop)(uint32_t msgId,      //
                                void* pUserContext); //

/// @brief Definition of callback used by message publishing configuration.
/// @warning This user callback should not be blocking, and should be as short as possible.
/// Elapsed callback, called when timer reach its configured period
/// @param [in] msgId Message instance identifier, returned by initializer creation steps.
/// @param [in] pUserContext User context, configured by initializer creation steps.
/// @param [in] pData Data published by SOPC_RT_Publisher_SetMessageValue or SOPC_RT_Publisher_ReleaseBuffer
/// @param [in] size Data size in bytes.
typedef void (*ptrCallbackSend)(uint32_t msgId,     //
                                void* pUserContext, //
                                void* pData,        //
                                uint32_t size);     //

/// @brief RT Publisher handle
typedef struct SOPC_RT_Publisher SOPC_RT_Publisher;

/// @brief RT Publisher initializer handle.
/// @warning Initializer object API is not thread safe.
typedef struct SOPC_RT_Publisher_Initializer SOPC_RT_Publisher_Initializer;

/// @brief Creation of an initializer. It will be used by a RT publisher initialization. After used by RT publisher
/// creation function, this one can be destroyed. Configuration will be initialized with
/// SOPC_RT_Publisher_Initializer_AddMessage function. After initialization of the initializer, this one shall be passed
/// to SOPC_RT_Publisher_Create function.
/// @param [in] maxSizeOfMessage Max size of a message, common to all message
/// @return NULL if invalid  parameters or out of memory.
SOPC_RT_Publisher_Initializer* SOPC_RT_Publisher_Initializer_Create(uint32_t maxSizeOfMessage); //

/// @brief Destroy initializer.
/// @param [inout] ppInitializer Initializer to destroy. Set to NULL on return.
void SOPC_RT_Publisher_Initializer_Destroy(SOPC_RT_Publisher_Initializer** ppInitializer);

/// @brief Adding message publishing configuration to publisher initializer
/// @param [in] pConfig RT publisher configuration
/// @param [in] period Period in heart beats
/// @param [in] offset Offset in heart beats
/// @param [in] pContext User context
/// @param [in] cbStart Start callback
/// @param [in] cbSend Elapsed period callback
/// @param [in] cbStop Stop callback
/// @param [in] initialStatus Message publication initial status after SOPC_RT_Publisher creation.
/// @param [out] pOutMsgId Message identifier on return
/// @return SOPC_STATUS_OK if valid message identifier is returned.
SOPC_ReturnStatus SOPC_RT_Publisher_Initializer_AddMessage(SOPC_RT_Publisher_Initializer* pConfig,        //
                                                           uint32_t period,                               //
                                                           uint32_t offset,                               //
                                                           void* pContext,                                //
                                                           ptrCallbackStart cbStart,                      //
                                                           ptrCallbackSend cbSend,                        //
                                                           ptrCallbackStop cbStop,                        //
                                                           SOPC_RT_Publisher_MessageStatus initialStatus, //
                                                           uint32_t* pOutMsgId);                          //

/// @brief RT Publisher object creation.
/// @return SOPC_RT_Publisher handle
SOPC_RT_Publisher* SOPC_RT_Publisher_Create(void);

/// @brief RT Publisher object destruction.
/// @param [inout] ppPubRt RT Publisher object to destroy. Set to NULL on return.
void SOPC_RT_Publisher_Destroy(SOPC_RT_Publisher** ppPubRt);

/// @brief Start RT publisher with initializer in parameter. Initializer shall be well initialized with all messages.
/// @param [in] pPub RT Publisher object
/// @param [in] pInitializer RT Publisher initializer
/// @return SOPC_STATUS_OK if switches to initialized from not initialized status.
/// NOK if already initialized.
/// INVALID STATE in the other status
SOPC_ReturnStatus SOPC_RT_Publisher_Initialize(SOPC_RT_Publisher* pPub,                      //
                                               SOPC_RT_Publisher_Initializer* pInitializer); //

/// @brief Heart beat, this function invoke update function of internal interrupt timer workspace. It should be called
/// from IRQ or high priority task. This function invoke user callback. So, this function should not take more time that
/// period set and don't be blocking.
/// @param [in] pPub RT Publisher object
/// @param [in] tickValue Cumulative tick value
/// @return SOPC_STATUS_OK if well initialized and not called by another process, else SOPT_STATUS_INVALID_STATE.
SOPC_ReturnStatus SOPC_RT_Publisher_HeartBeat(SOPC_RT_Publisher* pPub, //
                                              uint32_t tickValue);     //

/// @brief Stop RT publisher
/// @param [in] pPub RT Publisher object
/// @return Returns : SOPC_STATUS_OK if switches to no initialized from initialized status.
/// SOPC_STATUS_NOK if already stopped.
/// SOPC_INVALID_STATE could be returned if API is in use or if already stopped(HeartBeat, MessageSetValue... )
/// In that case, you should retry to call it.
SOPC_ReturnStatus SOPC_RT_Publisher_DeInitialize(SOPC_RT_Publisher* pPub);

/// @brief Get the next message publishing status, which will be taken into account by the next heart beat.
/// @param [in] pPub RT Publisher object
/// @param [in] msgIdentifier Message identifier returned by SOPC_RT_Publisher_Initializer_AddMessage
/// @param [out] pStatus Next status will be taken into account by next heart beat.
/// @return SOPC_STATUS_OK if status well returned
SOPC_ReturnStatus SOPC_RT_Publisher_GetMessagePubStatus(SOPC_RT_Publisher* pPub,                   //
                                                        uint32_t msgIdentifier,                    //
                                                        SOPC_RT_Publisher_MessageStatus* pStatus); //

/// @brief Set message publishing value for a given message identifier.
/// This function can be replace by SOPC_RT_Publiser_GetBuffer and OPC_RT_Publiser_ReleaseBuffer if you want externally
/// update data via SOPC_Buffer API.
/// @warning After start, if message value is not set, if valid period and timer started, elapsed callback will be
/// called with size = 0. Even if size = 0, value point always on valid space, but bytes are not significant !!!
/// @param [in] pPub RT Publisher object
/// @param [in] msgIdentifier Message identifier returned by SOPC_RT_Publisher_Initializer_AddMessage.
/// @param [in] value Value to publish
/// @param [in] size Size of value
/// @return SOPC_STATUS_OK if message value well configured.
SOPC_ReturnStatus SOPC_RT_Publisher_SetMessageValue(SOPC_RT_Publisher* pPub, //
                                                    uint32_t msgIdentifier,  //
                                                    uint8_t* value,          //
                                                    uint32_t size);          //

/// @brief Get buffer handle to write directly to DBO.
/// @param [in] pPub RT Publisher object
/// @param [in] msgIdentifier Message identifier returned by SOPC_RT_Publisher_Initializer_AddMessage
/// @param [inout] pBuffer SOPC_Buffer structure, with data pointer set to NULL.
/// @return SOPC_STATUS_OK : data pointer and maximum size are updated with pointer where write and max size allowed
SOPC_ReturnStatus SOPC_RT_Publisher_GetBuffer(SOPC_RT_Publisher* pPub, //
                                              uint32_t msgIdentifier,  //
                                              SOPC_Buffer* pBuffer);   //

/// @brief Commit data to publish and release buffer
/// @param [in] pPub RT Publisher object
/// @param [in] msgIdentifier Message identifier returned by SOPC_RT_Publisher_Initializer_AddMessage
/// @param [inout] pBuffer SOPC_Buffer structure, with data pointer set by GetBuffer and new size (number of significant
/// @param [in] bCancel Cancel commit
/// bytes). On return, data pointer is set to NULL.
/// @return SOPC_STATUS_OK if data well committed. SOPC_Buffer data pointer is set to NULL.
SOPC_ReturnStatus SOPC_RT_Publisher_ReleaseBuffer(SOPC_RT_Publisher* pPub, //
                                                  uint32_t msgIdentifier,  //
                                                  SOPC_Buffer* pBuffer,    //
                                                  bool bCancel);           //

#endif /* SOPC_RT_PUBLISHER_H_ */
