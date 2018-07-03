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

#include "sopc_array.h"
#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"

#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_time.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

#include "state_machine.h"
#include "toolkit_helpers.h"

/* =========
 * Internals
 * =========
 */

/* Client structures */

/* Global library variables */
static bool bLibInitialized = false;
static bool bLibConfigured = false;
static SOPC_LibSub_DisconnectCbk cbkDisco = NULL;
static SOPC_SLinkedList* pListConfig = NULL; /* IDs are cfgId == Toolkit cfgScId, value is SOPC_LibSub_ConnectionCfg */
static SOPC_SLinkedList* pListClient = NULL; /* IDs are cliId, value is a StaMac */
static SOPC_LibSub_ConnectionId nCreatedClient = 0;
static SOPC_Array* pArrScConfig = NULL; /* Stores the created scConfig to free them in SOPC_LibSub_Clear() */

/* Event callback */
void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);

/* ==================
 * API implementation
 * ==================
 */

SOPC_LibSub_CstString SOPC_LibSub_GetVersion(void)
{
    return "Subscribe library v" SOPC_LIBSUB_VERSION " on S2OPC Toolkit v" SOPC_TOOLKIT_VERSION;
}

SOPC_ReturnStatus SOPC_LibSub_Initialize(const SOPC_LibSub_StaticCfg* pCfg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pCfg || NULL == pCfg->host_log_callback || NULL == pCfg->disconnect_callback)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (bLibInitialized || bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
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
        bLibInitialized = true;
        status = SOPC_Toolkit_Initialize(ToolkitEventCallback);
    }

    if (!bLibInitialized)
    {
        /* Clean partial mallocs */
        SOPC_SLinkedList_Delete(pListConfig);
        pListConfig = NULL;
        SOPC_SLinkedList_Delete(pListClient);
        pListClient = NULL;
        SOPC_Array_Delete(pArrScConfig);
        pArrScConfig = NULL;
    }

    return status;
}

void SOPC_LibSub_Clear(void)
{
    SOPC_SLinkedListIterator pIter = NULL;
    SOPC_StaMac_Machine* pSM = NULL;
    SOPC_LibSub_ConnectionCfg* pCfg = NULL;

    SOPC_Toolkit_Clear();

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
        /* TODO: Free content of the struct, make a static function that can also be used in
         * SOPC_LibSub_ConfigureConnection */
        free(pCfg);
    }
    SOPC_SLinkedList_Delete(pListConfig);
    pListConfig = NULL;

    SOPC_Array_Delete(pArrScConfig);
    pArrScConfig = NULL;

    bLibInitialized = false;
    bLibConfigured = false;
}

SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConfigurationId* pCfgId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    uint32_t cfgId = 0;
    SOPC_LibSub_ConnectionCfg* pCfgCpy = NULL;

    if (!bLibInitialized || bLibConfigured)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == pCfg || NULL == pCfgId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pCfg->policyId)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Cannot configure connection with NULL policyId.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pCfg->username && NULL == pCfg->password)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Cannot configure connection with non NULL user name but NULL password.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Create the new configuration */
    status = Helpers_NewSCConfigFromLibSubCfg(pCfg->server_url, pCfg->security_policy, pCfg->security_mode,
                                              pCfg->disable_certificate_verification, pCfg->path_cert_auth,
                                              pCfg->path_cert_srv, pCfg->path_cert_cli, pCfg->path_key_cli,
                                              pCfg->path_crl, pCfg->sc_lifetime, &pscConfig);

    /* Store it to free it on SOPC_LibSub_Clear() */
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

    /* Copy it to append it safely to the internal list */
    if (SOPC_STATUS_OK == status)
    {
        pCfgCpy = malloc(sizeof(SOPC_LibSub_ConnectionCfg));
        if (NULL == pCfgCpy)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            /* TODO: Make a deep copy of the strings */
            *pCfgCpy = *pCfg;
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
        /* TODO: Free strings */
        free(pCfgCpy);
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Configured(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!bLibInitialized || bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        status = SOPC_Toolkit_Configured();
        if (SOPC_STATUS_OK == status)
        {
            bLibConfigured = true;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_LibSub_ConnectionCfg* pCfg = NULL;
    SOPC_StaMac_Machine* pSM = NULL;

    if (!bLibInitialized || !bLibConfigured || UINT32_MAX == nCreatedClient)
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
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Connect: unknown configuration id: %" PRIu32 ".", cfgId);
        }
    }

    /* Creates a client state machine */
    if (SOPC_STATUS_OK == status)
    {
        ++nCreatedClient;
        *pCliId = nCreatedClient;
        status = SOPC_StaMac_Create(cfgId, *pCliId, pCfg->policyId, pCfg->username, pCfg->password,
                                    pCfg->data_change_callback, (double) pCfg->publish_period_ms, pCfg->n_max_keepalive,
                                    pCfg->n_max_lifetime, pCfg->token_target, &pSM);
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

    /* Wait for the subscription to be created */
    /* TODO: use Mutex and CV */
    if (SOPC_STATUS_OK == status)
    {
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_HasSubscription(pSM))
        {
            SOPC_Sleep(1);
        }
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        SOPC_StaMac_Delete(&pSM);
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                SOPC_LibSub_CstString szNodeId,
                                                SOPC_LibSub_AttributeId attrId,
                                                SOPC_LibSub_DataId* pDataId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;
    uintptr_t appCtx = 0;

    if (!bLibInitialized || !bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Finds the state machine */
    if (SOPC_STATUS_OK == status)
    {
        pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
        if (NULL == pSM)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Create the monitored item and wait for its creation */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_CreateMonitoredItem(pSM, szNodeId, attrId, &appCtx, pDataId);
    }

    /* Wait for the monitored item to be created */
    /* TODO: use Mutex and CV */
    if (SOPC_STATUS_OK == status)
    {
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_HasMonItByAppCtx(pSM, appCtx))
        {
            SOPC_Sleep(1);
        }
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Disconnect(const SOPC_LibSub_ConnectionId cliId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;

    if (!bLibInitialized || !bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Retrieve the machine to disconnect */
    if (SOPC_STATUS_OK == status)
    {
        pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
        if (NULL == pSM)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == status && SOPC_StaMac_IsConnected(pSM))
    {
        status = SOPC_StaMac_StopSession(pSM);
    }

    /* Wait for the connection to be closed */
    /* TODO: use Mutex and CV */
    if (SOPC_STATUS_OK == status)
    {
        while (!SOPC_StaMac_IsError(pSM) && SOPC_StaMac_IsConnected(pSM))
        {
            SOPC_Sleep(1);
        }
    }

    return status;
}

/* ========================
 * Internal implementations
 * ========================
 */

void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext)
{
    SOPC_SLinkedListIterator pIterCli = NULL;
    SOPC_LibSub_ConnectionId cliId = 0;
    SOPC_StaMac_Machine* pSM = NULL;
    bool bProcessed = false;

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
            /* Post process the event. The only interesting event is for now CLOSED. */
            if (SE_CLOSED_SESSION == event)
            {
                /* The disconnect callback shall be called after the client has been destroyed */
                cbkDisco(cliId);
            }
        }
    }

    /* At least one machine should have processed the event */
    assert(bProcessed);
}
