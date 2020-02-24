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

#ifndef SOPC_RT_SUBSCRIBER_H_
#define SOPC_RT_SUBSCRIBER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_mailbox.h"

// Pin direction enumeration.
typedef enum SOPC_Pin_Direction
{
    SOPC_PIN_DIRECTION_IN,  // Input_Write function can be used with the pin
    SOPC_PIN_DIRECTION_OUT, // Output_Read functions can be used with the pin. Output_Write should be used only inside
                            // HeatBeat callback.
    SOPC_PIN_DIRECTION_SIZE = INT32_MAX
} SOPC_Pin_Direction;

// Pin access mode enumeration, used by Output_Read function.
typedef enum SOPC_Pin_Access
{
    SOPC_PIN_MODE_GET_NORMAL = SOPC_MSGBOX_MODE_GET_NORMAL, // Mode normal. This is the FIFO event mode.
    SOPC_PIN_MODE_GET_NEW_LAST,                             // Get the most up to data pushed data and "pop it"
    SOPC_PIN_MODE_GET_LAST,                                 // Get the most up to date pushed data and don't pop it.
    SOPC_PIN_MODE_SIZE = SOPC_MSGBOX_MODE_SIZE
} SOPC_Pin_Access;

// RT Subscriber object
typedef struct SOPC_RT_Subscriber SOPC_RT_Subscriber;

// RT Subscriber initializer object
typedef struct SOPC_RT_Subscriber_Initializer SOPC_RT_Subscriber_Initializer;

// RT Subscriber Beat Heart callback. This callback is called via Step function,
// which scans each input and call this user function.
// Input number and input user context are passed through this callback. Those
// parameters can be used as information for decode process.
// User in this callback shall use Output_Write function to update its outputs.
typedef SOPC_ReturnStatus (*ptrBeatHeartCallback)(SOPC_RT_Subscriber* pSub, // RT Subscriber object
                                                  void* pGlobalContext,     // Global User context
                                                  void* pInputContext,      // Input user context
                                                  uint32_t input_number,    // Input pin number
                                                  uint8_t* pData,           // Data received
                                                  uint32_t size);           // Size of data

// RT Subscriber object creation
SOPC_RT_Subscriber* SOPC_RT_Subscriber_Create(void);

// RT Subscriber object destruction
void SOPC_RT_Subscriber_Destroy(SOPC_RT_Subscriber** in_out_sub);

// RT Subscriber object initialization
// Returns : SOPC_STATUS_OK if initialized.
// SOPC_INVALID_STATE in the case of unexpected current status (initializing, in use...)
SOPC_ReturnStatus SOPC_RT_Subscriber_Initialize(SOPC_RT_Subscriber* in_out_sub,           // RT Subscriber object
                                                SOPC_RT_Subscriber_Initializer* in_init); // Initializer object

// RT Subscriber de-initialization
// Returns : SOPC_STATUS_OK if uninitialized.
// SOPC_INVALID_STATE is returned if RT Subscriber is currently in use, initializing or de initializing.
// SOPC_STATUS_NOK if already uninitialized.
SOPC_ReturnStatus SOPC_RT_Subscriber_DeInitialize(SOPC_RT_Subscriber* in_out_sub);

// RT Subscriber Initializer Creation
SOPC_RT_Subscriber_Initializer* SOPC_RT_Subscriber_Initializer_Create(ptrBeatHeartCallback cbStep, //
                                                                      void* pGlobalContext);       //

// RT Subscriber Initializer Destruction
void SOPC_RT_Subscriber_Initializer_Destroy(SOPC_RT_Subscriber_Initializer** in_out_p_init);

// Add input definition to initializer object
SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddInput(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_evts,                        // Max events supported by this input before overflow
    uint32_t in_max_data,                        // Max data supported by this input before overflow
    SOPC_Pin_Access in_scanmode,                 // Get last, get last new, get normal
    void* in_input_context,                      // User context
    uint32_t* out_pin);                          // Pin number associated

// Add output definition to initialize object
SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddOutput(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_clients,                     // Max concurrent clients supported by this output
    uint32_t in_max_evts,                        // Max events supported by this output before overflow
    uint32_t in_max_data,                        // Max data supported by this output before overflow
    uint32_t* out_pin);                          // Pin number associated to this output

// Read output sequence initialization. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Initialize(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                            uint32_t in_pin,            // Pin number to read
                                                            uint32_t* out_token); // Token to use with read function

// Read output sequence. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read(SOPC_RT_Subscriber* in_sub,  // RT Subscriber object
                                                 uint32_t in_pin,             // Pin number
                                                 uint32_t in_clt,             // Client number
                                                 uint32_t in_token,           // Token returned by Read Initialize
                                                 SOPC_Pin_Access in_scanmode, // Scanning mode
                                                 uint8_t** out_pData,         // Data pointer
                                                 uint32_t* out_size);         // Data size

// Read output sequence finalization. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Finalize(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                          uint32_t in_pin,            // Pin number
                                                          uint32_t* out_token);       // Token

// Write input. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Input_Write(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                 uint32_t in_pin,            // Input pin number
                                                 uint8_t* in_data,           // Data to write
                                                 uint32_t in_size);          // Data size

// Write output. Used inside RT Subscriber, in the user callback to update an output.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Write(SOPC_RT_Subscriber* in_sub, // RT Subscriber
                                                  uint32_t in_pin,            // Pin number
                                                  uint8_t* in_data,           // Data to write
                                                  uint32_t in_size);          // Size to write

// Beat heart. Use to read each input.
// For each read input, user callback is invoked.
SOPC_ReturnStatus SOPC_RT_Subscriber_BeatHeart(SOPC_RT_Subscriber* in_sub);

#endif /* SOPC_RT_SUBSCRIBER_H_ */
