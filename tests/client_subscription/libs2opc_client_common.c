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

/** \file
 *
 * \brief A client library that supports and automates the subscription.
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "sopc_array.h"
#include "sopc_atomic.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_config.h"
#include "sopc_version.h"

#include "sopc_encodeable.h"
#include "sopc_toolkit_async_api.h"

#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_time.h"
#include "sopc_types.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client_common.h"

#include "state_machine.h"
#include "toolkit_helpers.h"

/* =========
 * Internals
 * =========
 */

#define CONNECTION_TIMEOUT_MS_STEP 50

/* Client structures */

/* Global library variables */
static int32_t libInitialized = 0;
static int32_t libConfigured = 0;
static Mutex mutex; /* Mutex which protects global variables except libInitialized and libConfigured */
static SOPC_LibSub_DisconnectCbk cbkDisco = NULL;
static SOPC_ClientCommon_GetEndpointsCbk getEndpointsCbk = NULL;
static SOPC_SLinkedList* pListConfig = NULL; /* IDs are cfgId == Toolkit cfgScId, value is SOPC_LibSub_ConnectionCfg */
static SOPC_SLinkedList* pListClient = NULL; /* IDs are cliId, value is a StaMac */
static SOPC_LibSub_ConnectionId nCreatedClient = 0;
static SOPC_Array* pArrScConfig = NULL; /* Stores the created scConfig to free them in SOPC_LibSub_Clear() */

/* Event callback */
static void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);

/* ==================
 * API implementation
 * ==================
 */

SOPC_ReturnStatus SOPC_ClientCommon_Initialize(const SOPC_LibSub_StaticCfg* pCfg,
                                               const SOPC_ClientCommon_GetEndpointsCbk cbkGetEndpoints)
{
    if (NULL == pCfg || NULL == pCfg->host_log_callback || NULL == pCfg->disconnect_callback)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_Atomic_Int_Get(&libInitialized) != 0 || SOPC_Atomic_Int_Get(&libConfigured) != 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = Mutex_Initialization(&mutex);

    if (SOPC_STATUS_OK == status)
    {
        pListConfig = SOPC_SLinkedList_Create(0);
        pListClient = SOPC_SLinkedList_Create(0);
        pArrScConfig = SOPC_Array_Create(sizeof(SOPC_SecureChannel_Config*), 0,
                                         (SOPC_Array_Free_Func) Helpers_SecureChannel_Config_Free);
        if (NULL == pListConfig || NULL == pListClient || NULL == pArrScConfig)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        Helpers_SetLogger(pCfg->host_log_callback);
        cbkDisco = pCfg->disconnect_callback;
        status = SOPC_Toolkit_Initialize(ToolkitEventCallback);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitConfig_SetLogLevel(pCfg->toolkit_logger.level);
    }

    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_ToolkitConfig_SetCircularLogProperties(pCfg->toolkit_logger.maxBytes, pCfg->toolkit_logger.maxFiles);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (NULL == pCfg->toolkit_logger.log_path)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Log Path is set to null.");
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            status = SOPC_ToolkitConfig_SetCircularLogPath(pCfg->toolkit_logger.log_path, true);
        }
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Could not configure SDK logger.");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        getEndpointsCbk = cbkGetEndpoints;
    }

    if (SOPC_STATUS_OK != status)
    {
        /* Clean partial mallocs */
        SOPC_SLinkedList_Delete(pListConfig);
        pListConfig = NULL;
        SOPC_SLinkedList_Delete(pListClient);
        pListClient = NULL;
        SOPC_Array_Delete(pArrScConfig);
        pArrScConfig = NULL;
    }
    else
    {
        SOPC_Atomic_Int_Set(&libInitialized, 1);
    }

    return status;
}

void SOPC_ClientCommon_Clear(void)
{
    if (SOPC_Atomic_Int_Get(&libInitialized) == 0)
    {
        return;
    }

    SOPC_Toolkit_Clear();

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    SOPC_SLinkedListIterator pIter = NULL;
    SOPC_StaMac_Machine* pSM = NULL;
    SOPC_LibSub_ConnectionCfg* pCfg = NULL;

    SOPC_Atomic_Int_Set(&libInitialized, 0);
    SOPC_Atomic_Int_Set(&libConfigured, 0);

    pIter = SOPC_SLinkedList_GetIterator(pListClient);
    while (NULL != pIter)
    {
        pSM = (SOPC_StaMac_Machine*) SOPC_SLinkedList_Next(&pIter);
        SOPC_StaMac_Delete(&pSM);
    }
    SOPC_SLinkedList_Delete(pListClient);
    pListClient = NULL;

    pIter = SOPC_SLinkedList_GetIterator(pListConfig);
    while (NULL != pIter)
    {
        pCfg = (SOPC_LibSub_ConnectionCfg*) SOPC_SLinkedList_Next(&pIter);
        if (NULL != pCfg)
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Free((void*) pCfg->server_url);
            SOPC_Free((void*) pCfg->security_policy);
            SOPC_Free((void*) pCfg->path_cert_auth);
            SOPC_Free((void*) pCfg->path_cert_srv);
            SOPC_Free((void*) pCfg->path_cert_cli);
            SOPC_Free((void*) pCfg->path_key_cli);
            SOPC_Free((void*) pCfg->path_crl);
            SOPC_Free((void*) pCfg->policyId);
            SOPC_Free((void*) pCfg->username);
            SOPC_Free((void*) pCfg->password);
            SOPC_GCC_DIAGNOSTIC_RESTORE
            SOPC_Free(pCfg);
        }
    }
    SOPC_SLinkedList_Delete(pListConfig);
    pListConfig = NULL;

    SOPC_Array_Delete(pArrScConfig);
    pArrScConfig = NULL;

    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);
    Mutex_Clear(&mutex);
}

SOPC_ReturnStatus SOPC_ClientCommon_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                        SOPC_LibSub_ConfigurationId* pCfgId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    uint32_t cfgId = 0;
    SOPC_LibSub_ConnectionCfg* pCfgCpy = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == pCfg || NULL == pCfgId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pCfg->policyId)
    {
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Cannot configure connection with NULL policyId.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Create the new configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = Helpers_NewSCConfigFromLibSubCfg(pCfg->server_url, pCfg->security_policy, pCfg->security_mode,
                                                  pCfg->disable_certificate_verification, pCfg->path_cert_auth,
                                                  pCfg->path_cert_srv, pCfg->path_cert_cli, pCfg->path_key_cli,
                                                  pCfg->path_crl, pCfg->sc_lifetime, &pscConfig);
    }

    /* Store it to be able to free it on clear in SOPC_LibSub_Clear() */
    if (SOPC_STATUS_OK == status)
    {
        if (!SOPC_Array_Append(pArrScConfig, pscConfig))
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Add it to the Toolkit */
    if (SOPC_STATUS_OK == status)
    {
        /* TODO: store pscConfigs to free their content later */
        cfgId = SOPC_ToolkitClient_AddSecureChannelConfig(pscConfig);
        if (0 == cfgId)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Copy the caller's ConnectionCfg to append it safely to the internal list */
    if (SOPC_STATUS_OK == status)
    {
        pCfgCpy = SOPC_Calloc(1, sizeof(SOPC_LibSub_ConnectionCfg));
        if (NULL == pCfgCpy)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            pCfgCpy->security_mode = pCfg->security_mode;
            pCfgCpy->disable_certificate_verification = pCfg->disable_certificate_verification;
            pCfgCpy->publish_period_ms = pCfg->publish_period_ms;
            pCfgCpy->n_max_keepalive = pCfg->n_max_keepalive;
            pCfgCpy->n_max_lifetime = pCfg->n_max_lifetime;
            pCfgCpy->data_change_callback = pCfg->data_change_callback;
            pCfgCpy->timeout_ms = pCfg->timeout_ms;
            pCfgCpy->sc_lifetime = pCfg->sc_lifetime;
            pCfgCpy->token_target = pCfg->token_target;
            pCfgCpy->generic_response_callback = pCfg->generic_response_callback;

            /* These 3 strings are verified non NULL */
            pCfgCpy->server_url = SOPC_Malloc(strlen(pCfg->server_url) + 1);
            pCfgCpy->security_policy = SOPC_Malloc(strlen(pCfg->security_policy) + 1);
            pCfgCpy->policyId = SOPC_Malloc(strlen(pCfg->policyId) + 1);
            if (NULL == pCfgCpy->server_url || NULL == pCfgCpy->security_policy || NULL == pCfgCpy->policyId)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            if (NULL != pCfg->path_cert_auth)
            {
                pCfgCpy->path_cert_auth = SOPC_Malloc(strlen(pCfg->path_cert_auth) + 1);
                if (NULL == pCfgCpy->path_cert_auth)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (NULL != pCfg->path_cert_srv)
            {
                pCfgCpy->path_cert_srv = SOPC_Malloc(strlen(pCfg->path_cert_srv) + 1);
                if (NULL == pCfgCpy->path_cert_srv)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (NULL != pCfg->path_cert_cli)
            {
                pCfgCpy->path_cert_cli = SOPC_Malloc(strlen(pCfg->path_cert_cli) + 1);
                if (NULL == pCfgCpy->path_cert_cli)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (NULL != pCfg->path_key_cli)
            {
                pCfgCpy->path_key_cli = SOPC_Malloc(strlen(pCfg->path_key_cli) + 1);
                if (NULL == pCfgCpy->path_key_cli)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (NULL != pCfg->path_crl)
            {
                pCfgCpy->path_crl = SOPC_Malloc(strlen(pCfg->path_crl) + 1);
                if (NULL == pCfgCpy->path_crl)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (NULL != pCfg->username)
            {
                pCfgCpy->username = SOPC_Malloc(strlen(pCfg->username) + 1);
                if (NULL == pCfgCpy->username)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }
            if (NULL != pCfg->password)
            {
                pCfgCpy->password = SOPC_Malloc(strlen(pCfg->password) + 1);
                if (NULL == pCfgCpy->password)
                {
                    status = SOPC_STATUS_OUT_OF_MEMORY;
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
                strcpy((char*) pCfgCpy->server_url, pCfg->server_url);
                strcpy((char*) pCfgCpy->security_policy, pCfg->security_policy);
                strcpy((char*) pCfgCpy->policyId, pCfg->policyId);

                if (NULL != pCfg->path_cert_auth)
                {
                    strcpy((char*) pCfgCpy->path_cert_auth, pCfg->path_cert_auth);
                }
                if (NULL != pCfg->path_cert_srv)
                {
                    strcpy((char*) pCfgCpy->path_cert_srv, pCfg->path_cert_srv);
                }
                if (NULL != pCfg->path_cert_cli)
                {
                    strcpy((char*) pCfgCpy->path_cert_cli, pCfg->path_cert_cli);
                }
                if (NULL != pCfg->path_key_cli)
                {
                    strcpy((char*) pCfgCpy->path_key_cli, pCfg->path_key_cli);
                }
                if (NULL != pCfg->path_crl)
                {
                    strcpy((char*) pCfgCpy->path_crl, pCfg->path_crl);
                }
                if (NULL != pCfg->username)
                {
                    strcpy((char*) pCfgCpy->username, pCfg->username);
                }
                if (NULL != pCfg->password)
                {
                    strcpy((char*) pCfgCpy->password, pCfg->password);
                }
                SOPC_GCC_DIAGNOSTIC_RESTORE
            }
        }
    }
    /* Append it to the internal list */
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_SLinkedList_Append(pListConfig, cfgId, pCfgCpy) == NULL)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Handle the id to the client */
    if (SOPC_STATUS_OK == status)
    {
        *pCfgId = cfgId;
    }
    else if (NULL != pCfgCpy)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_Free((void*) pCfgCpy->server_url);
        SOPC_Free((void*) pCfgCpy->security_policy);
        SOPC_Free((void*) pCfgCpy->path_cert_auth);
        SOPC_Free((void*) pCfgCpy->path_cert_srv);
        SOPC_Free((void*) pCfgCpy->path_cert_cli);
        SOPC_Free((void*) pCfgCpy->path_key_cli);
        SOPC_Free((void*) pCfgCpy->path_crl);
        SOPC_Free((void*) pCfgCpy->policyId);
        SOPC_Free((void*) pCfgCpy->username);
        SOPC_Free((void*) pCfgCpy->password);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        SOPC_Free(pCfgCpy);
    }

    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_Configured(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else if (SOPC_Atomic_Int_Get(&libConfigured) != 1)
    {
        status = SOPC_Toolkit_Configured();
        if (SOPC_STATUS_OK == status)
        {
            SOPC_Atomic_Int_Set(&libConfigured, 1);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_LibSub_ConnectionCfg* pCfg = NULL;
    SOPC_StaMac_Machine* pSM = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    if (UINT32_MAX == nCreatedClient)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == pCliId)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check the configuration Id */
    if (SOPC_STATUS_OK == status)
    {
        pCfg = SOPC_SLinkedList_FindFromId(pListConfig, cfgId);
        if (pCfg == NULL)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Connect: unknown configuration id: %" PRIu32 ".", cfgId);
        }
    }

    /* Creates a client state machine */
    if (SOPC_STATUS_OK == status)
    {
        ++nCreatedClient;
        *pCliId = nCreatedClient;
        status = SOPC_StaMac_Create(cfgId, *pCliId, pCfg->policyId, pCfg->username, pCfg->password,
                                    pCfg->data_change_callback, (double) pCfg->publish_period_ms, pCfg->n_max_keepalive,
                                    pCfg->n_max_lifetime, pCfg->token_target, pCfg->timeout_ms,
                                    pCfg->generic_response_callback, &pSM);
    }

    /* Adds it to the list */
    if (SOPC_STATUS_OK == status)
    {
        if (pSM != SOPC_SLinkedList_Append(pListClient, *pCliId, pSM))
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Starts the machine */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_StartSession(pSM);
    }

    /* Release the lock so that the event handler can work properly while waiting */
    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Wait for the connection to be created */
    if (SOPC_STATUS_OK == status)
    {
        int count = 0;
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_IsConnected(pSM) &&
               count * CONNECTION_TIMEOUT_MS_STEP < pCfg->timeout_ms)
        {
            SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);
            ++count;
        }
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
        else if (count * CONNECTION_TIMEOUT_MS_STEP >= pCfg->timeout_ms)
        {
            status = SOPC_STATUS_TIMEOUT;
            SOPC_StaMac_SetError(pSM);
        }
    }

    if (SOPC_STATUS_OK != status && NULL != pSM)
    {
        mutStatus = Mutex_Lock(&mutex);
        assert(SOPC_STATUS_OK == mutStatus);
        SOPC_StaMac_Machine* removedSM = (SOPC_StaMac_Machine*) SOPC_SLinkedList_RemoveFromId(pListClient, *pCliId);
        assert(pSM == removedSM);
        SOPC_StaMac_Delete(&pSM);
        mutStatus = Mutex_Unlock(&mutex);
        assert(SOPC_STATUS_OK == mutStatus);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                      const SOPC_LibSub_CstString* lszNodeId,
                                                      const SOPC_LibSub_AttributeId* lattrId,
                                                      int32_t nElements,
                                                      SOPC_LibSub_DataId* lDataId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;
    uintptr_t appCtx = 0;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == status);

    /* Finds the state machine */
    pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Create the monitored item and wait for its creation */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_CreateMonitoredItem(pSM, lszNodeId, lattrId, nElements, &appCtx, lDataId);
    }

    int64_t timeout_ms = SOPC_StaMac_GetTimeout(pSM);

    /* Release the lock so that the event handler can work properly while waiting */
    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Wait for the monitored item to be created */
    if (SOPC_STATUS_OK == status)
    {
        int count = 0;
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_HasMonItByAppCtx(pSM, appCtx) &&
               count * CONNECTION_TIMEOUT_MS_STEP < timeout_ms)
        {
            SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);
            ++count;
        }
        /* When the request timeoutHint is lower than pCfg->timeout_ms, the machine will go in error,
         *  and NOK is returned. */
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
        else if (count * CONNECTION_TIMEOUT_MS_STEP >= timeout_ms)
        {
            status = SOPC_STATUS_TIMEOUT;
            SOPC_StaMac_SetError(pSM);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_AsyncSendRequestOnSession(SOPC_LibSub_ConnectionId cliId,
                                                              void* requestStruct,
                                                              uintptr_t requestContext)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Retrieve the machine on which the request will be sent */
    pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* IsConnected() returns true when in stActivating. However, as the Connect() is blocking,
     * it is not possible to be in this state here.
     */
    if (SOPC_STATUS_OK == status && !SOPC_StaMac_IsConnected(pSM))
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_SendRequest(pSM, requestStruct, requestContext, SOPC_REQUEST_SCOPE_APPLICATION,
                                         SOPC_REQUEST_TYPE_USER);
    }

    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_AsyncSendDiscoveryRequest(const char* endpointUrl, uintptr_t requestContext)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    uint32_t iscConfig = 0;
    SOPC_StaMac_ReqCtx* pReqCtx = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    /* configure a secure channel */
    const char* security_policy = SOPC_SecurityPolicy_None_URI;
    const int32_t security_mode = OpcUa_MessageSecurityMode_None;

    status = Helpers_NewSCConfigFromLibSubCfg(endpointUrl, security_policy, security_mode, true, NULL, NULL, NULL, NULL,
                                              NULL, 0, &pscConfig);

    if (SOPC_STATUS_OK == status && NULL == pscConfig)
    {
        status = SOPC_STATUS_NOK;
    }

    /* add a secure channel */
    if (SOPC_STATUS_OK == status)
    {
        iscConfig = SOPC_ToolkitClient_AddSecureChannelConfig(pscConfig);
        if (0 == iscConfig)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* send a discovery request without going through the StaMac */
    OpcUa_GetEndpointsRequest* pReq = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_Create(&OpcUa_GetEndpointsRequest_EncodeableType, (void**) &pReq);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_CopyFromCString(&pReq->EndpointUrl, endpointUrl);
        }
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "# Error: Could not create the GetEndpointsRequest.\n");
        }
    }

    /* create a context wrapper */
    if (SOPC_STATUS_OK == status)
    {
        pReqCtx = SOPC_Calloc(1, sizeof(SOPC_StaMac_ReqCtx));
        if (NULL == pReqCtx)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* wrap the application context */
    if (SOPC_STATUS_OK == status)
    {
        pReqCtx->uid = 0;
        pReqCtx->appCtx = requestContext;
        pReqCtx->requestScope = SOPC_REQUEST_SCOPE_DISCOVERY;
        pReqCtx->requestType = SOPC_REQUEST_TYPE_GET_ENDPOINTS;
    }

    /* send the request */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncSendDiscoveryRequest(iscConfig, pReq, (uintptr_t) pReqCtx);
    }

    /* free if needed */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pReq);
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_Disconnect(const SOPC_LibSub_ConnectionId cliId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Retrieve the machine to disconnect */
    pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_StaMac_IsConnected(pSM))
        {
            status = SOPC_StaMac_StopSession(pSM);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Release the lock so that the event handler can work properly while waiting */
    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Wait for the connection to be closed */
    if (SOPC_STATUS_OK == status)
    {
        int count = 0;
        while (!SOPC_StaMac_IsError(pSM) && SOPC_StaMac_IsConnected(pSM) && count < 100)
        {
            SOPC_Sleep(10);
            count += 1;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_CreateSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                       SOPC_ClientHelper_DataChangeCbk cbkWrapper)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Retrieve the machine to disconnect */
    pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_ConfigureDataChangeCallback(pSM, cbkWrapper);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_CreateSubscription(pSM);
    }

    /* Release the lock so that the event handler can work properly while waiting */
    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Wait for the monitored item to be created */
    if (SOPC_STATUS_OK == status)
    {
        int64_t timeout_ms = SOPC_StaMac_GetTimeout(pSM);
        int count = 0;
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_HasSubscription(pSM) &&
               count * CONNECTION_TIMEOUT_MS_STEP < timeout_ms)
        {
            SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);
            ++count;
        }
        /* When the request timeoutHint is lower than pCfg->timeout_ms, the machine will go in error,
         *  and NOK is returned. */
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
        else if (count * CONNECTION_TIMEOUT_MS_STEP >= timeout_ms)
        {
            status = SOPC_STATUS_TIMEOUT;
            SOPC_StaMac_SetError(pSM);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_ClientCommon_DeleteSubscription(const SOPC_LibSub_ConnectionId cliId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Retrieve the machine to disconnect */
    pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_StaMac_HasSubscription(pSM))
        {
            status = SOPC_StaMac_DeleteSubscription(pSM);
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }

    /* Release the lock so that the event handler can work properly while waiting */
    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* Wait for the subscription to be deleted */
    if (SOPC_STATUS_OK == status)
    {
        int64_t timeout_ms = SOPC_StaMac_GetTimeout(pSM);
        int count = 0;
        while (!SOPC_StaMac_IsError(pSM) && SOPC_StaMac_HasSubscription(pSM) &&
               count * CONNECTION_TIMEOUT_MS_STEP < timeout_ms)
        {
            SOPC_Sleep(CONNECTION_TIMEOUT_MS_STEP);
            ++count;
        }
        /* When the request timeoutHint is lower than pCfg->timeout_ms, the machine will go in error,
         *  and NOK is returned. */
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
        else if (count * CONNECTION_TIMEOUT_MS_STEP >= timeout_ms)
        {
            status = SOPC_STATUS_TIMEOUT;
            SOPC_StaMac_SetError(pSM);
        }
    }

    return status;
}

/* ========================
 * Internal implementations
 * ========================
 */

static bool LibCommon_IsDiscoveryEvent(SOPC_App_Com_Event event, uintptr_t appCtx)
{
    bool bDiscovery = true;
    SOPC_StaMac_RequestScope scope = SOPC_REQUEST_SCOPE_APPLICATION;

    /* Depending on the event, check whether it targets a state machine or not */
    switch (event)
    {
    /* appCtx is request context */
    case SE_RCV_DISCOVERY_RESPONSE:
    case SE_SND_REQUEST_FAILED:
        if (0 != appCtx)
        {
            scope = ((SOPC_StaMac_ReqCtx*) appCtx)->requestScope;
        }

        if (SOPC_REQUEST_SCOPE_DISCOVERY == scope)
        {
            bDiscovery = true;
        }
        else
        {
            bDiscovery = false;
        }
        break;
    /* appCtx is session context */
    case SE_RCV_SESSION_RESPONSE:
    case SE_SESSION_ACTIVATION_FAILURE:
    case SE_ACTIVATED_SESSION:
    case SE_SESSION_REACTIVATING:
    case SE_CLOSED_SESSION:
        bDiscovery = false;
        break;
    default:
        bDiscovery = false;
        Helpers_Log(SOPC_TOOLKIT_LOG_LEVEL_ERROR, "Unexpected event %d received.", event);
        break;
    }

    return bDiscovery;
}

static void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext)
{
    SOPC_SLinkedListIterator pIterCli = NULL;
    SOPC_LibSub_ConnectionId cliId = 0;
    SOPC_StaMac_Machine* pSM = NULL;
    bool bProcessed = false;

    if (SOPC_Atomic_Int_Get(&libInitialized) == 0 || SOPC_Atomic_Int_Get(&libConfigured) == 0)
    {
        return;
    }

    SOPC_ReturnStatus mutStatus = Mutex_Lock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);

    /* check for event type */
    if (LibCommon_IsDiscoveryEvent(event, appContext))
    {
        if (NULL != getEndpointsCbk)
        {
            /* intercept discovery response and call SOPC_ClientCommon_GetEndpointCbk if not NULL */
            if (SE_RCV_DISCOVERY_RESPONSE == event)
            {
                getEndpointsCbk(SOPC_STATUS_OK, param, ((SOPC_StaMac_ReqCtx*) appContext)->appCtx);
            }
            else
            {
                getEndpointsCbk(SOPC_STATUS_NOK, param, ((SOPC_StaMac_ReqCtx*) appContext)->appCtx);
            }
            bProcessed = true;
        }
        SOPC_Free((SOPC_StaMac_ReqCtx*) appContext);
    }
    else
    {
        /* List through known clients and call state machine event callback */
        pIterCli = SOPC_SLinkedList_GetIterator(pListClient);
        while (NULL != pIterCli)
        {
            pSM = SOPC_SLinkedList_NextWithId(&pIterCli, &cliId);
            /* No more than one machine shall process the event */
            if (SOPC_StaMac_EventDispatcher(pSM, NULL, event, IdOrStatus, param, appContext))
            {
                assert(!bProcessed);
                bProcessed = true;
                /* Post process the event for callbacks. */
                if (SE_CLOSED_SESSION == event || SE_SESSION_ACTIVATION_FAILURE == event)
                {
                    /* The disconnect callback shall be called after the client has been destroyed */
                    cbkDisco(cliId);
                }
            }
        }
    }

    /* TODO we receive a SE_SND_REQUEST_FAILED  when we try to send a GetEndpoint to a non-existing server */
    /* What should we do ? */

    /* At least one machine or a generic callback should have processed the event */
    assert(bProcessed);

    mutStatus = Mutex_Unlock(&mutex);
    assert(SOPC_STATUS_OK == mutStatus);
}
