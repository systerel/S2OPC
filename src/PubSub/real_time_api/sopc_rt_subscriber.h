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

/// @file sopc_rt_subscriber.h
/// @brief Real time subscriber, updated by step function called from IRQ, read input messages from ::SOPC_MsgBox and
/// call message management callback (decode, update data target, etc.)
///
/// RT Subscriber is used to interface one or several input variables to reception performed under interrupt by example,
/// and interface one or several output variables to the user application.
///
/// RT Subscriber is initialized with a user callback, invoked periodically by heart beat function called under
/// interrupt by example.
///
/// RT Subscriber handle one or several output variables, which can be written in the frame of the user callback.
///
/// RT Subscriber inputs are written out of the frame of the callback, read in the frame of the user callback.
///
/// RT Subscriber outputs are written in the frame of the user callback, read out of the frame of the user callback.
///
/// RT Subscriber input shall be written by 1 writer max.
///
/// RT Subscriber output can be read by several clients at same time.
///
/// Following steps describes how to create a RT Subscriber:
/// * 1) Create an initializer with SOPC_RT_Subscriber_Initializer_Create which specify user heart beat callback
/// * 2) Add your inputs, with their parameters: max events, max data, scanning mode (how data are pop and passed to
/// user callback). Retrieve input number information.
/// * 3) Add your outputs, with their parameters: max events, data, number of max simultaneous clients
/// * 4) Create a SOPC_RT_Subscriber object
/// * 5) Initialize SOPC_RT_Subscriber object with SOPC_RT_Subscriber_Initializer object. You can destroy it after
/// initialization.
///
/// Following steps describes how to use a RT Subscriber
/// * 1) After creation, heart beat SOPC_RT_Subscriber_HeartBeat shall be invoked periodically.
/// * 2) Use SOPC_RT_Subscriber_Input_Write from the user application with the identifier of the input to write an input
/// * 3) In the frame of the user callback, this one is called with input number, data and data size as parameters.
/// User can apply its own treatment on this data, and write an output with SOPC_RT_Subscriber_Output_Write with the
/// identifier of the output.
/// * 4) Outside the callback, from the user application, use SOPC_RT_Subscriber_Output_Read_XXX to read the output.
///
/// Following steps describers how to read an output:
/// * 1) Call SOPC_RT_Subscriber_Output_Read_Initialize with the output number. A token is returned.
/// * 2) Call SOPC_RT_Subscriber_Output_Read with the previously returned token, the output number, and the client
/// identifier.
/// * 3) Copy the data from the pointer and size previously returned.
/// * 4) Call SOPC_RT_Subscriber_Output_Read_Finalize to indicate data for this client are not further used, copied...
///
/// Todo: For writing without copy to an output or an input, there are no implementation, but under layer mechanism
/// (message box) implementation are ready to implement writing without copy via 3 steps write operation via a data
/// handle (see SOPC_MsgBox_DataHandle_XXX functions)

#ifndef SOPC_RT_SUBSCRIBER_H_
#define SOPC_RT_SUBSCRIBER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_buffer.h"
#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

#include "sopc_msgbox.h"

/// @brief Pin direction enumeration.
typedef enum SOPC_Pin_Direction
{
    SOPC_PIN_DIRECTION_IN,  ///< Input_Write function can be used with the pin
    SOPC_PIN_DIRECTION_OUT, ///< Output_Read functions can be used with the pin. Output_Write should be used only inside
                            /// HeatBeat callback.
    SOPC_PIN_DIRECTION_SIZE = INT32_MAX
} SOPC_Pin_Direction;

/// @brief Pin access mode enumeration, used by SOPC_RT_Subscriber_Output_Read function.
typedef enum SOPC_Pin_Access
{
    SOPC_PIN_MODE_GET_NORMAL = SOPC_MSGBOX_MODE_GET_NORMAL, ///< Mode normal. This is the FIFO event mode.
    SOPC_PIN_MODE_GET_NEW_LAST,                             ///< Get the most up to data pushed data and "pop it"
    SOPC_PIN_MODE_GET_LAST,                                 ///< Get the most up to date pushed data and don't pop it.
    SOPC_PIN_MODE_SIZE = SOPC_MSGBOX_MODE_SIZE
} SOPC_Pin_Access;

/// @brief RT Subscriber object
typedef struct SOPC_RT_Subscriber SOPC_RT_Subscriber;

/// @brief RT Subscriber initializer object.
/// @warning Initializer object API is not thread safe.
typedef struct SOPC_RT_Subscriber_Initializer SOPC_RT_Subscriber_Initializer;

/// @brief RT Subscriber Beat Heart callback. This callback is called via SOPC_RT_Subscriber_HeartBeat function,
/// which scans each input, and for each input call this user function.
///
/// Input number and input user context, set by initializer, are passed through this callback. Those
/// parameters can be used as information to implement by example a decoder process.
///
/// @warning Do not write to pData parameter!
///
/// User in this callback shall use SOPC_RT_Subscriber_Output_Write function to update its outputs.
/// @param [in] pSub RT Subscriber object
/// @param [in] pGlobalContext Global user context, set by SOPC_RT_Subscriber_Initializer_Create
/// @param [in] pInputContext Input user context, set by SOPC_RT_Subscriber_Initializer_AddInput
/// @param [in] input_number Input identifier, returned by SOPC_RT_Subscriber_Initializer_AddInput
/// @param [in] pData Data received
/// @param [in] size Size of data received
typedef SOPC_ReturnStatus (*ptrBeatHeartCallback)(SOPC_RT_Subscriber* pSub,
                                                  void* pGlobalContext,
                                                  void* pInputContext,
                                                  uint32_t input_number,
                                                  uint8_t* pData,
                                                  uint32_t size);

/// @brief RT Subscriber object creation
/// @return SOPC_RT_Subscriber object
SOPC_RT_Subscriber* SOPC_RT_Subscriber_Create(void);

/// @brief RT Subscriber object destruction
/// @param [inout] in_out_sub RT Subscriber object to destroy. Set to NULL on return.
void SOPC_RT_Subscriber_Destroy(SOPC_RT_Subscriber** in_out_sub);

/// @brief RT Subscriber object initialization
/// @param [inout] in_out_sub RT Subscriber object to initialize
/// @param [in] in_init RT Subscriber Initializer object initialized with a minimum of minimum 1 input / 1 output
/// @return SOPC_STATUS_OK if initialized.
/// SOPC_INVALID_STATE in the case of unexpected current status (initializing, in use...)
SOPC_ReturnStatus SOPC_RT_Subscriber_Initialize(SOPC_RT_Subscriber* in_out_sub,
                                                SOPC_RT_Subscriber_Initializer* in_init);

/// @brief RT Subscriber de-initialization
/// @param [inout] in_out_sub RT Subscriber object to de initialize.
/// @return SOPC_STATUS_OK if uninitialized.
/// SOPC_INVALID_STATE is returned if RT Subscriber is currently in use, initializing or de initializing.
/// SOPC_STATUS_NOK if already uninitialized.
SOPC_ReturnStatus SOPC_RT_Subscriber_DeInitialize(SOPC_RT_Subscriber* in_out_sub);

/// @brief RT Subscriber Initializer Creation
/// @param [in] cbStep User callback will be called for each input by SOPC_RT_Subscriber_HeartBeat
/// @param [in] pGlobalContext A global context which will be passed to callback for all input
/// @return SOPC_RT_Subscriber_Initializer object
SOPC_RT_Subscriber_Initializer* SOPC_RT_Subscriber_Initializer_Create(ptrBeatHeartCallback cbStep,
                                                                      void* pGlobalContext);

/// @brief RT Subscriber Initializer Destruction
/// @param [in] in_out_p_init RT Subscriber Initializer object to destroy
void SOPC_RT_Subscriber_Initializer_Destroy(SOPC_RT_Subscriber_Initializer** in_out_p_init);

/// @brief Add input definition to initializer object
/// @param [inout] in_out_init Initializer object
/// @param [in] in_max_evts Max events supported by this input before overflow
/// @param [in] in_max_data Max data supported by this input before overflow
/// @param [in] in_scanmode Get last, get last new, get normal
/// @param [in] in_input_context User context
/// @param [out] out_pin Pin number associated to this input.
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddInput(SOPC_RT_Subscriber_Initializer* in_out_init,
                                                          uint32_t in_max_evts,
                                                          uint32_t in_max_data,
                                                          SOPC_Pin_Access in_scanmode,
                                                          void* in_input_context,
                                                          uint32_t* out_pin);

/// @brief Add output definition to initialize object
/// @param [in] in_out_init Initializer object
/// @param [in] in_max_clients Max concurrent clients supported by this output
/// @param [in] in_max_evts Max events supported by this output before overflow
/// @param [in] in_max_data Max data supported by this output before overflow
/// @param [in] out_pin Pin number associated to this output
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddOutput(SOPC_RT_Subscriber_Initializer* in_out_init,
                                                           uint32_t in_max_clients,
                                                           uint32_t in_max_evts,
                                                           uint32_t in_max_data,
                                                           uint32_t* out_pin);

/// @brief Read output sequence initialization. Used outside RT Subscriber.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number to read (identifier returned by SOPC_RT_Subscriber_Initializer_AddOutput)
/// @param [out] out_token Token which identify read output sequence. Will be used by SOPC_RT_Subscriber_Output_Read and
/// SOPC_RT_Subscriber_Output_Read_Finalize
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Initialize(SOPC_RT_Subscriber* in_sub,
                                                            uint32_t in_pin,
                                                            size_t* out_token);

/// @brief Read output sequence. Used outside RT Subscriber.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddOutput
/// @param [in] in_clt Client number, shall be between 0 and max client - 1
/// @param [in] in_token Token which identify read output sequence. Returned by
/// SOPC_RT_Subscriber_Output_Read_Initialize
/// @param [in] in_scanmode Scanning mode. Get normal, get new last, get last...
/// @param [out] out_pData Data pointer. @warning Don't write to this location.
/// @param [out] out_size Data size (significant bytes) pointed by *out_pData.
/// @return SOPC_STATUS_OK if successful.
/// SOPC_STATUS_INVALID_STATE if read sequence is performing for the same client identifier in other context.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read(SOPC_RT_Subscriber* in_sub,
                                                 uint32_t in_pin,
                                                 uint32_t in_clt,
                                                 size_t in_token,
                                                 SOPC_Pin_Access in_scanmode,
                                                 uint8_t** out_pData,
                                                 uint32_t* out_size);

/// @brief Read output sequence finalization. Used outside RT Subscriber.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number, returned by SOPC_RT_Subscriber_Initializer_AddOutput
/// @param [inout] in_out_token Token which identify read output sequence. Returned by
/// SOPC_RT_Subscriber_Output_Read_Initialize. Set to UINT32_MAX on return (invalid sequence)
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Finalize(SOPC_RT_Subscriber* in_sub,
                                                          uint32_t in_pin,
                                                          size_t* in_out_token);

/// @brief Write input. Used outside RT Subscriber.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddInput
/// @param [in] in_data Data to write (buffer will be copied to internal buffer)
/// @param [in] in_size Size to write
/// @return SOPC_STATUS_OK if successful.
/// SOPC_STATUS_INVALID_STATE if used by another process.
SOPC_ReturnStatus SOPC_RT_Subscriber_Input_Write(SOPC_RT_Subscriber* in_sub,
                                                 uint32_t in_pin,
                                                 uint8_t* in_data,
                                                 uint32_t in_size);

/// @brief Write output. Used inside RT Subscriber, in the user callback to update an output.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number
/// @param [in] in_data Data to write
/// @param [in] in_size Size to write
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Write(SOPC_RT_Subscriber* in_sub,
                                                  uint32_t in_pin,
                                                  uint8_t* in_data,
                                                  uint32_t in_size);

/// @brief Get direct data pointer for output selected pin
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_sopc_buffer SOPC Buffer structure,
/// with data pointer set to internal buffer on return.
/// On entry, data pointer of SOPC_Buffer shall be set to NULL.
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_WriteDataHandle_GetBuffer(SOPC_RT_Subscriber* in_sub,
                                                                      uint32_t in_pin,
                                                                      SOPC_Buffer* in_out_sopc_buffer);

/// @brief Get direct data pointer for input selected pin
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_sopc_buffer SOPC Buffer structure,
/// with data pointer set to internal buffer on return.
/// On entry, data pointer of SOPC_Buffer shall be set to NULL.
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Input_WriteDataHandle_GetBuffer(SOPC_RT_Subscriber* in_sub,
                                                                     uint32_t in_pin,
                                                                     SOPC_Buffer* in_out_sopc_buffer);

/// @brief Release direct data pointer for output selected pin, without API concurrent accesses protection.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_sopc_buffer Size field is use to update significant bytes.
/// SOPC Buffer structure, with data pointer set to NULL on return.
/// On entry, SOPC_Buffer data pointer is normally not NULL if GetBuffer was successful.
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_WriteDataHandle_ReleaseBuffer(SOPC_RT_Subscriber* in_sub,
                                                                          uint32_t in_pin,
                                                                          SOPC_Buffer* in_out_sopc_buffer);

/// @brief Release direct data pointer for input selected pin, without API concurrent accesses protection.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_sopc_buffer Size field is use to update significant bytes.
/// SOPC Buffer structure, with data pointer set to NULL on return.
/// On entry, SOPC_Buffer data pointer is normally not NULL if GetBuffer was successful.
/// @return SOPC_STATUS_OK if successful.
SOPC_ReturnStatus SOPC_RT_Subscriber_Input_WriteDataHandle_ReleaseBuffer(SOPC_RT_Subscriber* in_sub,
                                                                         uint32_t in_pin,
                                                                         SOPC_Buffer* in_out_sopc_buffer);

/// @brief Heart beat. Use to read each input.
/// For each read input, user callback is invoked.
/// @param [in] in_sub RT Subscriber object
/// @return SOPC_STATUS_OK if successful. SOPC_STATUS_INVALID_STATE if not initialized.
/// SOPC_STATUS_NOK or other error can be returned in case of an error on an input read or input treatment.
SOPC_ReturnStatus SOPC_RT_Subscriber_HeartBeat(SOPC_RT_Subscriber* in_sub);

#endif /* SOPC_RT_SUBSCRIBER_H_ */
