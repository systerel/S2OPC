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

/// @file sopc_msgbox.h
/// @brief Non blocking fifo based on ::SOPC_DoubleBuffer mechanism, used by ::SOPC_RT_Subscriber
/// to access received messages with several modes (simple FIFO, pop latest message, get latest message)
///
/// Implement a shared variable between one writer and several readers.
/// Access mode can be:
/// * normal : value stored into an internal fifo
/// * new last : only the most up to date value is read and removed from fifo for the concerned reader
/// * last : only the most up to date value is read but not removed from fifo for the concerned reader
/// See SOPC_MsgBox and SOPC_MsgBox_DataHandle description to see how to use API.

#ifndef SOPC_MSGBOX_H_
#define SOPC_MSGBOX_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_doublebuffer.h"

/// @brief Mode for POP function.
typedef enum SOPC_MSGBOX_MODE
{
    SOPC_MSGBOX_MODE_GET_NORMAL,   ///< Mode normal. This is the FIFO event mode.
    SOPC_MSGBOX_MODE_GET_NEW_LAST, ///< Get the most up to date pushed data and "pop it"
    SOPC_MSGBOX_MODE_GET_LAST,     ///< Get the most up to date pushed data and don't pop it.
    SOPC_MSGBOX_MODE_SIZE = INT32_MAX
} SOPC_MsgBox_Mode;

/// @brief Message box handle
/// @brief To use message box, follow the steps above:
/// @brief * 1)Create message box with SOPC_MsgBox_Create
/// @brief * 2)From writer, use SOPC_MsgBox_Push to add event, or use SOPC_MsgBox_DataHandle_XXX functions to work
/// directly on
/// @brief internal buffer
/// @brief * 3)From reader, initialize POP operation with SOPC_MsgBox_Pop_Initialize
/// @brief * 4)From reader, get event pointer with SOPC_MsgBox_Pop_GetEvtPtr
/// @brief * 5)From reader, release event point with SOPC_MsgBox_Pop_Finalize
typedef struct SOPC_MsgBox SOPC_MsgBox;

/// @brief Message box data handle writer side. Used with SOPC_MsgBox_DataHandle functions
typedef struct SOPC_MsgBox_DataHandle SOPC_MsgBox_DataHandle;

/// @brief Message Box Creation
/// @param [in] max_clients Max message box concurrent client. Client identifier between 0 to max-1
/// @param [in] max_evts Max events to manage writer burst.
/// @param [in] max_data_evt max cumulative data in bytes
/// @return NULL if invalid parameters or not enough memory. Else SOPC_MsgBox object.
SOPC_MsgBox* SOPC_MsgBox_Create(uint32_t max_clients, uint32_t max_evts, uint32_t max_data_evt);

/// @brief Message Box Destruction
/// @param [inout] ppMsgBox Message box handle to destroy. Set to NULL after free.
void SOPC_MsgBox_Destroy(SOPC_MsgBox** ppMsgBox);

/// @brief Push data to message box.
/// If you want write directly without copy into message box, use data handle.
/// @param [in] pMsgBox Message box handle
/// @param [in] data Data to push (copy input buffer into internal buffer)
/// @param [in] size Size of data
/// @return SOPC_STATUS_OK if data well pushed.
/// SOPC_INVALID_STATE if API is in use concurrently by for same client id.
/// SOPC_STATUS_NOK in case of invalid parameters : size is 0 or data is null pointer
SOPC_ReturnStatus SOPC_MsgBox_Push(SOPC_MsgBox* pMsgBox, uint8_t* data, uint32_t size);

/// @brief Reset message box
/// @param [in] pMsgBox Message box handle
/// @return Returns : SOPC_STATUS_OK if data well pushed
/// SOPC_INVALID_STATE if API is in use concurrently by for same client id.
SOPC_ReturnStatus SOPC_MsgBox_Reset(SOPC_MsgBox* pMsgBox);

/// @brief Message box pop initialization. Shall be called before SOPC_MsgBox_Pop_GetEvtPtr.
/// @param [in] pMsgBox Message box handle
/// @param [out] pIdBuffer Buffer identifier to use with SOPC_MsgBox_Pop_GetEvtPtr and SOPC_MsgBox_Pop_Finalize.
/// @return SOPC_STATUS_OK if a valid idBuffer is returned. This id shall be used by SOPC_MsgBox_Pop_GetEvtPtr function.
/// It reset idBuffer to UINT32_MAX in case of error.
SOPC_ReturnStatus SOPC_MsgBox_Pop_Initialize(SOPC_MsgBox* pMsgBox, size_t* pIdBuffer);

/// @brief Message box pop operation. Used to get an event.
/// @brief This function shall be used AFTER a Pop_Initialize.
/// @brief Multiple calls can be performed.
/// @brief After all calls, SOPC_MsgBox_Pop_Finalize shall be called.
/// @param [in] pMsgBox Message box object
/// @param [in] idBuffer Identifier returned by SOPC_MsgBox_Pop_Initialize
/// @param [in] idclient Client identifier
/// @param [out] ppData Pointer on data of an event. Shall not be freed or written.
/// @param [out] pSize Size of data event returned
/// @param [out] pNbPendOrIgnoreEvents Number of pending events
/// @param [in] mode Mode GET_NORMAL, GET_LAST or GET_NEW_LAST
/// * If mode GET_NORMAL, reader pop a new event from its point of view.
/// * If mode GET_NEW_LAST, reader pop the last event if it is new from its point of view.
/// * If mode GET_LAST, reader pop the last event even if not new (read several time the same last event)
/// @return SOPC_STATUS_OK if data is returned via ppData.
/// SOPC_INVALID_STATE if API is in use concurrently by for same client id.
SOPC_ReturnStatus SOPC_MsgBox_Pop_GetEvtPtr(SOPC_MsgBox* pMsgBox,
                                            size_t idBuffer,
                                            uint32_t idclient,
                                            uint8_t** ppData,
                                            uint32_t* pSize,
                                            uint32_t* pNbPendOrIgnoreEvents,
                                            SOPC_MsgBox_Mode mode);

/// @brief Message box pop finalization.
/// @brief Shall be called after a sequence Pop_Initialize - Pop_GetEvtPtr ... Pop_GetEvtPtr
/// @brief It reset idBuffer to UINT32_MAX value.
/// @param [in] pMsgBox Message box handle
/// @param [inout] pIdBuffer Buffer identifier returned by SOPC_MsgBox_Pop_Initialize
/// @return SOPC_STATUS_OK if buffer identifier is valid.
SOPC_ReturnStatus SOPC_MsgBox_Pop_Finalize(SOPC_MsgBox* pMsgBox, size_t* pIdBuffer);

/// @brief Message box data handle creation. Used to directly write into event buffer.
/// @warning Not tested API!
/// @param [in] pMsgBox Message box handle
/// @return Message box data handle used to expose an event buffer for write operation,
SOPC_MsgBox_DataHandle* SOPC_MsgBox_DataHandle_Create(SOPC_MsgBox* pMsgBox);

/// @brief Message box data handle - Initialization.
/// @brief After initialization, SOPC_MsgBox_DataHandle_GetDataEvt shall be used to expose buffer informations.
/// @brief SOPC_MsgBox_DataHandle_UpdateDataEvtSize is used to write number of signficant bytes contained by exposed
/// buffer.
/// @brief To commit write operation, SOPC_MsgBox_DataHandle_Finalize shall be used.
/// @param [in] pDataHandle Message box data handle.
/// @return SOPC_STATUS_OK if data handle is valid and not not initialized or finalized.
SOPC_ReturnStatus SOPC_MsgBox_DataHandle_Initialize(SOPC_MsgBox_DataHandle* pDataHandle);

/// @brief Expose event buffer information
/// @param [in] pDataHandle Message box data handle
/// @param [out] ppData Exposed data event buffer
/// @param [out] pMaxAllowedSize Max allowed data size
/// @return SOPC_STATUS_OK if event retrieved.
/// SOPC_INVALID_STATE if API is in use concurrently.
SOPC_ReturnStatus SOPC_MsgBox_DataHandle_GetDataEvt(SOPC_MsgBox_DataHandle* pDataHandle,
                                                    uint8_t** ppData,
                                                    uint32_t* pMaxAllowedSize);

/// @brief Update event data size (number of effective significant bytes)
/// @param [in] pDataHandle Message box data handle
/// @param [in] size Number of significant bytes
/// @return SOPC_STATUS_OK if event retrieved.
/// SOPC_INVALID_STATE if API is in use concurrently.
SOPC_ReturnStatus SOPC_MsgBox_DataHandle_UpdateDataEvtSize(SOPC_MsgBox_DataHandle* pDataHandle, uint32_t size);

/// @brief Message box data handle - Finalize write operation (commit modification)
/// @param [in] pDataHandle Message box data Handle
/// @param [in] bCancel Write operation canceled if true.
/// @return SOPC_STATUS_OK if event retrieved.
/// SOPC_INVALID_STATE if API is in use concurrently.
SOPC_ReturnStatus SOPC_MsgBox_DataHandle_Finalize(SOPC_MsgBox_DataHandle* pDataHandle, bool bCancel);

/// @brief Message box data handle destruction.
/// @param [inout] ppDataHandle Message box data handle. Set to NULL.
/// @return SOPC_STATUS_OK if valid handle, well finalized.
SOPC_ReturnStatus SOPC_MsgBox_DataHandle_Destroy(SOPC_MsgBox_DataHandle** ppDataHandle);

#endif /* SOPC_MSGBOX_H_ */
