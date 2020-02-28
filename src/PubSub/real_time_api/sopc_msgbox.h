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

#ifndef SOPC_MAILBOX_H_
#define SOPC_MAILBOX_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_doublebuffer.h"

typedef enum SOPC_MSGBOX_MODE
{
    SOPC_MSGBOX_MODE_GET_NORMAL,   // Mode normal. This is the FIFO event mode.
    SOPC_MSGBOX_MODE_GET_NEW_LAST, // Get the most up to data pushed data and "pop it"
    SOPC_MSGBOX_MODE_GET_LAST,     // Get the most up to date pushed data and don't pop it.
    SOPC_MSGBOX_MODE_SIZE = INT32_MAX
} SOPC_MsgBox_Mode;

// Message box class
typedef struct SOPC_MsgBox SOPC_MsgBox;

// Message box data handle writer side. Used with SOPC_MsgBox_DataHandle functions
typedef struct SOPC_MsgBox_DataHandle SOPC_MsgBox_DataHandle;

// Message Box Creation
// Returns : NULL if invalid parameters or not enough memory. Else SOPC_MsgBox object.
SOPC_MsgBox* SOPC_MsgBox_Create(
    uint32_t max_clients,   // Max message box concurrent client. Client identifier between 0 -> max-1
    uint32_t max_evts,      // Max events to manage writer burst.
    uint32_t max_data_evt); // Max cumulative data in bytes.

// Message Box Destruction
void SOPC_MsgBox_Destroy(SOPC_MsgBox** ppMsgBox);

// Push data to message box
// Returns : SOPC_STATUS_OK if data well pushed
// SOPC_INVALID_STATE if API is in use concurrently by for same client id.
// SOPC_STATUS_NOK in case of invalid parameters : size < 1, data = NULL, to big atomic data size
SOPC_ReturnStatus SOPC_MsgBox_Push(SOPC_MsgBox* pMsgBox, // Message box
                                   uint8_t* data,        // Data to push
                                   uint32_t size);       // Size of data

// Reset message box
// Returns : SOPC_STATUS_OK if data well pushed
// SOPC_INVALID_STATE if API is in use concurrently by for same client id.
SOPC_ReturnStatus SOPC_MsgBox_Reset(SOPC_MsgBox* pMsgBox);

// Message box pop initialization. Shall be called before Pop_GetEvtPtr.
// Returns : SOPC_STATUS_OK if a valid idBuffer is returned. This id shall be used by GetEvtPtr function.
// It reset idBuffer to UINT32_MAX in case of error.
SOPC_ReturnStatus SOPC_MsgBox_Pop_Initialize(
    SOPC_MsgBox* pMsgBox, // Message box object
    uint32_t* pIdBuffer); // Buffer identifier to use with Pop_GetEvtPtr and Pop_Finalize

// Message box pop operation. Used to get an event.
// This function shall be used AFTER a Pop_Initialize.
// Multiple calls can be performed.
// After all calls, Pop_Finalize shall be called.
// Returns : SOPC_STATUS_OK if data is returned via ppData
// If mode GET_NORMAL, reader pop a new event from its point of view.
// If mode GET_NEW_LAST, reader pop the last event if it is new from its point of view.
// If mode GET_LAST, reader pop the last event even if not new (read several time the same last event)
SOPC_ReturnStatus SOPC_MsgBox_Pop_GetEvtPtr(SOPC_MsgBox* pMsgBox,            // Message box object
                                            uint32_t handlePopSequence,      // Identifier returned by Pop_Initialize
                                            uint32_t idclient,               // Client identifier
                                            uint8_t** ppData,                // Data of an event
                                            uint32_t* pSize,                 // Size of data event returned
                                            uint32_t* pNbPendOrIgnoreEvents, // Number of pending events
                                            SOPC_MsgBox_Mode mode); // Mode GET_NORMAL, GET_LAST or GET_NEW_LAST

// Message box pop finalization.
// Shall be called after a sequence Pop_Initialize - Pop_GetEvtPtr ... Pop_GetEvtPtr
// It reset idBuffer to UINT32_MAX value.
SOPC_ReturnStatus SOPC_MsgBox_Pop_Finalize(SOPC_MsgBox* pMsgBox, // Message box object
                                           uint32_t* pIdBuffer); // @ point on uin32_t id returned by Pop_Initialize

// Not tested API. Used to expose directly event buffer for write operation.
SOPC_MsgBox_DataHandle* SOPC_MsgBox_DataHandle_Create(SOPC_MsgBox* pMsgBox);

SOPC_ReturnStatus SOPC_MsgBox_DataHandle_Initialize(SOPC_MsgBox_DataHandle* pDataHandle);

SOPC_ReturnStatus SOPC_MsgBox_DataHandle_GetDataEvt(SOPC_MsgBox_DataHandle* pDataHandle, //
                                                    uint8_t** ppData,                    //
                                                    uint32_t* pMaxAllowedSize);          //

SOPC_ReturnStatus SOPC_MsgBox_DataHandle_UpdateDataEvtSize(SOPC_MsgBox_DataHandle* pDataHandle, //
                                                           uint32_t size);                      //

SOPC_ReturnStatus SOPC_MsgBox_DataHandle_Finalize(SOPC_MsgBox_DataHandle* pDataHandle, //
                                                  bool bCancel);                       //

SOPC_ReturnStatus SOPC_MsgBox_DataHandle_Destroy(SOPC_MsgBox_DataHandle** ppDataHandle);

#endif /* SOPC_MAILBOX_H_ */
