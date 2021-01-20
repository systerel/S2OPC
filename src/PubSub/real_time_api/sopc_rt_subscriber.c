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

/// @file sopc_rt_subscriber.c

#include "sopc_rt_subscriber.h"

/// @brief Private status of RT Subscriber
typedef enum E_SUBSCRIBER_SYNC_STATUS
{
    E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED, ///< RT Subscriber not initialized
    E_SUBSCRIBER_SYNC_STATUS_INITIALIZING,    ///< RT Subscriber is initializing
    E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING,  ///< RT Subscriber is deinitializing
    E_SUBSCRIBER_SYNC_STATUS_INITIALIZED,     ///< RT Subscriber initialized
    E_SUBSCRIBER_SYNC_STATUS_LOCKED,          ///< RT Subscriber can't be deinitialized because in use.
    E_SUBSCRIBER_SYNC_STATUS_SIZE = INT32_MAX
} eSubscriberSyncStatus;

/// @brief Inputs table definition
typedef struct SOPC_RT_Subscriber_Inputs
{
    uint32_t nbInputs;                    ///< Number of inputs
    SOPC_MsgBox** pMsgBox;                ///< Table of inputs
    SOPC_MsgBox_DataHandle** pDataHandle; ///< Table of input write without copy handle
    SOPC_Pin_Access* pReadMode;           ///< Table of scan mode for each input
    void** ppContext;                     ///< Table of user context for each input
} SOPC_RT_Subscriber_Inputs;

/// @brief Output table definition
typedef struct SOPC_RT_Subscriber_Outputs
{
    uint32_t nbOutputs;                   ///< Number of outputs
    SOPC_MsgBox** pMsgBox;                ///< Table of outputs
    SOPC_MsgBox_DataHandle** pDataHandle; ///< Table of output write without copy handle
} SOPC_RT_Subscriber_Outputs;

/// @brief RT Subscriber definition
struct SOPC_RT_Subscriber
{
    eSubscriberSyncStatus status;       ///< RT subscriber status
    SOPC_RT_Subscriber_Inputs inputs;   ///< Inputs
    SOPC_RT_Subscriber_Outputs outputs; ///< Outputs
    ptrBeatHeartCallback cbStep;        ///< Step callback
    void* pUserContext;                 ///< User context
};

/// @brief Subscriber configuration list element
typedef struct SOPC_RT_Subscriber_Initializer_PinConfig
{
    uint32_t max_evts;                                     ///< Max events supported by pin before overflow
    uint32_t max_data;                                     ///< Max data supported by pin before overflow
    uint32_t nbClients;                                    ///< Max concurrent clients which can read same pin.
    SOPC_Pin_Access scanMode;                              ///< Used only for inputs.
    void* inputContext;                                    ///< Used only for inputs.
    struct SOPC_RT_Subscriber_Initializer_PinConfig* next; ///< Pointer on next pin configuration.
} SOPC_RT_Subscriber_Initializer_PinConfig;

/// @brief RT Subscriber initializer object
struct SOPC_RT_Subscriber_Initializer
{
    uint32_t nbInputs;                                     ///< Number of inputs in the inputs list
    SOPC_RT_Subscriber_Initializer_PinConfig* pInputsList; ///< Inputs list

    uint32_t nbOutputs;                                     ///< Number of outputs lists
    SOPC_RT_Subscriber_Initializer_PinConfig* pOutputsList; ///< Outputs list

    ptrBeatHeartCallback cbStep; ///< Heart beat callback
    void* pUserContext;          ///< Global User context
};

// Declarations of private functions

/// @brief Try to increment RT Subscriber status. Allow to define if API is in use.
/// @param [in] in_sub RT Subscriber object.
/// @return Incremented status by 1 if initial status was equal or greater than initialized
static inline eSubscriberSyncStatus _SOPC_RT_Subscriber_IncrementInUseStatus(SOPC_RT_Subscriber* in_sub);

/// @brief Try to decrement RT Subscriber status. Allow to define if API is in use.
/// @param [in] in_sub RT Subscriber object.
/// @return Decremented status by 1 if initial status was greater than initialized
static inline eSubscriberSyncStatus _SOPC_RT_Subscriber_DecrementInUseStatus(SOPC_RT_Subscriber* in_sub);

/// @brief Initialize RT Subscriber object without API concurrent accesses protection.
/// @param [inout] in_out_sub RT Subscriber object
/// @param [in] in_init RT Subscriber initializer object
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Initialize(SOPC_RT_Subscriber* in_out_sub,
                                                               SOPC_RT_Subscriber_Initializer* in_init);

/// @brief DeInitialize RT Subscriber object without API concurrent accesses protection.
/// @param [inout] in_out_sub RT Subscriber object to deinitialize
/// @return SOPC_STATUS_OK if successful.
static inline void _SOPC_RT_Subscriber_DeInitialize(SOPC_RT_Subscriber* in_out_sub);

/// @brief Add pin to an initializer. Called by SOPC_RT_Subscriber_Initializer_AddInput
/// and SOPC_RT_Subscriber_Initializer_AddInput
/// @param [inout] in_out_init Initializer object
/// @param [in] in_max_clients Max number of simultaneous clients accesses
/// @param [in] in_max_evts Max events supported by this pin before overflow
/// @param [in] in_max_data Max data supported by this pin
/// @param [in] in_mode Direction, output or input
/// @param [in] in_scanmode Scan mode, only taken into account for input pin
/// @param [in] in_input_context User context linked to this pin
/// @param [out] out_pinNumber Pin number allocated for this pin.
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Initializer_AddPin(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_clients,                     // Max client
    uint32_t in_max_evts,                        // Max events supported by this output before overflow
    uint32_t in_max_data,                        // Max data supported by this output
    SOPC_Pin_Direction in_mode,                  // Direction
    SOPC_Pin_Access in_scanmode,                 // Scan mode
    void* in_input_context,                      // User context linked to this pin
    uint32_t* out_pinNumber);                    // pin number.

/// @brief Read pin sequence initialization, used by SOPC_RT_Subscriber_Input_Read_Initialize
/// and SOPC_RT_Subscriber_Output_Read_Initialize.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_dir Pin direction, input or output
/// @param [in] in_pin Pin number
/// @param [out] out_token Token of read sequence
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Read_Initialize(
    SOPC_RT_Subscriber* in_sub, // RT subscriber object
    SOPC_Pin_Direction in_dir,  // Pin direction
    uint32_t in_pin,            // pin number
    size_t* out_token);         // token of read sequence

/// @brief Read pin sequence, without API concurrent accesses protection
/// @param [in] in_itf Pin direction
/// @param [in] in_pSub RT Subscriber object
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [in] in_clt Client number, shall be between 0 and max client - 1
/// @param [in] in_token Token which identify read output sequence. Returned by
/// SOPC_RT_Subscriber_Pin_Read_Initialize
/// @param [in] in_mode Access mode. Get normal, get new last, get last...
/// @param [out] out_pData Data pointer. @warning Don't write to this location.
/// @param [out] out_size Data size (significant bytes) pointed by *out_pData.
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Read(SOPC_RT_Subscriber* in_pSub, // RT Subscriber object
                                                             SOPC_Pin_Direction in_itf,   // pin direction
                                                             uint32_t in_pin,             // Pin number
                                                             uint32_t in_clt,             // Client number
                                                             size_t in_token,             // Token of read sequence
                                                             SOPC_Pin_Access in_mode,     // Access mode
                                                             uint8_t** out_pData,         // Pointer on data
                                                             uint32_t* out_size);         // Size of data

/// @brief Read pin sequence finalization.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_itf Pin direction
/// @param [in] in_pin Pin number, returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_token Token which identify read output sequence. Returned by
/// SOPC_RT_Subscriber_Output_Read_Initialize. Set to UINT32_MAX on return (invalid sequence)
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Read_Finalize(
    SOPC_RT_Subscriber* in_sub, // RT Subscriber object
    SOPC_Pin_Direction in_itf,  // Pin direction
    uint32_t in_pin,            // Pin number
    size_t* in_out_token);      // token

/// @brief Get direct data pointer for selected pin, without API concurrent accesses protection.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_itf Pin direction
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_sopc_buffer SOPC Buffer structure,
/// with data pointer set to internal buffer on return.
/// On entry, data pointer of SOPC_Buffer shall be set to NULL.
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_WriteDataHandle_GetBuffer(
    SOPC_RT_Subscriber* in_sub,       // RT Subscriber object
    SOPC_Pin_Direction in_itf,        // Pin direction
    uint32_t in_pin,                  // Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
    SOPC_Buffer* in_out_sopc_buffer); // SOPC Buffer structure, with data pointer set to internal buffer on return

/// @brief Release direct data pointer for selected pin, without API concurrent accesses protection.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_itf Pin direction
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
/// @param [inout] in_out_sopc_buffer Size field is use to update significant bytes.
/// SOPC Buffer structure, with data pointer set to NULL on return.
/// On entry, SOPC_Buffer data pointer is normally not NULL if GetBuffer was successful.
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_WriteDataHandle_ReleaseBuffer(
    SOPC_RT_Subscriber* in_sub,       // RT Subscriber object
    SOPC_Pin_Direction in_itf,        // Pin direction
    uint32_t in_pin,                  // Pin number returned by SOPC_RT_Subscriber_Initializer_AddPin
    SOPC_Buffer* in_out_sopc_buffer); // SOPC Buffer structure, with data pointer set to NULL on return

/// @brief Write pin, without API concurrent accesses protection.
/// @param [in] in_sub RT Subscriber object
/// @param [in] in_itf Pin direction
/// @param [in] in_pin Pin number returned by SOPC_RT_Subscriber_Initializer_AddInput
/// @param [in] in_data Data to write (buffer will be copied to internal buffer)
/// @param [in] in_size Size to write
/// @return SOPC_STATUS_OK if successful.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Write(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                              SOPC_Pin_Direction in_itf,  // Pin direction
                                                              uint32_t in_pin,            // Pin number
                                                              uint8_t* in_data,           // Data to write
                                                              uint32_t in_size);          // Size to write

/// @brief Heart beat. Use to read each input.
/// For each read input, user callback is invoked.
/// No concurrent accesses protection.
/// @param [in] in_sub RT Subscriber object
/// @return SOPC_STATUS_OK if successful. SOPC_STATUS_INVALID_STATE if not initialized.
/// SOPC_STATUS_NOK or other error can be returned in case of an error on an input read or input treatment.
static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_HeartBeat(SOPC_RT_Subscriber* in_sub);

SOPC_RT_Subscriber* SOPC_RT_Subscriber_Create(void)
{
    SOPC_RT_Subscriber* pSub = SOPC_Calloc(1, sizeof(SOPC_RT_Subscriber));
    return pSub;
}

void SOPC_RT_Subscriber_Destroy(SOPC_RT_Subscriber** in_out_sub)
{
    if (NULL != in_out_sub)
    {
        SOPC_RT_Subscriber* pSub = *in_out_sub;
        if (NULL != pSub)
        {
            SOPC_Free(pSub);
            *in_out_sub = NULL;
        }
    }
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Initialize(SOPC_RT_Subscriber* in_out_sub, SOPC_RT_Subscriber_Initializer* in_init)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_out_sub || NULL == in_init)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus expectedStatus = E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED;
    eSubscriberSyncStatus desiredStatus = E_SUBSCRIBER_SYNC_STATUS_INITIALIZING;
    bool bTransition = __atomic_compare_exchange(&in_out_sub->status, &expectedStatus, &desiredStatus, false,
                                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

    if (bTransition)
    {
        result = _SOPC_RT_Subscriber_Initialize(in_out_sub, in_init);
        if (SOPC_STATUS_OK == result)
        {
            desiredStatus = E_SUBSCRIBER_SYNC_STATUS_INITIALIZED;
            __atomic_store(&in_out_sub->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            _SOPC_RT_Subscriber_DeInitialize(in_out_sub);
            desiredStatus = E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED;
            __atomic_store(&in_out_sub->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
    }
    else if (E_SUBSCRIBER_SYNC_STATUS_INITIALIZING == expectedStatus ||
             E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING == expectedStatus ||
             E_SUBSCRIBER_SYNC_STATUS_LOCKED <= expectedStatus)
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_DeInitialize(SOPC_RT_Subscriber* in_out_sub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_out_sub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus expectedStatus = E_SUBSCRIBER_SYNC_STATUS_INITIALIZED;
    eSubscriberSyncStatus desiredStatus = E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING;
    bool bTransition = __atomic_compare_exchange(&in_out_sub->status, &expectedStatus, &desiredStatus, false,
                                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

    if (bTransition)
    {
        _SOPC_RT_Subscriber_DeInitialize(in_out_sub);

        desiredStatus = E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED;
        __atomic_store(&in_out_sub->status, &desiredStatus, __ATOMIC_SEQ_CST);
    }
    else if (E_SUBSCRIBER_SYNC_STATUS_INITIALIZING == expectedStatus ||
             E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING == expectedStatus ||
             E_SUBSCRIBER_SYNC_STATUS_LOCKED <= expectedStatus)
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

SOPC_RT_Subscriber_Initializer* SOPC_RT_Subscriber_Initializer_Create(ptrBeatHeartCallback cbStep, void* pGlobalContext)
{
    if (NULL == cbStep)
    {
        return NULL;
    }
    SOPC_RT_Subscriber_Initializer* pInit = SOPC_Calloc(1, sizeof(SOPC_RT_Subscriber_Initializer));
    if (NULL != pInit)
    {
        pInit->cbStep = cbStep;
        pInit->pUserContext = pGlobalContext;
    }
    return pInit;
}

void SOPC_RT_Subscriber_Initializer_Destroy(SOPC_RT_Subscriber_Initializer** in_out_p_init)
{
    if (NULL != in_out_p_init)
    {
        SOPC_RT_Subscriber_Initializer* pInit = *in_out_p_init;
        if (NULL != pInit)
        {
            SOPC_RT_Subscriber_Initializer_PinConfig* pPinConf = pInit->pInputsList;
            SOPC_RT_Subscriber_Initializer_PinConfig* pTemp = NULL;
            while (NULL != pPinConf)
            {
                pTemp = pPinConf;
                pPinConf = pPinConf->next;
                SOPC_Free(pTemp);
            }
            pPinConf = pInit->pOutputsList;
            while (NULL != pPinConf)
            {
                pTemp = pPinConf;
                pPinConf = pPinConf->next;
                SOPC_Free(pTemp);
            }
            SOPC_Free(pInit);
            *in_out_p_init = NULL;
        }
    }
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddInput(SOPC_RT_Subscriber_Initializer* in_out_init,
                                                          uint32_t in_max_evts,
                                                          uint32_t in_max_data,
                                                          SOPC_Pin_Access in_scanmode,
                                                          void* in_input_context,
                                                          uint32_t* out_pin)

{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    result = _SOPC_RT_Subscriber_Initializer_AddPin(in_out_init, 1, in_max_evts, in_max_data, SOPC_PIN_DIRECTION_IN,
                                                    in_scanmode, in_input_context, out_pin);

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddOutput(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_clients,                     // Max concurrent clients supported by this output
    uint32_t in_max_evts,                        // Max events supported by this output before overflow
    uint32_t in_max_data,                        // Max data supported by this output before overflow
    uint32_t* out_pin)                           // Pin number associated to this output

{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    result = _SOPC_RT_Subscriber_Initializer_AddPin(in_out_init, in_max_clients, in_max_evts, in_max_data,
                                                    SOPC_PIN_DIRECTION_OUT, SOPC_PIN_MODE_GET_NORMAL, NULL, out_pin);

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Initialize(SOPC_RT_Subscriber* in_sub,
                                                            uint32_t in_pin,
                                                            size_t* out_token)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == out_token)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_Read_Initialize(in_sub, SOPC_PIN_DIRECTION_OUT, in_pin, out_token);

        // Do not restore previous state. Mark as "in use" until that finalize call.
        if (SOPC_STATUS_OK == result)
        {
            // A successful initialization indicates that message box pointer can be used out of protection
            // phase whereas a de initialization phase try to start.
            // Increment this counter. Decremented after successful finalization.
            _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);
        }

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read(SOPC_RT_Subscriber* in_sub,
                                                 uint32_t in_pin,
                                                 uint32_t in_clt,
                                                 size_t in_token,
                                                 SOPC_Pin_Access in_scanmode,
                                                 uint8_t** out_pData,
                                                 uint32_t* out_size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == out_pData || NULL == out_size)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_Read(in_sub, SOPC_PIN_DIRECTION_OUT, in_pin, in_clt, in_token, in_scanmode,
                                              out_pData, out_size);

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Finalize(SOPC_RT_Subscriber* in_sub,
                                                          uint32_t in_pin,
                                                          size_t* in_out_token)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == in_out_token)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_Read_Finalize(in_sub, SOPC_PIN_DIRECTION_OUT, in_pin, in_out_token);

        if (SOPC_STATUS_OK == result)
        {
            // A successful finalization indicates that message box pointer will not be further
            // used out of protection
            // De initialization phase  is allowed.
            _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
        }

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Input_Write(SOPC_RT_Subscriber* in_sub,
                                                 uint32_t in_pin,
                                                 uint8_t* in_data,
                                                 uint32_t in_size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || in_data == NULL || in_size < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_Write(in_sub, SOPC_PIN_DIRECTION_IN, in_pin, in_data, in_size);

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Write(SOPC_RT_Subscriber* in_sub,
                                                  uint32_t in_pin,
                                                  uint8_t* in_data,
                                                  uint32_t in_size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || in_data == NULL || in_size < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_Write(in_sub, SOPC_PIN_DIRECTION_OUT, in_pin, in_data, in_size);

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Output_WriteDataHandle_GetBuffer(SOPC_RT_Subscriber* in_sub,
                                                                      uint32_t in_pin,
                                                                      SOPC_Buffer* in_out_sopc_buffer)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == in_out_sopc_buffer || NULL != in_out_sopc_buffer->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_WriteDataHandle_GetBuffer(in_sub, SOPC_PIN_DIRECTION_OUT, in_pin,
                                                                   in_out_sopc_buffer);

        // Do not restore previous state. Mark as "in use" until that finalize call.
        if (SOPC_STATUS_OK == result)
        {
            // A successful initialization indicates that message box pointer can be used out of protection
            // phase whereas a de initialization phase try to start.
            // Increment this counter. Decremented after successful finalization.
            _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);
        }

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Input_WriteDataHandle_GetBuffer(SOPC_RT_Subscriber* in_sub,
                                                                     uint32_t in_pin,
                                                                     SOPC_Buffer* in_out_sopc_buffer)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == in_out_sopc_buffer || NULL != in_out_sopc_buffer->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_WriteDataHandle_GetBuffer(in_sub, SOPC_PIN_DIRECTION_IN, in_pin,
                                                                   in_out_sopc_buffer);

        // Do not restore previous state. Mark as "in use" until that finalize call.
        if (SOPC_STATUS_OK == result)
        {
            // A successful initialization indicates that message box pointer can be used out of protection
            // phase whereas a de initialization phase try to start.
            // Increment this counter. Decremented after successful finalization.
            _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);
        }

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Output_WriteDataHandle_ReleaseBuffer(SOPC_RT_Subscriber* in_sub,
                                                                          uint32_t in_pin,
                                                                          SOPC_Buffer* in_out_sopc_buffer)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == in_out_sopc_buffer || NULL == in_out_sopc_buffer->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_WriteDataHandle_ReleaseBuffer(in_sub, SOPC_PIN_DIRECTION_OUT, in_pin,
                                                                       in_out_sopc_buffer);

        if (SOPC_STATUS_OK == result)
        {
            // A successful finalization indicates that message box pointer will not be further
            // used out of protection
            // De initialization phase  is allowed.
            _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
        }

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_Input_WriteDataHandle_ReleaseBuffer(SOPC_RT_Subscriber* in_sub,
                                                                         uint32_t in_pin,
                                                                         SOPC_Buffer* in_out_sopc_buffer)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == in_out_sopc_buffer || NULL == in_out_sopc_buffer->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_Pin_WriteDataHandle_ReleaseBuffer(in_sub, SOPC_PIN_DIRECTION_IN, in_pin,
                                                                       in_out_sopc_buffer);

        if (SOPC_STATUS_OK == result)
        {
            // A successful finalization indicates that message box pointer will not be further
            // used out of protection
            // De initialization phase  is allowed.
            _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
        }

        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

SOPC_ReturnStatus SOPC_RT_Subscriber_HeartBeat(SOPC_RT_Subscriber* in_sub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = _SOPC_RT_Subscriber_IncrementInUseStatus(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = _SOPC_RT_Subscriber_HeartBeat(in_sub);
        _SOPC_RT_Subscriber_DecrementInUseStatus(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_HeartBeat(SOPC_RT_Subscriber* in_sub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    SOPC_ReturnStatus globalResult = SOPC_STATUS_OK;

    if (NULL == in_sub || NULL == in_sub->cbStep)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t token = 0;
    uint32_t size = 0;
    uint8_t* pData = NULL;

    for (uint32_t pin = 0; pin < in_sub->inputs.nbInputs; pin++)
    {
        token = 0;
        size = 0;
        pData = NULL;

        result = _SOPC_RT_Subscriber_Pin_Read_Initialize(in_sub, SOPC_PIN_DIRECTION_IN, pin, &token);

        if (SOPC_STATUS_OK == result)
        {
            do
            {
                result = _SOPC_RT_Subscriber_Pin_Read(in_sub, SOPC_PIN_DIRECTION_IN, pin, 0, token,
                                                      in_sub->inputs.pReadMode[pin], &pData, &size);

                if (pData != NULL && size > 0 && SOPC_STATUS_OK == result)
                {
                    result =
                        in_sub->cbStep(in_sub, in_sub->pUserContext, in_sub->inputs.ppContext[pin], pin, pData, size);
                }

            } while ((pData != NULL) && (size > 0) && (SOPC_STATUS_OK == result) &&
                     (in_sub->inputs.pReadMode[pin] != SOPC_PIN_MODE_GET_LAST));

            _SOPC_RT_Subscriber_Pin_Read_Finalize(in_sub, SOPC_PIN_DIRECTION_IN, pin, &token);
        }

        if (SOPC_STATUS_OK != result)
        {
            globalResult = result;
        }
    }

    return globalResult;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Initialize(SOPC_RT_Subscriber* in_out_sub,
                                                               SOPC_RT_Subscriber_Initializer* in_init)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (in_init->nbInputs < 1 || in_init->nbOutputs < 1 || in_init->cbStep == NULL)
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == result)
    {
        in_out_sub->cbStep = in_init->cbStep;

        in_out_sub->inputs.pMsgBox = (SOPC_MsgBox**) SOPC_Calloc(1, sizeof(SOPC_MsgBox*) * in_init->nbInputs);
        if (NULL == in_out_sub->inputs.pMsgBox)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        in_out_sub->outputs.pMsgBox = (SOPC_MsgBox**) SOPC_Calloc(1, sizeof(SOPC_MsgBox*) * in_init->nbOutputs);
        if (NULL == in_out_sub->outputs.pMsgBox)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        in_out_sub->inputs.pDataHandle =
            (SOPC_MsgBox_DataHandle**) SOPC_Calloc(1, sizeof(SOPC_MsgBox_DataHandle*) * in_init->nbInputs);
        if (NULL == in_out_sub->inputs.pDataHandle)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        in_out_sub->outputs.pDataHandle =
            (SOPC_MsgBox_DataHandle**) SOPC_Calloc(1, sizeof(SOPC_MsgBox_DataHandle*) * in_init->nbOutputs);
        if (NULL == in_out_sub->outputs.pDataHandle)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        in_out_sub->inputs.pReadMode = (SOPC_Pin_Access*) SOPC_Calloc(1, sizeof(SOPC_Pin_Access) * in_init->nbInputs);
        if (NULL == in_out_sub->inputs.pReadMode)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        in_out_sub->inputs.ppContext = (void**) SOPC_Calloc(1, sizeof(void*) * in_init->nbInputs);
        if (NULL == in_out_sub->inputs.ppContext)
        {
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        SOPC_RT_Subscriber_Initializer_PinConfig* p = in_init->pInputsList;
        while ((NULL != p) && (in_out_sub->inputs.nbInputs < in_init->nbInputs) && (SOPC_STATUS_OK == result))
        {
            in_out_sub->inputs.pReadMode[in_out_sub->inputs.nbInputs] = p->scanMode;
            in_out_sub->inputs.ppContext[in_out_sub->inputs.nbInputs] = p->inputContext;
            in_out_sub->inputs.pMsgBox[in_out_sub->inputs.nbInputs] =
                SOPC_MsgBox_Create(p->nbClients, p->max_evts, p->max_data);

            in_out_sub->inputs.pDataHandle[in_out_sub->inputs.nbInputs] =
                SOPC_MsgBox_DataHandle_Create(in_out_sub->inputs.pMsgBox[in_out_sub->inputs.nbInputs]);

            p = p->next;

            if (in_out_sub->inputs.pMsgBox[in_out_sub->inputs.nbInputs] == NULL ||
                in_out_sub->inputs.pDataHandle[in_out_sub->inputs.nbInputs] == NULL)
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                in_out_sub->inputs.nbInputs++;
            }
        }

        p = in_init->pOutputsList;
        while ((NULL != p) && (in_out_sub->outputs.nbOutputs < in_init->nbOutputs) && (SOPC_STATUS_OK == result))
        {
            in_out_sub->outputs.pMsgBox[in_out_sub->outputs.nbOutputs] =
                SOPC_MsgBox_Create(p->nbClients, p->max_evts, p->max_data);

            in_out_sub->outputs.pDataHandle[in_out_sub->outputs.nbOutputs] =
                SOPC_MsgBox_DataHandle_Create(in_out_sub->outputs.pMsgBox[in_out_sub->outputs.nbOutputs]);

            p = p->next;

            if (in_out_sub->outputs.pMsgBox[in_out_sub->outputs.nbOutputs] == NULL ||
                in_out_sub->outputs.pDataHandle[in_out_sub->outputs.nbOutputs] == NULL)
            {
                result = SOPC_STATUS_NOK;
            }
            else
            {
                in_out_sub->outputs.nbOutputs++;
            }
        }
    }
    return result;
}

static inline void _SOPC_RT_Subscriber_DeInitialize(SOPC_RT_Subscriber* in_out_sub)
{
    if (in_out_sub->inputs.pDataHandle != NULL)
    {
        for (uint32_t i = 0; i < in_out_sub->inputs.nbInputs; i++)
        {
            SOPC_MsgBox_DataHandle_Destroy(&in_out_sub->inputs.pDataHandle[i]);
        }
        SOPC_Free(in_out_sub->inputs.pDataHandle);
        in_out_sub->inputs.pDataHandle = NULL;
    }

    if (in_out_sub->inputs.pMsgBox != NULL)
    {
        for (uint32_t i = 0; i < in_out_sub->inputs.nbInputs; i++)
        {
            SOPC_MsgBox_Destroy(&in_out_sub->inputs.pMsgBox[i]);
        }
        SOPC_Free(in_out_sub->inputs.pMsgBox);
        in_out_sub->inputs.pMsgBox = NULL;
        in_out_sub->inputs.nbInputs = 0;
    }

    if (in_out_sub->inputs.pReadMode != NULL)
    {
        SOPC_Free(in_out_sub->inputs.pReadMode);
        in_out_sub->inputs.pReadMode = NULL;
    }

    if (in_out_sub->inputs.ppContext != NULL)
    {
        SOPC_Free(in_out_sub->inputs.ppContext);
        in_out_sub->inputs.ppContext = NULL;
    }

    if (in_out_sub->outputs.pDataHandle != NULL)
    {
        for (uint32_t i = 0; i < in_out_sub->outputs.nbOutputs; i++)
        {
            SOPC_MsgBox_DataHandle_Destroy(&in_out_sub->outputs.pDataHandle[i]);
        }
        SOPC_Free(in_out_sub->outputs.pDataHandle);
        in_out_sub->outputs.pDataHandle = NULL;
    }

    if (in_out_sub->outputs.pMsgBox != NULL)
    {
        for (uint32_t i = 0; i < in_out_sub->outputs.nbOutputs; i++)
        {
            SOPC_MsgBox_Destroy(&in_out_sub->outputs.pMsgBox[i]);
        }
        SOPC_Free(in_out_sub->outputs.pMsgBox);
        in_out_sub->outputs.pMsgBox = NULL;
        in_out_sub->outputs.nbOutputs = 0;
    }
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_WriteDataHandle_GetBuffer(SOPC_RT_Subscriber* in_sub,
                                                                                  SOPC_Pin_Direction in_itf,
                                                                                  uint32_t in_pin,
                                                                                  SOPC_Buffer* in_out_sopc_buffer)
{
    if (NULL == in_sub || NULL == in_out_sopc_buffer || in_out_sopc_buffer->data != NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    switch (in_itf)
    {
    case SOPC_PIN_DIRECTION_IN:
        if (in_pin >= in_sub->inputs.nbInputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_PIN_DIRECTION_OUT:
        if (in_pin >= in_sub->outputs.nbOutputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }
    SOPC_MsgBox_DataHandle* pDataHandle = NULL;

    if (SOPC_STATUS_OK == result)
    {
        switch (in_itf)
        {
        case SOPC_PIN_DIRECTION_OUT:
            pDataHandle = in_sub->outputs.pDataHandle[in_pin];
            break;
        case SOPC_PIN_DIRECTION_IN:
            pDataHandle = in_sub->inputs.pDataHandle[in_pin];
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }

    {
        // Try initialization of data handle
        uint8_t* buffer = NULL;
        uint32_t maximumAllowedSize = 0;

        if (SOPC_STATUS_OK == result)
        {
            result = SOPC_MsgBox_DataHandle_Initialize(pDataHandle);

            if (SOPC_STATUS_OK == result)
            {
                result = SOPC_MsgBox_DataHandle_GetDataEvt(pDataHandle, &buffer, &maximumAllowedSize);
            }
        }

        if (SOPC_STATUS_OK == result)
        {
            in_out_sopc_buffer->maximum_size = maximumAllowedSize;
            in_out_sopc_buffer->initial_size = maximumAllowedSize;
            in_out_sopc_buffer->current_size = maximumAllowedSize;
            in_out_sopc_buffer->length = 0;
            in_out_sopc_buffer->position = 0;
            in_out_sopc_buffer->data = buffer;
        }
        else
        {
            in_out_sopc_buffer->maximum_size = 0;
            in_out_sopc_buffer->current_size = 0;
            in_out_sopc_buffer->initial_size = 0;
            in_out_sopc_buffer->length = 0;
            in_out_sopc_buffer->position = 0;
            in_out_sopc_buffer->data = NULL;
        }
    }
    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_WriteDataHandle_ReleaseBuffer(SOPC_RT_Subscriber* in_sub,
                                                                                      SOPC_Pin_Direction in_itf,
                                                                                      uint32_t in_pin,
                                                                                      SOPC_Buffer* in_out_sopc_buffer)
{
    if (NULL == in_sub || NULL == in_out_sopc_buffer || in_out_sopc_buffer->data == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    switch (in_itf)
    {
    case SOPC_PIN_DIRECTION_IN:
        if (in_pin >= in_sub->inputs.nbInputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_PIN_DIRECTION_OUT:
        if (in_pin >= in_sub->outputs.nbOutputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }
    SOPC_MsgBox_DataHandle* pDataHandle = NULL;

    if (SOPC_STATUS_OK == result)
    {
        switch (in_itf)
        {
        case SOPC_PIN_DIRECTION_OUT:
            pDataHandle = in_sub->outputs.pDataHandle[in_pin];
            break;
        case SOPC_PIN_DIRECTION_IN:
            pDataHandle = in_sub->inputs.pDataHandle[in_pin];
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        // Try to update data size
        bool bCancel = false;

        result = SOPC_MsgBox_DataHandle_UpdateDataEvtSize(pDataHandle, in_out_sopc_buffer->length);

        // Cancel commit in case of error
        if (SOPC_STATUS_OK == result)
        {
            bCancel = false;
        }
        else
        {
            bCancel = true;
        }

        // Only result of finalize is returned. If pDataHandle is NULL, return INVALID_PARAM
        result = SOPC_MsgBox_DataHandle_Finalize(pDataHandle, bCancel);
    }

    in_out_sopc_buffer->maximum_size = 0;
    in_out_sopc_buffer->current_size = 0;
    in_out_sopc_buffer->initial_size = 0;
    in_out_sopc_buffer->length = 0;
    in_out_sopc_buffer->position = 0;
    in_out_sopc_buffer->data = NULL;

    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Write(SOPC_RT_Subscriber* in_sub,
                                                              SOPC_Pin_Direction in_itf,
                                                              uint32_t in_pin,
                                                              uint8_t* in_data,
                                                              uint32_t in_size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_sub || NULL == in_data || in_size < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (in_itf)
    {
    case SOPC_PIN_DIRECTION_IN:
        if (in_pin >= in_sub->inputs.nbInputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_PIN_DIRECTION_OUT:
        if (in_pin >= in_sub->outputs.nbOutputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }
    SOPC_MsgBox* pMsgBox = NULL;

    if (SOPC_STATUS_OK == result)
    {
        switch (in_itf)
        {
        case SOPC_PIN_DIRECTION_OUT:
            pMsgBox = in_sub->outputs.pMsgBox[in_pin];
            break;
        case SOPC_PIN_DIRECTION_IN:
            pMsgBox = in_sub->inputs.pMsgBox[in_pin];
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        result = SOPC_MsgBox_Push(pMsgBox, in_data, in_size);
    }

    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Read_Initialize(SOPC_RT_Subscriber* in_sub,
                                                                        SOPC_Pin_Direction in_dir,
                                                                        uint32_t in_pin,
                                                                        size_t* out_token)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_sub || NULL == out_token)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (in_dir)
    {
    case SOPC_PIN_DIRECTION_IN:
        if (in_pin >= in_sub->inputs.nbInputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_PIN_DIRECTION_OUT:
        if (in_pin >= in_sub->outputs.nbOutputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    SOPC_MsgBox* pMsgBox = NULL;

    if (SOPC_STATUS_OK == result)
    {
        switch (in_dir)
        {
        case SOPC_PIN_DIRECTION_IN:
            pMsgBox = in_sub->inputs.pMsgBox[in_pin];
            break;
        case SOPC_PIN_DIRECTION_OUT:
            pMsgBox = in_sub->outputs.pMsgBox[in_pin];
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        result = SOPC_MsgBox_Pop_Initialize(pMsgBox, out_token);
    }

    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Read(SOPC_RT_Subscriber* in_pSub,
                                                             SOPC_Pin_Direction in_itf,
                                                             uint32_t in_pin,
                                                             uint32_t in_clt,
                                                             size_t in_token,
                                                             SOPC_Pin_Access in_mode,
                                                             uint8_t** out_pData,
                                                             uint32_t* out_size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_pSub || NULL == out_pData || NULL == out_size)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (in_itf)
    {
    case SOPC_PIN_DIRECTION_IN:
        if (in_pin >= in_pSub->inputs.nbInputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_PIN_DIRECTION_OUT:
        if (in_pin >= in_pSub->outputs.nbOutputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    SOPC_MsgBox* pMsgBox = NULL;

    if (SOPC_STATUS_OK == result)
    {
        switch (in_itf)
        {
        case SOPC_PIN_DIRECTION_IN:
            pMsgBox = in_pSub->inputs.pMsgBox[in_pin];

            break;
        case SOPC_PIN_DIRECTION_OUT:
            pMsgBox = in_pSub->outputs.pMsgBox[in_pin];
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }
    if (SOPC_STATUS_OK == result)
    {
        uint32_t nbPendingsEvents = 0;
        result = SOPC_MsgBox_Pop_GetEvtPtr(pMsgBox, in_token, in_clt, out_pData, out_size, &nbPendingsEvents,
                                           (SOPC_MsgBox_Mode) in_mode);
    }
    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Pin_Read_Finalize(SOPC_RT_Subscriber* in_sub,
                                                                      SOPC_Pin_Direction in_itf,
                                                                      uint32_t in_pin,
                                                                      size_t* in_out_token)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_sub || NULL == in_out_token)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (in_itf)
    {
    case SOPC_PIN_DIRECTION_IN:
        if (in_pin >= in_sub->inputs.nbInputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_PIN_DIRECTION_OUT:
        if (in_pin >= in_sub->outputs.nbOutputs)
        {
            result = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    SOPC_MsgBox* pMsgBox = NULL;

    if (SOPC_STATUS_OK == result)
    {
        switch (in_itf)
        {
        case SOPC_PIN_DIRECTION_IN:
            pMsgBox = in_sub->inputs.pMsgBox[in_pin];
            break;
        case SOPC_PIN_DIRECTION_OUT:
            pMsgBox = in_sub->outputs.pMsgBox[in_pin];
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }
    if (SOPC_STATUS_OK == result)
    {
        result = SOPC_MsgBox_Pop_Finalize(pMsgBox, in_out_token);
    }

    return result;
}

static inline SOPC_ReturnStatus _SOPC_RT_Subscriber_Initializer_AddPin(SOPC_RT_Subscriber_Initializer* in_out_init,
                                                                       uint32_t in_max_clients,
                                                                       uint32_t in_max_evts,
                                                                       uint32_t in_max_data,
                                                                       SOPC_Pin_Direction in_mode,
                                                                       SOPC_Pin_Access in_scanmode,
                                                                       void* in_input_context,
                                                                       uint32_t* out_pinNumber)

{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_out_init || in_max_evts < 1 || in_max_data < 1 || in_max_clients < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_RT_Subscriber_Initializer_PinConfig* pIter = NULL;

    switch (in_mode)
    {
    case SOPC_PIN_DIRECTION_OUT:
        pIter = in_out_init->pOutputsList;
        break;
    case SOPC_PIN_DIRECTION_IN:
        pIter = in_out_init->pInputsList;
        break;
    default:
        result = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    SOPC_RT_Subscriber_Initializer_PinConfig* pIterPrevious = NULL;
    SOPC_RT_Subscriber_Initializer_PinConfig* pNewPin = NULL;

    if (SOPC_STATUS_OK == result)
    {
        pIterPrevious = NULL;
        pNewPin = SOPC_Calloc(1, sizeof(SOPC_RT_Subscriber_Initializer_PinConfig));
    }

    if (NULL == pNewPin)
    {
        result = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == result)
    {
        pNewPin->nbClients = in_max_clients;
        pNewPin->max_evts = in_max_evts;
        pNewPin->max_data = in_max_data;
        pNewPin->scanMode = in_scanmode;
        pNewPin->inputContext = in_input_context;

        while (pIter != NULL)
        {
            pIterPrevious = pIter;
            pIter = pIter->next;
        }

        if (pIterPrevious == NULL)
        {
            switch (in_mode)
            {
            case SOPC_PIN_DIRECTION_OUT:
                in_out_init->pOutputsList = pNewPin;
                break;
            case SOPC_PIN_DIRECTION_IN:
                in_out_init->pInputsList = pNewPin;
                break;
            default:
                result = SOPC_STATUS_INVALID_PARAMETERS;
                break;
            }
        }
        else
        {
            pIterPrevious->next = pNewPin;
        }

        switch (in_mode)
        {
        case SOPC_PIN_DIRECTION_OUT:
            *out_pinNumber = in_out_init->nbOutputs;
            in_out_init->nbOutputs++;
            break;
        case SOPC_PIN_DIRECTION_IN:
            *out_pinNumber = in_out_init->nbInputs;
            in_out_init->nbInputs++;
            break;
        default:
            result = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }

    // Keep my ass clean
    if (SOPC_STATUS_OK != result)
    {
        if (pNewPin != NULL)
        {
            SOPC_Free(pNewPin);
            pNewPin = NULL;
        }
    }

    return result;
}

static inline eSubscriberSyncStatus _SOPC_RT_Subscriber_IncrementInUseStatus(SOPC_RT_Subscriber* in_sub)
{
    if (NULL == in_sub)
    {
        return E_SUBSCRIBER_SYNC_STATUS_SIZE;
    }

    eSubscriberSyncStatus currentStatus = 0;
    eSubscriberSyncStatus newStatus = E_SUBSCRIBER_SYNC_STATUS_SIZE;
    bool bTransition = false;

    do
    {
        __atomic_load(&in_sub->status, &currentStatus, __ATOMIC_SEQ_CST);
        if (currentStatus >= E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
        {
            newStatus = currentStatus + 1;
        }
        else
        {
            newStatus = currentStatus;
        }
        bTransition = __atomic_compare_exchange(&in_sub->status, &currentStatus, &newStatus, false, __ATOMIC_SEQ_CST,
                                                __ATOMIC_SEQ_CST);
    } while (!bTransition);

    return newStatus;
}

static inline eSubscriberSyncStatus _SOPC_RT_Subscriber_DecrementInUseStatus(SOPC_RT_Subscriber* in_sub)
{
    if (NULL == in_sub)
    {
        return E_SUBSCRIBER_SYNC_STATUS_SIZE;
    }

    eSubscriberSyncStatus currentStatus = 0;
    eSubscriberSyncStatus newStatus = E_SUBSCRIBER_SYNC_STATUS_SIZE;
    bool bTransition = false;

    do
    {
        __atomic_load(&in_sub->status, &currentStatus, __ATOMIC_SEQ_CST);
        if (currentStatus > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
        {
            newStatus = currentStatus - 1;
        }
        else
        {
            newStatus = currentStatus;
        }
        bTransition = __atomic_compare_exchange(&in_sub->status, &currentStatus, &newStatus, false, __ATOMIC_SEQ_CST,
                                                __ATOMIC_SEQ_CST);
    } while (!bTransition);

    return newStatus;
}
