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

#include "sopc_rt_subscriber.h"

// Private status of RT Publisher
typedef enum E_SUBSCRIBER_SYNC_STATUS
{
    E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED,
    E_SUBSCRIBER_SYNC_STATUS_INITIALIZING,
    E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING,
    E_SUBSCRIBER_SYNC_STATUS_INITIALIZED,
    E_SUBSCRIBER_SYNC_STATUS_LOCKED,
    E_SUBSCRIBER_SYNC_STATUS_SIZE = INT32_MAX
} eSubscriberSyncStatus;

// Inputs table definition
typedef struct SOPC_RT_Subscriber_Inputs
{
    uint32_t nbInputs;          // Number of inputs
    SOPC_MsgBox** pMsgBox;      // Table of inputs
    SOPC_Pin_Access* pReadMode; // Table of scan mode for each input
    void** ppContext;           // Table of user context for each input
} SOPC_RT_Subscriber_Inputs;

// Output table definition
typedef struct SOPC_RT_Subscriber_Outputs
{
    uint32_t nbOutputs;    // Number of outputs
    SOPC_MsgBox** pMsgBox; // Table of outputs
} SOPC_RT_Subscriber_Outputs;

// Subscriber definition
struct SOPC_RT_Subscriber
{
    eSubscriberSyncStatus status;       // RT subscriber status
    SOPC_RT_Subscriber_Inputs inputs;   // Inputs
    SOPC_RT_Subscriber_Outputs outputs; // Outputs
    ptrBeatHeartCallback cbStep;        // Step callback
    void* pUserContext;                 // User context
};

// Subscriber configuration

typedef struct SOPC_RT_Subscriber_Initializer_PinConfig
{
    uint32_t max_evts;        // Max events supported by pin before overflow
    uint32_t max_data;        // Max data supported by pin before overflow
    uint32_t nbClients;       // Max concurrent clients which can read same pin.
    SOPC_Pin_Access scanMode; // Used only for inputs.
    void* inputContext;       // Used only for inputs.
    struct SOPC_RT_Subscriber_Initializer_PinConfig* next;
} SOPC_RT_Subscriber_Initializer_PinConfig;

struct SOPC_RT_Subscriber_Initializer
{
    uint32_t nbInputs;
    SOPC_RT_Subscriber_Initializer_PinConfig* pInputsList;

    uint32_t nbOutputs;
    SOPC_RT_Subscriber_Initializer_PinConfig* pOutputsList;

    ptrBeatHeartCallback cbStep;
    void* pUserContext;
};

// Declarations of private functions

static inline eSubscriberSyncStatus __sync_increment_subscriber_in_use_counter(SOPC_RT_Subscriber* in_sub);

static inline eSubscriberSyncStatus __sync_decrement_subscriber_in_use_counter(SOPC_RT_Subscriber* in_sub);

static inline SOPC_ReturnStatus __sopc_rt_subscriber_initialize(SOPC_RT_Subscriber* in_out_sub,           //
                                                                SOPC_RT_Subscriber_Initializer* in_init); //

static inline void __sopc_rt_subscriber_deinitialize(SOPC_RT_Subscriber* in_sub);

static inline SOPC_ReturnStatus __sopc_rt_subscriber_initializer_addpin(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_clients,                     // Max client
    uint32_t in_max_evts,                        // Max events supported by this output before overflow
    uint32_t in_max_data,                        // Max data supported by this output
    SOPC_Pin_Direction in_mode,                  // Direction
    SOPC_Pin_Access in_scanmode,                 // Scan mode
    void* in_input_context,
    uint32_t* out_pinNumber); // pin number.

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinreadinitialize(
    SOPC_RT_Subscriber* in_sub, // RT subscriber object
    SOPC_Pin_Direction in_dir,  // Pin direction
    uint32_t in_pin,            // pin number
    uint32_t* out_token);       // token of read sequence

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinread(SOPC_RT_Subscriber* in_pSub, // RT Subscriber object
                                                             SOPC_Pin_Direction in_itf,   // pin direction
                                                             uint32_t in_pin,             // Pin number
                                                             uint32_t in_clt,             // Client number
                                                             uint32_t in_token,           // Token of read sequence
                                                             SOPC_Pin_Access in_mode,     // Access mode
                                                             uint8_t** out_pData,         // Pointer on data
                                                             uint32_t* out_size);         // Size of data

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinreadfinalize(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                                     SOPC_Pin_Direction in_itf,  // Pin direction
                                                                     uint32_t in_pin,            // Pin number
                                                                     uint32_t* in_out_token);    // token

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinwrite(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                              SOPC_Pin_Direction in_itf,  // Pin direction
                                                              uint32_t in_pin,            // Pin number
                                                              uint8_t* in_data,           // Data to write
                                                              uint32_t in_size);          // Size to write

static inline SOPC_ReturnStatus __sopc_rt_subscriber_beatheart(SOPC_RT_Subscriber* in_sub);

// *** Definitions of public functions ***

// RT Subscriber object creation
SOPC_RT_Subscriber* SOPC_RT_Subscriber_Create(void)
{
    SOPC_RT_Subscriber* pSub = SOPC_Calloc(1, sizeof(SOPC_RT_Subscriber));
    return pSub;
}

// RT Subscriber object destruction
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

// RT Subscriber object initialization
SOPC_ReturnStatus SOPC_RT_Subscriber_Initialize(SOPC_RT_Subscriber* in_out_sub,          // RT Subscriber object
                                                SOPC_RT_Subscriber_Initializer* in_init) // Initializer object
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_out_sub || NULL == in_init)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus expectedStatus = E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED;
    eSubscriberSyncStatus desiredStatus = E_SUBSCRIBER_SYNC_STATUS_INITIALIZING;
    bool bTransition = __atomic_compare_exchange(&in_out_sub->status, //
                                                 &expectedStatus,     //
                                                 &desiredStatus,      //
                                                 false,               //
                                                 __ATOMIC_SEQ_CST,    //
                                                 __ATOMIC_SEQ_CST);   //

    if (bTransition)
    {
        result = __sopc_rt_subscriber_initialize(in_out_sub, in_init);
        if (SOPC_STATUS_OK == result)
        {
            desiredStatus = E_SUBSCRIBER_SYNC_STATUS_INITIALIZED;
            __atomic_store(&in_out_sub->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            __sopc_rt_subscriber_deinitialize(in_out_sub);
            desiredStatus = E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED;
            __atomic_store(&in_out_sub->status, &desiredStatus, __ATOMIC_SEQ_CST);
        }
    }
    else if (E_SUBSCRIBER_SYNC_STATUS_INITIALIZING == expectedStatus ||   //
             E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING == expectedStatus || //
             E_SUBSCRIBER_SYNC_STATUS_LOCKED <= expectedStatus)           //
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// RT Subscriber de-initialization
SOPC_ReturnStatus SOPC_RT_Subscriber_DeInitialize(SOPC_RT_Subscriber* in_out_sub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_out_sub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus expectedStatus = E_SUBSCRIBER_SYNC_STATUS_INITIALIZED;
    eSubscriberSyncStatus desiredStatus = E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING;
    bool bTransition = __atomic_compare_exchange(&in_out_sub->status, //
                                                 &expectedStatus,     //
                                                 &desiredStatus,      //
                                                 false,               //
                                                 __ATOMIC_SEQ_CST,    //
                                                 __ATOMIC_SEQ_CST);   //

    if (bTransition)
    {
        __sopc_rt_subscriber_deinitialize(in_out_sub);

        desiredStatus = E_SUBSCRIBER_SYNC_STATUS_NOT_INITIALIZED;
        __atomic_store(&in_out_sub->status, &desiredStatus, __ATOMIC_SEQ_CST);
    }
    else if (E_SUBSCRIBER_SYNC_STATUS_INITIALIZING == expectedStatus ||   //
             E_SUBSCRIBER_SYNC_STATUS_DEINITIALIZING == expectedStatus || //
             E_SUBSCRIBER_SYNC_STATUS_LOCKED <= expectedStatus)           //
    {
        result = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        result = SOPC_STATUS_NOK;
    }

    return result;
}

// RT Subscriber Initializer Creation
SOPC_RT_Subscriber_Initializer* SOPC_RT_Subscriber_Initializer_Create(ptrBeatHeartCallback cbStep, void* pContext)
{
    if (NULL == cbStep)
    {
        return NULL;
    }
    SOPC_RT_Subscriber_Initializer* pInit = SOPC_Calloc(1, sizeof(SOPC_RT_Subscriber_Initializer));
    if (NULL != pInit)
    {
        pInit->cbStep = cbStep;
        pInit->pUserContext = pContext;
    }
    return pInit;
}

// RT Subscriber Initializer Destruction
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

// Add input definition to initializer object
SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddInput(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_evts,                        // Max events supported by this input before overflow
    uint32_t in_max_data,                        // Max data supported by this input before overflow
    SOPC_Pin_Access in_scanmode,                 // Get last, get last new, get normal
    void* in_input_context,                      // User context
    uint32_t* out_pin)                           // Pin number associated

{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    result = __sopc_rt_subscriber_initializer_addpin(in_out_init,           //
                                                     1,                     //
                                                     in_max_evts,           //
                                                     in_max_data,           //
                                                     SOPC_PIN_DIRECTION_IN, //
                                                     in_scanmode,           //
                                                     in_input_context,      //
                                                     out_pin);              //

    return result;
}

// Add output definition to initialize object
SOPC_ReturnStatus SOPC_RT_Subscriber_Initializer_AddOutput(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_clients,                     // Max concurrent clients supported by this output
    uint32_t in_max_evts,                        // Max events supported by this output before overflow
    uint32_t in_max_data,                        // Max data supported by this output before overflow
    uint32_t* out_pin)                           // Pin number associated to this output

{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    result = __sopc_rt_subscriber_initializer_addpin(in_out_init,              //
                                                     in_max_clients,           //
                                                     in_max_evts,              //
                                                     in_max_data,              //
                                                     SOPC_PIN_DIRECTION_OUT,   //
                                                     SOPC_PIN_MODE_GET_NORMAL, // Not used
                                                     NULL,                     // Not used
                                                     out_pin);                 //

    return result;
}

// Read output sequence initialization. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Initialize(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                            uint32_t in_pin,            // Pin number to read
                                                            uint32_t* out_token) // Token to use with read function
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == out_token)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = __sync_increment_subscriber_in_use_counter(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = __sopc_rt_subscriber_pinreadinitialize(in_sub,                 //
                                                        SOPC_PIN_DIRECTION_OUT, //
                                                        in_pin,                 //
                                                        out_token);             //

        // Do not restore previous state. Mark as "in use" until that finalize call.
        if (SOPC_STATUS_OK == result)
        {
            // A successful initialization indicates that message box pointer can be used out of protection
            // phase whereas a de initialization phase try to start.
            // Increment this counter. Decremented after successful finalization.
            __sync_increment_subscriber_in_use_counter(in_sub);
        }

        __sync_decrement_subscriber_in_use_counter(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Read output sequence. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read(SOPC_RT_Subscriber* in_sub,  // RT Subscriber object
                                                 uint32_t in_pin,             // Pin number
                                                 uint32_t in_clt,             // Client number
                                                 uint32_t in_token,           // Token returned by Read Initialize
                                                 SOPC_Pin_Access in_scanmode, // Access mode
                                                 uint8_t** out_pData,         // Data pointer
                                                 uint32_t* out_size)          // Data size
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == out_pData || NULL == out_size)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = __sync_increment_subscriber_in_use_counter(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = __sopc_rt_subscriber_pinread(in_sub,                 //
                                              SOPC_PIN_DIRECTION_OUT, //
                                              in_pin,                 //
                                              in_clt,                 //
                                              in_token,               //
                                              in_scanmode,            //
                                              out_pData,              //
                                              out_size);              //

        __sync_decrement_subscriber_in_use_counter(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Read output sequence finalization. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Read_Finalize(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                          uint32_t in_pin,            // Pin number
                                                          uint32_t* out_token)        // Token
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || NULL == out_token)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = __sync_increment_subscriber_in_use_counter(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = __sopc_rt_subscriber_pinreadfinalize(in_sub,                 //
                                                      SOPC_PIN_DIRECTION_OUT, //
                                                      in_pin,                 //
                                                      out_token);             //

        if (SOPC_STATUS_OK == result)
        {
            // A successful finalization indicates that message box pointer will not be further
            // used out of protection
            // De initialization phase  is allowed.
            __sync_decrement_subscriber_in_use_counter(in_sub);
        }

        __sync_decrement_subscriber_in_use_counter(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Write input. Used outside RT Subscriber.
SOPC_ReturnStatus SOPC_RT_Subscriber_Input_Write(SOPC_RT_Subscriber* in_sub, // RT Subscriber object
                                                 uint32_t in_pin,            // Input pin number
                                                 uint8_t* in_data,           // Data to write
                                                 uint32_t in_size)           // Data size
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || in_data == NULL || in_size < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = __sync_increment_subscriber_in_use_counter(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = __sopc_rt_subscriber_pinwrite(in_sub,                //
                                               SOPC_PIN_DIRECTION_IN, //
                                               in_pin,                //
                                               in_data,               //
                                               in_size);              //

        __sync_decrement_subscriber_in_use_counter(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Write output. Used inside RT Subscriber, in the user callback to update an output.
SOPC_ReturnStatus SOPC_RT_Subscriber_Output_Write(SOPC_RT_Subscriber* in_sub, // RT Subscriber
                                                  uint32_t in_pin,            // Pin number
                                                  uint8_t* in_data,           // Data to write
                                                  uint32_t in_size)           // Size to write
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub || in_data == NULL || in_size < 1)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = __sync_increment_subscriber_in_use_counter(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = __sopc_rt_subscriber_pinwrite(in_sub,                 //
                                               SOPC_PIN_DIRECTION_OUT, //
                                               in_pin,                 //
                                               in_data,                //
                                               in_size);               //

        __sync_decrement_subscriber_in_use_counter(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Beat heart. Use to read each input.
// For each read input, user callback is invoked.
SOPC_ReturnStatus SOPC_RT_Subscriber_HeartBeat(SOPC_RT_Subscriber* in_sub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == in_sub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    eSubscriberSyncStatus status = __sync_increment_subscriber_in_use_counter(in_sub);

    if (status > E_SUBSCRIBER_SYNC_STATUS_INITIALIZED)
    {
        result = __sopc_rt_subscriber_beatheart(in_sub);
        __sync_decrement_subscriber_in_use_counter(in_sub);
    }
    else
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    return result;
}

// Private functions definitions

static inline SOPC_ReturnStatus __sopc_rt_subscriber_beatheart(SOPC_RT_Subscriber* in_sub)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == in_sub || NULL == in_sub->cbStep)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t token = 0;
    uint32_t size = 0;
    uint8_t* pData = NULL;

    for (uint32_t pin = 0; pin < in_sub->inputs.nbInputs; pin++)
    {
        token = 0;
        size = 0;
        pData = NULL;

        result = __sopc_rt_subscriber_pinreadinitialize(in_sub,                //
                                                        SOPC_PIN_DIRECTION_IN, //
                                                        pin,                   //
                                                        &token);               //

        if (SOPC_STATUS_OK == result)
        {
            do
            {
                result = __sopc_rt_subscriber_pinread(in_sub,                        //
                                                      SOPC_PIN_DIRECTION_IN,         //
                                                      pin,                           //
                                                      0,                             //
                                                      token,                         //
                                                      in_sub->inputs.pReadMode[pin], //
                                                      &pData,                        //
                                                      &size);                        //

                if (pData != NULL && size > 0 && SOPC_STATUS_OK == result)
                {
                    result = in_sub->cbStep(in_sub,               //
                                            in_sub->pUserContext, //
                                            in_sub->inputs.ppContext[pin],
                                            pin,   //
                                            pData, //
                                            size); //
                }

            } while ((pData != NULL)                                                //
                     && (size > 0)                                                  //
                     && (SOPC_STATUS_OK == result)                                  //
                     && (in_sub->inputs.pReadMode[pin] != SOPC_PIN_MODE_GET_LAST)); //

            __sopc_rt_subscriber_pinreadfinalize(in_sub, SOPC_PIN_DIRECTION_IN, pin, &token);
        }
    }

    return result;
}

static inline SOPC_ReturnStatus __sopc_rt_subscriber_initialize(SOPC_RT_Subscriber* in_out_sub,          //
                                                                SOPC_RT_Subscriber_Initializer* in_init) //
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
            in_out_sub->inputs.pMsgBox[in_out_sub->inputs.nbInputs] = SOPC_MsgBox_Create(p->nbClients, //
                                                                                         p->max_evts,  //
                                                                                         p->max_data); //
            p = p->next;

            if (in_out_sub->inputs.pMsgBox[in_out_sub->inputs.nbInputs] == NULL)
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
            in_out_sub->outputs.pMsgBox[in_out_sub->outputs.nbOutputs] = SOPC_MsgBox_Create(p->nbClients, //
                                                                                            p->max_evts,  //
                                                                                            p->max_data); //
            p = p->next;

            if (in_out_sub->outputs.pMsgBox[in_out_sub->outputs.nbOutputs] == NULL)
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

static inline void __sopc_rt_subscriber_deinitialize(SOPC_RT_Subscriber* in_sub)
{
    if (in_sub->inputs.pMsgBox != NULL)
    {
        for (uint32_t i = 0; i < in_sub->inputs.nbInputs; i++)
        {
            SOPC_MsgBox_Destroy(&in_sub->inputs.pMsgBox[i]);
        }
        SOPC_Free(in_sub->inputs.pMsgBox);
        in_sub->inputs.pMsgBox = NULL;
        in_sub->inputs.nbInputs = 0;
    }

    if (in_sub->inputs.pReadMode != NULL)
    {
        SOPC_Free(in_sub->inputs.pReadMode);
        in_sub->inputs.pReadMode = NULL;
    }

    if (in_sub->inputs.ppContext != NULL)
    {
        SOPC_Free(in_sub->inputs.ppContext);
        in_sub->inputs.ppContext = NULL;
    }

    if (in_sub->outputs.pMsgBox != NULL)
    {
        for (uint32_t i = 0; i < in_sub->outputs.nbOutputs; i++)
        {
            SOPC_MsgBox_Destroy(&in_sub->outputs.pMsgBox[i]);
        }
        SOPC_Free(in_sub->outputs.pMsgBox);
        in_sub->outputs.pMsgBox = NULL;
        in_sub->outputs.nbOutputs = 0;
    }
}

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinwrite(SOPC_RT_Subscriber* in_sub, //
                                                              SOPC_Pin_Direction in_itf,  //
                                                              uint32_t in_pin,
                                                              uint8_t* in_data, //
                                                              uint32_t in_size) //
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
        result = SOPC_MsgBox_Push(pMsgBox,  //
                                  in_data,  //
                                  in_size); //
    }

    return result;
}

// Initialize read sequence for handle
static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinreadinitialize(SOPC_RT_Subscriber* in_sub, //
                                                                       SOPC_Pin_Direction in_dir,  //
                                                                       uint32_t in_pin,            //
                                                                       uint32_t* out_token)        //
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
        result = SOPC_MsgBox_Pop_Initialize(pMsgBox,    //
                                            out_token); //
    }

    return result;
}

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinread(SOPC_RT_Subscriber* in_pSub, //
                                                             SOPC_Pin_Direction in_itf,   //
                                                             uint32_t in_pin,             //
                                                             uint32_t in_clt,             //
                                                             uint32_t in_token,           //
                                                             SOPC_Pin_Access in_mode,     //
                                                             uint8_t** out_pData,         //
                                                             uint32_t* out_size)          //
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
        result = SOPC_MsgBox_Pop_GetEvtPtr(pMsgBox,                     //
                                           in_token,                    //
                                           in_clt,                      //
                                           out_pData,                   //
                                           out_size,                    //
                                           &nbPendingsEvents,           //
                                           (SOPC_MsgBox_Mode) in_mode); //
    }
    return result;
}

static inline SOPC_ReturnStatus __sopc_rt_subscriber_pinreadfinalize(SOPC_RT_Subscriber* in_sub, //
                                                                     SOPC_Pin_Direction in_itf,  //
                                                                     uint32_t in_pin,            //
                                                                     uint32_t* in_out_token)     //
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
        result = SOPC_MsgBox_Pop_Finalize(pMsgBox,       //
                                          in_out_token); //
    }

    return result;
}

// Add an output to initializer
static inline SOPC_ReturnStatus __sopc_rt_subscriber_initializer_addpin(
    SOPC_RT_Subscriber_Initializer* in_out_init, // Initializer object
    uint32_t in_max_clients,                     //
    uint32_t in_max_evts,                        // Max events supported by this output before overflow
    uint32_t in_max_data,                        // Max data supported by this output
    SOPC_Pin_Direction in_mode,                  // Direction
    SOPC_Pin_Access accessMode,                  // Used only by RT Subscriber Step function which scan inputs.
    void* in_input_context,                      // Used only by RT Subscriber Step function which can inputs
    uint32_t* pinNumber)                         // Output number. Used inside user callback to update output.

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
        pNewPin->scanMode = accessMode;
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
            *pinNumber = in_out_init->nbOutputs;
            in_out_init->nbOutputs++;
            break;
        case SOPC_PIN_DIRECTION_IN:
            *pinNumber = in_out_init->nbInputs;
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

static inline eSubscriberSyncStatus __sync_increment_subscriber_in_use_counter(SOPC_RT_Subscriber* in_sub)
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

static inline eSubscriberSyncStatus __sync_decrement_subscriber_in_use_counter(SOPC_RT_Subscriber* in_sub)
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
